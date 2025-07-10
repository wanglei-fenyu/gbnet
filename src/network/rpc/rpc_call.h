#pragma once 
#include "../session/session.h"
#include "rpc_function.hpp"
#include "../network_function.hpp"



namespace gb
{

constexpr int64_t kRpcdefaultTimeout = 1000 * 5; //5秒

typedef rpc_listen_fun rpc_done_call;


enum class RpcErrorCode {
    None = 0,
    Timeout = 1,
    Cancel = 2,
    InvalidRequest = 3,
};

union SequenceId
{
    struct
    {
        uint64_t index : 32;
        uint64_t seq : 32;
    };
    uint64_t value;
};


class RpcCall : public std::enable_shared_from_this<RpcCall>
{
public:
    RpcCall();
    ~RpcCall();
    void                      SetId(uint64_t id) { id_ = id; }
    uint64_t                  GetId() { return id_; }
    void                      SetTimeout(std::function<void()> timeout_fun, int64_t timeout = kRpcdefaultTimeout);
    void                      SetTimeout(int64_t timeout);
    void                      SetSession(const std::shared_ptr<Session>& session);
    std::shared_ptr<Session>& GetSession() { return session_; }
    void                      Call(Meta& meta,const ReadBufferPtr buffer = nullptr);
    //void                      Call(Meta& meta, std::vector<uint8_t>& data);
    void                      Cancel();
    bool                      HasCallBack() const;
    bool                      HasSession();
    void                      Done(const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) const;
    bool                      IsError();
    RpcErrorCode              ErrorCode();
    template <class F>
    void SetCallBack(F f);

private:
    void StartTimer();

private:
    uint64_t                  id_;             //唯一标识
    //std::chrono::milliseconds timeout_;        //超时时间
    //int64_t                   timer_id_;       //计时器id
    mutable std::optional<Asio::steady_timer> timer_;
    std::chrono::steady_clock::duration timeout_;       //使用毫秒
    std::function<void()>     timeout_func_;   //超时回调
    bool                      is_cancel_;      //是否已经取消
    std::shared_ptr<Session>  session_;        //网络会话
    rpc_done_call             done_call_bcak_; //回调函数
    RpcErrorCode              error_code_;     //错误码
};

template <class F>
inline void RpcCall::SetCallBack(F f)
{
    rpc_listen_fun func;
    if constexpr (std::is_same<F, sol::function>::value)
    {
        auto            lua_state = f.lua_state();
        sol::state_view lua_view(lua_state);
        sol::state*     state = (sol::state*)&lua_view;
        func                  = RpcFunctionaTraits<sol::function>::make(state, f);
    }
    else if constexpr (HasInvokeOperator<typename std::decay<F>::type>::value)
        func = RpcLambdaFunc(f, &F::operator());
    else
    {
        func = RpcFunctionaTraits<F>::make(f);
    }
    done_call_bcak_ = func;
}


using RpcCallPtr = std::shared_ptr<RpcCall>;
} // namespace gb
