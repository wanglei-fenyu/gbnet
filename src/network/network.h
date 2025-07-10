#pragma once 
#include "common/def.h"
#include "common/define.h"
#include "session/session.h"
#include "network_function.hpp"
#include "../log/log_help.h"
#include "rpc/rpc_call.h"
#include "rpc/rpc_reply.h"
#include "rpc/rpc_function.hpp"
#include "../protobuf/meta.pb.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "io_service_pool/io_service_pool.h"
#include "net/server.h"
#include "buffer/buffer.h"
#include "md5.hpp"
NAMESPACE_BEGIN(gb)


void                 net_init();
void Send(Session *session,int type, int id, google::protobuf::Message& msg);
void Send(std::shared_ptr<Session> session,int type, int id, google::protobuf::Message& msg);
void ListenOption(int type, int id, net_listen_fun f, std::string protoName);

template <typename F>
void Listen(int type, int id, F f,std::string protoName="")
{
	net_listen_fun func;
	if  constexpr (std::is_same<F, sol::function>::value)
		func = NetFunctionaTraits<sol::function>::make(f, protoName);
	else if constexpr (HasInvokeOperator<typename std::decay<F>::type>::value)
		func = ServerLambdaFunc(f, &F::operator());
	else
		func = NetFunctionaTraits<F>::make(f);
	ListenOption(type, id, std::move(func), protoName);

}

void UnListen(int type, int id, std::string signal, int level = 0);
void Dispatch(const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size);

uint64_t GetSequence();
void     _Call(RpcCallPtr call, std::string method, sol::variadic_args& args);


void    _Call(Meta& meta, RpcCallPtr call, const ReadBufferPtr buffer = nullptr);

std::shared_ptr<Server>& GetServer();

template<typename ...Args>
void Call(RpcCallPtr call, std::string method, Args&&... args)
{
    if (!call)
    {
        return;
    }
	Meta meta;
	uint64_t method_key = MD5::MD5Hash64(method.c_str());
	meta.set_method(method_key);
	uint64_t seq_id = GetSequence();
	meta.set_sequence(seq_id);
	meta.set_mode(MsgMode::Request);
	call->SetId(seq_id);
	if constexpr (sizeof...(args) > 0)
	{
		std::vector<uint8_t> data =  gb::msgpack::pack<Args...>(std::forward<Args>(args)...);
        WriteBuffer          write_buffer;
        write_buffer.Append((const char*)data.data(), data.size());
		ReadBufferPtr read_buffer(new ReadBuffer());
        write_buffer.SwapOut(read_buffer.get());
		_Call(meta, call,read_buffer);
	}
	else
	{
		_Call(meta, call);
	}
}

void RegisterOption(std::string method, rpc_listen_fun fn);
void UnRegister(std::string method);
void RpcCancel(int64_t seq_id);
void OnReceiveCall(const SessionPtr& session, const ReadBufferPtr& buffer, int meta_size, int64_t data_size);

template<typename F>
void Register(std::string method, F fn)
{
    net_listen_fun func;
	if  constexpr (!std::is_same<F, sol::function>::value)
		func = RpcFunctionaTraits<F>::make(fn);
	else if constexpr (HasInvokeOperator<typename std::decay<F>::type>::value)
		func = RpcLambdaFunc(fn, &F::operator());
	else
	{
		auto lua_state = fn.lua_state();
		sol::state_view lua_view(lua_state);
		sol::state* state = (sol::state*)&lua_view;
		func = RpcFunctionaTraits<sol::function>::make(state,fn);
	}
	RegisterOption(method, func);

}



template<typename T>
struct RpcCallAwaiter 
{
public:
	using private_call = std::function<void(std::coroutine_handle<>,T&)>;
public:
	RpcCallAwaiter(private_call call_back) : private_call_(std::move(call_back))
	{
	}
public:
	bool await_ready() noexcept { return false; }

	void await_suspend(std::coroutine_handle<> handle)
	{
		private_call_(handle,resoult_);
	}

	T await_resume() { return std::move(resoult_); }
private:
	T resoult_;
	private_call private_call_;
};

template<>
struct RpcCallAwaiter<void>
{
public:
	using private_call = std::function<void(std::coroutine_handle<>)>;
public:
	RpcCallAwaiter(private_call call_back) : private_call_(std::move(call_back))
	{
	}
public:
	bool await_ready() noexcept { return false; }

	void await_suspend(std::coroutine_handle<> handle)
	{
		private_call_(handle);
	}

	void await_resume() { return; }
private:
	private_call private_call_;
};



template <typename T1 = void,typename... Rets>
struct CoRpc
{
    using ResultType = std::conditional_t<sizeof...(Rets) == 0, T1, std::tuple<T1,Rets...>>;
	template<typename... Args>
    static async_simple::coro::Lazy<ResultType> execute(gb::RpcCallPtr call, std::string method, Args... args) noexcept {
        if constexpr ((sizeof...(Rets) == 0) )
        {
			if constexpr (std::is_void_v<ResultType>)
			{
                co_return co_await RpcCallAwaiter<void>{[&](std::coroutine_handle<> h) {
					call->SetCallBack([&, h]() { h.resume(); });
					call->SetTimeout([&, h]() { h.resume(); });
                    gb::Call(call, method, std::forward<Args>(args)...);
                    }};

			}
			else
			{
				co_return co_await RpcCallAwaiter<ResultType>{[&](std::coroutine_handle<> h, ResultType& result) {
					call->SetCallBack([&, h](ResultType r) { 
						result = std::move(r); 
						h.resume(); 
					});
					call->SetTimeout([&,h]() { h.resume(); });
                    gb::Call(call, method, std::forward<Args>(args)...);
				}};
			}
        }
        else
        {
			co_return co_await RpcCallAwaiter<ResultType>{[&](std::coroutine_handle<> h, ResultType& result) {
				call->SetCallBack([&, h](T1 t,Rets... r) { 
					result = std::make_tuple(std::move(t),std::move(r)...);
					h.resume(); 
				});
				call->SetTimeout([&,h]() { h.resume(); });
                gb::Call(call, method, std::forward<Args>(args)...);
			}};
        }
    }
};


NAMESPACE_END

