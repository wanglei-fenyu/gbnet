#if 0
#pragma once
#include "network/network_function.hpp"
#include "network/rpc/rpc_call.h"
#include "network/md5.hpp"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"

void UnListen(int type, int id, std::string signal, int level);
void Listen_Option(int type, int id, gb::net_listen_fun f,std::string protoName);
template <typename F>
void Listen(int type, int id, F f, std::string protoName = "")
{
    gb::net_listen_fun func;
    if constexpr (std::is_same<F, sol::function>::value)
        func = gb::NetFunctionaTraits<sol::function>::make(f, protoName);
    else if constexpr (gb::HasInvokeOperator<typename std::decay<F>::type>::value)
        func = gb::ServerLambdaFunc(f, &F::operator());
    else
        func = gb::NetFunctionaTraits<F>::make(f);
    Listen_Option(type, id, std::move(func),protoName);
}

uint64_t GetSequence();
void     Call(Meta& meta, gb::RpcCallPtr call);
void     Call(Meta& meta, gb::RpcCallPtr call, std::vector<uint8_t>& data);
void     Call(Meta& meta, gb::RpcCallPtr call, std::vector<uint8_t>&& data);
void     Call(gb::RpcCallPtr call, std::string method, sol::variadic_args& args);

template <typename... Args>
void Call(gb::RpcCallPtr call, std::string method, Args&&... args)
{
    Meta meta;
    uint64_t    method_key = MD5::MD5Hash64(method.c_str());
    meta.set_method(method_key);
    uint64_t seq_id = GetSequence();
    meta.set_sequence(seq_id);
    meta.set_compress_type(CompressTypeZlib);
    meta.set_mode(MsgMode::Request);
    call->SetId(seq_id);
    if constexpr (sizeof...(args) > 0)
    {
        std::vector<uint8_t> data = gb::msgpack::pack<Args&&...>(std::forward<Args>(args)...);
        Call(meta, call, data);
    }
    else
    {
        Call(meta, call);
    }
}

void OnReceived(const gb::SessionPtr& session, const gb::ReadBufferPtr& buffer, int meta_size, int64_t data_size);



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
                co_return co_await RpcCallAwaiter<void>{[&](std::coroutine_handle<> h)noexcept {
                        call->SetCallBack([&, h]() { h.resume(); });
                        call->SetTimeout([&, h]() { h.resume(); });
                        ::Call(call, method, args...);
                    }};

			}
			else
			{
				co_return co_await RpcCallAwaiter<ResultType>{[&](std::coroutine_handle<> h, ResultType& result)noexcept {
					call->SetCallBack([&, h](ResultType r) { 
						result = std::move(r); 
						h.resume(); 
					});
					call->SetTimeout([&,h]() { h.resume(); });
					::Call(call, method, args...);
				}};
			}
        }
        else
        {
			co_return co_await RpcCallAwaiter<ResultType>{[&](std::coroutine_handle<> h, ResultType& result)noexcept {
				call->SetCallBack([&, h](T1 t,Rets... r) { 
					result = std::make_tuple(std::move(t),std::move(r)...);
					h.resume(); 
				});
				call->SetTimeout([&,h]() { h.resume(); });
				::Call(call, method, args...);
			}};
        }
    }
};

#endif