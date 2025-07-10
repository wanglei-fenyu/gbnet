#include "network.h"
#include "../common/worker/worker_manager.h"
NAMESPACE_BEGIN(gb)
using ListenMap = typename std::unordered_map<uint64_t,net_listen_fun>;
using RpcInterfaceMap = typename  std::map<uint64_t, rpc_listen_fun>;
using RpcCallerMap = typename std::map<uint64_t, RpcCallPtr>;
//using RpcCallerMap = typename std::map<uint32_t,std::map<uint64_t, RpcCallPtr&>>;
static thread_local ListenMap gs_ListenFunctionMap;
static thread_local RpcInterfaceMap gs_RpcInterfaceMap;
static thread_local RpcCallerMap gs_RpcCallerMap;
static thread_local int32_t gs_sequence_tail = 0;

static std::shared_ptr<Server> gs_Server;

uint64_t GetSequence()
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
    Id.seq   = gs_sequence_tail++;
    return Id.value;
}

void UnListen(int type, int id, std::string signal, int level)
{
	uint64_t key = (((uint64_t)type) << 32) + id;
	auto p_FunsIt = gs_ListenFunctionMap.find(key);
	if (gs_ListenFunctionMap.end() == p_FunsIt)
	{
		LOG_ERROR("message don't listen type:{} id:{}", type, id);
		return;
	}
	{
        gs_ListenFunctionMap.erase(p_FunsIt);
	}
}

void Send(Session* session, int type, int id, google::protobuf::Message& msg)
{
	Meta meta;
	meta.set_id(id);
	meta.set_type(type);
	session->Send(&meta, &msg);
}
void Send(std::shared_ptr<Session> session, int type, int id, google::protobuf::Message& msg)
{
	Meta meta;
	meta.set_id(id);
	meta.set_type(type);
	session->Send(&meta, &msg);

}


void ListenOption(int type, int id, net_listen_fun f, std::string protoName)
{
	uint64_t key = (((uint64_t)type) << 32) + id;
    gs_ListenFunctionMap[key] = f;
}

void _Call(RpcCallPtr call, std::string method, sol::variadic_args& args)
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
        _Call(meta, call, read_buffer);
    }
    else
    {
        _Call(meta, call);
    }
}

void _Call(Meta& meta, RpcCallPtr call, const ReadBufferPtr buffer)
{
    if (!call)
        return;
	if (!gs_RpcCallerMap.insert({meta.sequence(), call}).second)
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
void RpcCancel(int64_t seq_id)
{
    auto it = gs_RpcCallerMap.find(seq_id);
    if (it != gs_RpcCallerMap.end())
		gs_RpcCallerMap.erase(it);
}

void RegisterOption(std::string method, rpc_listen_fun fn)
{
	uint64_t key = MD5::MD5Hash64(method.c_str());
	gs_RpcInterfaceMap.insert({key,fn});
}

void UnRegister(std::string method)
{
	uint64_t key = MD5::MD5Hash64(method.c_str());
	gs_RpcInterfaceMap.erase(key);
}


void Dispatch(const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size)
{
	switch (meta.mode())
	{
    case MsgMode::Msg:
	{
		auto worker = WorkerManager::Instance()->GetWorker(meta.id() % WorkerManager::Instance()->Size());
        if (worker)
        {
			worker->Post([session = session, buffer = buffer ,meta = std::move(meta), meta_size,data_size]() mutable
				{
					uint64_t key = (((uint64_t)meta.type()) << 32) + (int)meta.id();
					auto fun = gs_ListenFunctionMap.find(key);
					if (fun != gs_ListenFunctionMap.end())
						fun->second(session, buffer,meta,meta_size,data_size);
				});
        }
        #if USE_MAIN_THREAD
        else
        {
            uint64_t key = (((uint64_t)meta.type()) << 32) + (int)meta.id();
            auto     fun = gs_ListenFunctionMap.find(key);
            if (fun != gs_ListenFunctionMap.end())
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
            worker->Post([session = session, buffer = buffer ,meta = std::move(meta), meta_size,data_size]() mutable
                {
                uint64_t key  = meta.method();
                auto     func = gs_RpcInterfaceMap.find(key);
                if (func == gs_RpcInterfaceMap.end())
                    return;
                func->second(session, buffer, meta, meta_size, data_size);
            });
        }
        #if USE_MAIN_THREAD
        else
        {
            int64_t key  = meta.method();
            auto     fun = gs_RpcInterfaceMap.find(key);
            if (fun != gs_RpcInterfaceMap.end())
				fun->second(session, buffer,meta,meta_size,data_size);
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
			worker->Post([worker,session = session, buffer = buffer ,meta = std::move(meta), meta_size,data_size]() mutable
				{
                    
					uint32_t   thread = worker->GetWorkerId();
					uint64_t seq       = meta.sequence();
					auto  it         = gs_RpcCallerMap.find(seq);
					if (it == gs_RpcCallerMap.end())
						return;
					if (it->second)
						it->second->Done(session,buffer,meta,meta_size,data_size);
					gs_RpcCallerMap.erase(it);
				});
        }
        #if USE_MAIN_THREAD
        else
        {
            std::thread::id id        = std::this_thread::get_id();
            uint32_t        thread_id = *((uint32_t*)&id);
            auto            it        = gs_RpcCallerMap.find(sequence.value);
            if (it == gs_RpcCallerMap.end())
                return;
            if (it->second)
                it->second->Done(session, buffer, meta, meta_size, data_size);
            gs_RpcCallerMap.erase(it);
        }
        #endif
	}
	default:
		break;
	}

}

void OnReceiveCall(const SessionPtr& session, const ReadBufferPtr& buffer, int meta_size, int64_t data_size)
{
    Meta meta;
    if (meta.ParseFromBoundedZeroCopyStream(buffer.get(), meta_size))
    {
        Dispatch(session, buffer, meta, meta_size, data_size);
    }
}

void net_init()
{
	auto [ip, port] = AppTypeMgr::Instance()->GetServerIpPort();
	std::string uir = ip + ":" + port;

	gb::ServerOptions options;
    options.keep_alive_time = -1;
    options.io_service_pool_size = 1;
    gs_Server               = std::make_shared<gb::Server>(options);
    gs_Server->SetAcceptCallBack([](const SessionPtr& session) {
        session->set_return_io_service_pool_fun([]() -> gb::IoServicePoolPtr {
            return gs_Server->GetIoServicePool();
        });
        LOG_INFO("Accept:{}", session->socket().local_endpoint().address().to_string());
    });
    gs_Server->SetCloseCallBack([](const SessionPtr& session) {
        LOG_INFO("Close:{}", session->socket().local_endpoint().address().to_string());
    });
    gs_Server->SetReceivedCallBack(OnReceiveCall);
	
	if (!gs_Server->Start(uir))
	{
        LOG_ERROR("server start fail");
	}
}

std::shared_ptr<Server>& GetServer()
{
    return gs_Server;
}



NAMESPACE_END

