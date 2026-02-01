#include "network_manager.h"
#include "../../common/worker/worker_manager.h"
#include "app/app_def.h"
#include "../net/server.h"

NAMESPACE_BEGIN(gb)


void NetworkManager::Init(HandleInterface* handleInterface)
{
	if (!handleInterface)
	{
        LOG_ERROR("handle interface is nullptr")
        return;
	}
	auto [ip, port] = AppTypeMgr::Instance()->GetServerIpPort();
	std::string uir = ip + ":" + port;

    handleInterface->SetReceivedCallBack([this](const SessionPtr& session, const ReadBufferPtr& buffer, int meta_size, int64_t data_size) {
		OnReceiveCall(session, buffer, meta_size, data_size);
	});
	
}

uint64_t NetworkManager::GetSequence()
{
	auto work = WorkerManager::Instance()->GetCurWorker();
	if (!work)
	{
		LOG_ERROR("cur not work thread");
		return 0;
	}
	uint32_t   thread = work->GetWorkerId();
	SequenceId Id;
	Id.value = 0;
	Id.index = work->GetIndex();
	Id.seq   = sequence_tail_++;
	return Id.value;
}

void NetworkManager::UnListen(int type, int id, std::string signal, int level)
{
	uint64_t key = (((uint64_t)type) << 32) + id;
	auto p_FunsIt = listen_function_map_.find(key);
	if (listen_function_map_.end() == p_FunsIt)
	{
		LOG_ERROR("message don't listen type:{} id:{}", type, id);
		return;
	}
	{
		listen_function_map_.erase(p_FunsIt);
	}
}

void NetworkManager::Send(Session* session, int type, int id, google::protobuf::Message& msg)
{
	Meta meta;
	meta.set_id(id);
	meta.set_type(type);
	session->Send(&meta, &msg);
}

void NetworkManager::Send(std::shared_ptr<Session> session, int type, int id, google::protobuf::Message& msg)
{
	Meta meta;
	meta.set_id(id);
	meta.set_type(type);
	session->Send(&meta, &msg);
}

void NetworkManager::ListenOption(int type, int id, net_listen_fun f, std::string protoName)
{
	uint64_t key = (((uint64_t)type) << 32) + id;
	listen_function_map_[key] = f;
}

void NetworkManager::CallImpl(RpcCallPtr call, std::string method, sol::variadic_args& args)
{
	if (!call)
	{
		return;
	}
	Meta meta;
	uint64_t    method_key = MD5::MD5Hash64(method.c_str());
	meta.set_method(method_key);
	uint64_t seq_id = GetSequence();
	meta.set_sequence(seq_id);
	meta.set_mode(MsgMode::Request);
	call->SetId(seq_id);
	if (args.size() > 0)
	{
		std::vector<uint8_t> data = gb::msgpack::pack(args);
		WriteBuffer          write_buffer;
		write_buffer.Append((const char *)data.data(), data.size());
		ReadBufferPtr read_buffer(new ReadBuffer());
		write_buffer.SwapOut(read_buffer.get());
		CallImpl(meta, call, read_buffer);
	}
	else
	{
		CallImpl(meta, call);
	}
}

void NetworkManager::CallImpl(Meta& meta, RpcCallPtr call, const ReadBufferPtr buffer)
{
	if (!call)
		return;
	if (!rpc_caller_map_.insert({meta.sequence(), call}).second)
	{
		LOG_ERROR("insert gs_RpcCallerMap fail seq:{} method{}", meta.sequence(), meta.method());
	}
	if (buffer && buffer->TotalCount() > 0)
	{
		call->Call(meta, buffer);
	}
	else
	{
		call->Call(meta);
	}
}

void NetworkManager::RpcCancel(int64_t seq_id)
{
	auto it = rpc_caller_map_.find(seq_id);
	if (it != rpc_caller_map_.end())
		rpc_caller_map_.erase(it);
}

void NetworkManager::RegisterOption(std::string method, rpc_listen_fun fn)
{
	uint64_t key = MD5::MD5Hash64(method.c_str());
	rpc_interface_map_.insert({key, fn});
}

void NetworkManager::UnRegister(std::string method)
{
	uint64_t key = MD5::MD5Hash64(method.c_str());
	rpc_interface_map_.erase(key);
}

void NetworkManager::Dispatch(const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size)
{
	switch (meta.mode())
	{
	case MsgMode::Msg:
	{
		auto worker = WorkerManager::Instance()->GetWorker(meta.id() % WorkerManager::Instance()->Size());
		if (worker)
		{
			worker->Post([this, session = session, buffer = buffer, meta = std::move(meta), meta_size, data_size]() mutable
				{
					uint64_t key = (((uint64_t)meta.type()) << 32) + (int)meta.id();
					auto fun = listen_function_map_.find(key);
					if (fun != listen_function_map_.end())
						fun->second(session, buffer, meta, meta_size, data_size);
				});
		}
#if USE_MAIN_THREAD
		else
		{
			uint64_t key = (((uint64_t)meta.type()) << 32) + (int)meta.id();
			auto     fun = listen_function_map_.find(key);
			if (fun != listen_function_map_.end())
				fun->second(session, buffer, meta, meta_size, data_size);
		}
#endif
		break;
	}

	case MsgMode::Request:
	{
		auto worker = WorkerManager::Instance()->GetWorker(meta.id() % WorkerManager::Instance()->Size());
		if (worker)
		{
			worker->Post([this, session = session, buffer = buffer, meta = std::move(meta), meta_size, data_size]() mutable
				{
					uint64_t key  = meta.method();
					auto     func = rpc_interface_map_.find(key);
					if (func == rpc_interface_map_.end())
						return;
					func->second(session, buffer, meta, meta_size, data_size);
				});
		}
#if USE_MAIN_THREAD
		else
		{
			int64_t key  = meta.method();
			auto     fun = rpc_interface_map_.find(key);
			if (fun != rpc_interface_map_.end())
				fun->second(session, buffer, meta, meta_size, data_size);
		}
#endif
		break;
	}
	case MsgMode::Response:
	{
		SequenceId Id;
		Id.value = meta.sequence();
		auto worker = WorkerManager::Instance()->GetWorker(Id.index);
		if (worker)
		{
			worker->Post([this, worker, session = session, buffer = buffer, meta = std::move(meta), meta_size, data_size]() mutable
				{
					uint32_t   thread = worker->GetWorkerId();
					uint64_t seq       = meta.sequence();
					auto  it         = rpc_caller_map_.find(seq);
					if (it == rpc_caller_map_.end())
						return;
					if (it->second)
						it->second->Done(session, buffer, meta, meta_size, data_size);
					rpc_caller_map_.erase(it);
				});
		}
#if USE_MAIN_THREAD
		else
		{
			std::thread::id id        = std::this_thread::get_id();
			uint32_t        thread_id = *((uint32_t*)&id);
			SequenceId sequence;
			sequence.value = meta.sequence();
			auto            it        = rpc_caller_map_.find(sequence.value);
			if (it == rpc_caller_map_.end())
				return;
			if (it->second)
				it->second->Done(session, buffer, meta, meta_size, data_size);
			rpc_caller_map_.erase(it);
		}
#endif
	}
	default:
		break;
	}
}

void NetworkManager::OnReceiveCall(const SessionPtr& session, const ReadBufferPtr& buffer, int meta_size, int64_t data_size)
{
	Meta meta;
	if (meta.ParseFromBoundedZeroCopyStream(buffer.get(), meta_size))
	{
		Dispatch(session, buffer, meta, meta_size, data_size);
	}
}

NAMESPACE_END
