#pragma once 
#include <sol/sol.hpp>
#include "../msgpack/msgpack.hpp"
#include "../session/session.h"

namespace gb
{


class RpcReply : public std::enable_shared_from_this<RpcReply>
{
public:
    RpcReply(Meta& meta, const std::shared_ptr<Session>& session);
    RpcReply(Meta&& meta, const std::shared_ptr<Session>& session);
    void                            Send(const std::vector<uint8_t>& data);
    const std::shared_ptr<Session>& GetSession() const;
    bool                            Valid() const;
    
    void Invoke();
    template <typename... Args>
    void Invoke(Args&... args);

    template <typename... Args>
    void Invoke(Args&&... args);

    void Invoke(sol::variadic_args args);

    Meta& GetMeta();

private:
    Meta              meta_;
    std::shared_ptr<Session> session_;
};

template <typename... Args>
inline void RpcReply::Invoke(Args&... args)
{
    if (!Valid())
        return;

    std::vector<uint8_t> data = gb::msgpack::pack<Args&&...>(std::forward<Args>(args)...);
    Send(data);
}

template <typename... Args>
inline void RpcReply::Invoke(Args&&... args)
{
    if (!Valid())
        return;

    std::vector<uint8_t> data = gb::msgpack::pack<Args&&...>(std::forward<Args>(args)...);
    Send(data);
}

} // namespace gb
