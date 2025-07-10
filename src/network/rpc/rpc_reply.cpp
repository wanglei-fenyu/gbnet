#include "rpc_reply.h"
#include "rpc_call.h"
#include "log/log_help.h"
namespace gb
{

RpcReply::RpcReply(Meta& meta, const std::shared_ptr<Session>& session) :
    meta_(meta), session_(session)
{
    meta_.set_mode(MsgMode::Response);
}

RpcReply::RpcReply(Meta&& meta, const std::shared_ptr<Session>& session) :
    meta_(meta), session_(session)
{
    meta_.set_mode(MsgMode::Response);
}

void RpcReply::Send(const std::vector<uint8_t>& data)
{
    if (session_ && Valid())
    {
        SequenceId Id;
        Id.value = meta_.sequence();
        session_->Send(&meta_, data);
    }
}

const std::shared_ptr<Session>& RpcReply::GetSession() const
{
    return session_;
}

bool RpcReply::Valid() const
{
    return (session_ && !session_->is_closed() && session_->is_connected());
}

void RpcReply::Invoke(sol::variadic_args args)
{
    const std::vector<uint8_t> data = gb::msgpack::pack(std::move(args));
    Send(data);
}


void RpcReply::Invoke()
{
    if (!Valid())
        return;
    std::vector<uint8_t> data;
    Send(data);

}

Meta& RpcReply::GetMeta()
{
    return meta_;
}

} // namespace gb
