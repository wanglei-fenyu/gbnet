#if 0
#include "network.h"
#include "network/session/session.h"
#include "network/buffer/buffer.h"
using ListenMap = typename std::unordered_map<uint64_t,gb::net_listen_fun>;
using RpcInterfaceMap = typename  std::map<uint64_t, gb::rpc_listen_fun>;
using RpcCallerMap = typename std::map<uint64_t, gb::RpcCallPtr>;

static ListenMap gs_ListenFunctionMap;
static RpcCallerMap gs_RpcCallerMap;
static RpcInterfaceMap gs_RpcInterfaceMap;
static int32_t gs_sequence_tail = 0;


uint64_t GetSequence()
{
	std::thread::id id = std::this_thread::get_id();
	uint32_t thread_id = *((uint32_t*)&id);
	return (((uint64_t)thread_id) << 32) + (++gs_sequence_tail);
}

void Listen_Option(int type, int id, gb::net_listen_fun f, std::string protoName)
{
    uint64_t key      = (((uint64_t)type) << 32) + id;
    auto     p_FunsIt = gs_ListenFunctionMap[key] = f;
}

void Call(Meta& meta, gb::RpcCallPtr call)
{
	if (!gs_RpcCallerMap.insert({ meta.sequence(),call }).second)
	{
		LOG_ERROR("insert gs_RpcCallerMap fail seq:{} method{}",meta.sequence(),meta.method());

	}
	//开启计时器

	//调用回调
	call->Call(meta);
}

void Call(Meta& meta, gb::RpcCallPtr call,std::vector<uint8_t>& data)
{
	if (!gs_RpcCallerMap.insert({ meta.sequence(),call }).second)
	{
		LOG_ERROR("insert gs_RpcCallerMap fail seq:{} method{}",meta.sequence(),meta.method());

	}
	//开启计时器

	//调用回调
	call->Call(meta,data);
}

void Call(Meta & meta, gb::RpcCallPtr call, std::vector<uint8_t>&& data)
{
	if (!gs_RpcCallerMap.insert({meta.sequence(), call}).second)
	{
		LOG_ERROR("insert gs_RpcCallerMap fail seq:{} method{}", meta.sequence(), meta.method());
	}
	//开启计时器

	//调用回调
	call->Call(meta, data);

}
void Call(gb::RpcCallPtr call, std::string method, sol::variadic_args& args)
{
	Meta meta;
	uint64_t method_key = MD5::MD5Hash64(method.c_str());
	meta.set_method(method_key);
	uint64_t seq_id = GetSequence();
	meta.set_sequence(seq_id);
	meta.set_mode(MsgMode::Request);
	call->SetId(seq_id);
	if( args.size() > 0)
	{
		std::vector<uint8_t> data =  gb::msgpack::pack(args);
		Call(meta, call,std::move(data));
	}
	else
	{
		Call(meta, call);
	}
}

void UnListen(int type, int id)
{
    uint64_t key      = (((uint64_t)type) << 32) + id;
    auto     p_FunsIt = gs_ListenFunctionMap.find(key);
    if (gs_ListenFunctionMap.end() == p_FunsIt)
    {
        LOG_ERROR("message don't listen type:{} id:{}", type, id);
        return;
    }
    {
        gs_ListenFunctionMap.erase(p_FunsIt);
    }
}



static void Dispatch(const gb::SessionPtr& session, const gb::ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size)
{
	switch (meta.mode())
	{
    case MsgMode::Msg:
	{
		uint64_t key = (((uint64_t)meta.type()) << 32) + (int)meta.id();
		auto fun = gs_ListenFunctionMap.find(key);
		if (fun!= gs_ListenFunctionMap.end())
				fun->second(session, buffer,meta,meta_size,data_size);
		break;
	}
	case MsgMode::Request:
	{	
		uint64_t key = meta.method();
		auto func = gs_RpcInterfaceMap.find(key);
		if (func == gs_RpcInterfaceMap.end())
			return;

		auto& f = func->second;
		f(session, buffer,meta,meta_size,data_size);
		break;

	}

	case MsgMode::Response:
	{
		uint64_t seq = meta.sequence();
		auto it = gs_RpcCallerMap.find(seq);
		if (it == gs_RpcCallerMap.end())
			return;

		it->second->Done(session, buffer,meta,meta_size,data_size);
		gs_RpcCallerMap.erase(it);

		break;
	}
	default:
		break;
	}

}

void OnReceived(const gb::SessionPtr& session, const gb::ReadBufferPtr& buffer,int meta_size,int64_t data_size) 
{
	Meta meta;
	if (meta.ParseFromBoundedZeroCopyStream(buffer.get(), meta_size))
	{
		Dispatch(session, buffer, meta, meta_size, data_size);
	}
}


#endif
