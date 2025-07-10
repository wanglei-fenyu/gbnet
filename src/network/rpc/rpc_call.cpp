#include "rpc_call.h"
#include "../../common/timer_help.h"
#include "../../common/worker/worker_manager.h"
#include "log/log_help.h"
namespace gb
{
extern void RpcCancel(int64_t seq_id);
RpcCall::RpcCall() :
    id_(0), timeout_(std::chrono::milliseconds(kRpcdefaultTimeout)), timeout_func_(nullptr), is_cancel_(false), session_(nullptr), done_call_bcak_(nullptr), timer_(std::nullopt), error_code_(RpcErrorCode::None)
{
}

RpcCall::~RpcCall()
{
    if (timer_ && timer_->expiry() > std::chrono::steady_clock::now())
    {
        timer_->cancel();
    }
}

void RpcCall::SetTimeout(std::function<void()> timeout_fun, int64_t timeout)
{
    timeout_func_ = timeout_fun;
    SetTimeout(timeout);
};

void RpcCall::SetTimeout(int64_t timeout) 
{
    timeout_ = std::chrono::milliseconds(timeout);
    if (!timer_ && HasSession()) {
         timer_ = Asio::steady_timer(GetSession()->ioservice());
    }
}

void RpcCall::SetSession(const std::shared_ptr<Session>& session)
{
    session_ = session;
    if (!timer_ && session_) {
         timer_ = Asio::steady_timer(GetSession()->ioservice());
    }
}

void RpcCall::Call(Meta& meta, const ReadBufferPtr buffer /*= nullptr*/)
{
	error_code_ = RpcErrorCode::None;
	StartTimer();
    if (session_)
    {
        if (buffer)
            session_->Send(&meta, buffer);
        else
            session_->Send(&meta);

    }
}



void RpcCall::Cancel()
{
    is_cancel_ = true;
    if (timer_ && timer_->expiry() > std::chrono::steady_clock::now())
    {
        timer_->cancel();
    }
	SequenceId Id;
    Id.value    = GetId();
	auto worker = WorkerManager::Instance()->GetWorker(Id.index);
	if (worker)
	{
		worker->Post([self = shared_from_this()]() { RpcCancel(self->GetId()); });
	}
}

bool RpcCall::HasCallBack() const
{
    return done_call_bcak_ != nullptr;
}

bool RpcCall::HasSession()
{
    if (!session_)
        return false;
    if (session_->is_closed() || session_->is_connected())
        return false;
    return true;
}

void RpcCall::Done(const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) const
{
    // 取消定时器
    if (timer_ && timer_->expiry() > std::chrono::steady_clock::now())
    {
        timer_->cancel();
    }


    if (is_cancel_ || !HasCallBack())
        return;
    done_call_bcak_(session,buffer,meta, meta_size,data_size);
}

bool RpcCall::IsError()
{
    return error_code_ != RpcErrorCode::None;
}

RpcErrorCode RpcCall::ErrorCode() 
{
    return error_code_;
}

void RpcCall::StartTimer()
{
    if (!session_ || !timer_)
    {
        error_code_ = RpcErrorCode::InvalidRequest;
        Cancel();
    }
    is_cancel_ = false;
    timer_->expires_after(timeout_);
	timer_->async_wait([self = shared_from_this()](const Error_code& error) {
        if (self->is_cancel_ || error == Asio::error::operation_aborted) {
            return;
        }
        SequenceId Id;
        Id.value    = self->GetId();
        auto worker = WorkerManager::Instance()->GetWorker(Id.index);
		if (worker)
		{
			worker->Post([self] {self->Cancel();});
		}

		LOG_WARN("RPC timeout {}", self->id_);
		if (!error && !self->is_cancel_ && self->timeout_func_) {
				self->timeout_func_();
				self->error_code_ = RpcErrorCode::Timeout;
		}
	});
}

} // namespace gb
