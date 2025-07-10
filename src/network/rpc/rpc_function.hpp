#pragma once 
#include <functional>
#include <type_traits>
#include "../session/session.h"
#include <sol/sol.hpp>
#include <lua.hpp>
#include "../../script/script.h"
#include "../../protobuf/meta.pb.h"
#include "rpc_reply.h"
#include "../msgpack/msgpack.hpp"
#include <gbnet/buffer/compressed_stream.h>
#include "rpc_function_help.h"

namespace gb
{


typedef std::function<void(const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size)> rpc_listen_fun;

template <typename T>
class HasCallOperator
{
    typedef char yes;
    typedef struct
    {
        char a[2];
    } no;

    template <typename TT>
    static yes HasFunc(decltype(&TT::operator()));
    template <typename TT>
    static no HasFunc(...);

public:
    static const bool value = sizeof(HasFunc<T>(nullptr)) == sizeof(yes);
};




template <class Fn, class F = Fn>
struct RpcFunctionaTraits
{
    static rpc_listen_fun make(F fp)
    {
        static_assert("rpc function invalid");
        return nullptr;
    }
};




template <>
struct RpcFunctionaTraits<sol::function, sol::function>
{
    static rpc_listen_fun make(sol::state* state, sol::function func)
    {
        //auto            lua_state = func.lua_state();
        //sol::state_view lua_view(lua_state);

        return [state, func](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            int      top = lua_gettop(state->lua_state());
			std::string s = "";
			GetMsgData(meta, buffer, meta_size, data_size,s);
			RpcReply reply(std::move(meta), session);
            if (data_size > 0)
            {
                sol::variadic_args arg = gb::msgpack::unpack(*state, (uint8_t*)s.data(), s.size());
                sol::protected_function_result result = func(reply, arg);
                if (!result.valid())
                {
                    OnScriptError(result);
                }
            }
            else
            {
                sol::protected_function_result result = func(reply);
                if (!result.valid())
                {
                    OnScriptError(result);
                }
            }
            lua_settop(state->lua_state(), top);
        };
    }
};

template <class R, class F>
struct RpcFunctionaTraits<R (*)(), F>
{
    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            f();
        };
    }
};


template <class R, class F, class P0>
struct RpcFunctionaTraits<R (*)(P0), F>
{
    typedef std::decay<P0>::type PP0;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            if constexpr (std::is_base_of<google::protobuf::Message, PP0>::value)
            {
                PP0 p0;
                if (p0.ParsePartialFromZeroCopyStream(buffer.get()))
                {
                    f(std::forward<P0>(p0));
                }
            }
            else if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                f(RpcReply(std::move(meta), session));
            }
            else
            {
                std::string s = "";
                GetMsgData(meta, buffer, meta_size, data_size,s);
                auto [p0] = gb::msgpack::unpack<PP0>((uint8_t*)s.data() , s.size());
                f(std::forward<P0>(p0));
            }
        };
    }
};

template <class R, class F, class P0, class P1>
struct RpcFunctionaTraits<R (*)(P0, P1), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {

                if constexpr (std::is_base_of<google::protobuf::Message, PP1>::value)
                {
                    PP1 p1;
                    if (p1.ParsePartialFromZeroCopyStream(buffer.get()))
                    {
                        f(RpcReply(std::move(meta), session), std::forward<P1>(p1));
                    }
                }
                else
                {

                    std::string s = "";
                    GetMsgData(meta, buffer, meta_size, data_size,s);
					auto [p1] = gb::msgpack::unpack<PP1>((uint8_t*)s.data(), s.size());
					f(RpcReply(std::move(meta), session), std::forward<P1>(p1));
                }
            }
            else
            {
                std::string s = "";
                GetMsgData(meta, buffer, meta_size, data_size,s);
                auto [p0, p1] = gb::msgpack::unpack<PP0, PP1>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1));
            }
        };
    }
};


template <class R, class F, class P0, class P1, class P2>
struct RpcFunctionaTraits<R (*)(P0, P1, P2), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;
    typedef std::decay<P2>::type PP2;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            std::string s = "";
            GetMsgData(meta, buffer, meta_size, data_size,s);
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                auto [p1, p2] = gb::msgpack::unpack<PP1, PP2>((uint8_t*)s.data(), s.size());
                f(RpcReply(std::move(meta), session), std::forward<P1>(p1),std::forward<P2>(p2));
            }
            else
            {
                auto [p0, p1, p2] = gb::msgpack::unpack<PP0,PP1, PP2>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2));
            }
        };
    }
};


template <class R, class F, class P0, class P1, class P2, class P3>
struct RpcFunctionaTraits<R (*)(P0, P1, P2, P3), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;
    typedef std::decay<P2>::type PP2;
    typedef std::decay<P3>::type PP3;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            std::string s = "";
            GetMsgData(meta, buffer, meta_size, data_size,s);
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                auto [p1, p2,p3] = gb::msgpack::unpack<PP1, PP2,PP3>((uint8_t*)s.data(), s.size());
                f(RpcReply(std::move(meta), session), std::forward<P1>(p1),std::forward<P2>(p2),std::forward<P2>(p3));
            }
            else
            {
                auto [p0, p1, p2,p3] = gb::msgpack::unpack<PP0,PP1, PP2,PP3>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2),std::forward<P2>(p3));
            }
        };
    }
};




template <class R, class F, class P0, class P1, class P2, class P3,class P4>
struct RpcFunctionaTraits<R (*)(P0, P1, P2, P3, P4), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;
    typedef std::decay<P2>::type PP2;
    typedef std::decay<P3>::type PP3;
    typedef std::decay<P4>::type PP4;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            std::string s = "";
            GetMsgData(meta, buffer, meta_size, data_size,s);
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                auto [p1, p2,p3,p4] = gb::msgpack::unpack<PP1, PP2, PP3, PP4>((uint8_t*)s.data(), s.size());
                f(RpcReply(std::move(meta), session), std::forward<P1>(p1),std::forward<P2>(p2),std::forward<P3>(p3),std::forward<P4>(p4));
            }
            else
            {
                auto [p0, p1, p2,p3,p4] = gb::msgpack::unpack<PP0, PP1, PP2, PP3, PP4>((uint8_t*)s.data(),s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2),std::forward<P3>(p3),std::forward<P4>(p4));
            }
        };
    }
};


template <class R, class F, class P0, class P1, class P2, class P3,class P4,class P5>
struct RpcFunctionaTraits<R (*)(P0, P1, P2, P3, P4, P5), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;
    typedef std::decay<P2>::type PP2;
    typedef std::decay<P3>::type PP3;
    typedef std::decay<P4>::type PP4;
    typedef std::decay<P5>::type PP5;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            std::string s = "";
            GetMsgData(meta, buffer, meta_size, data_size,s);
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                auto [p1,p2,p3,p4,p5] = gb::msgpack::unpack<PP1, PP2, PP3, PP4, PP5>((uint8_t*)s.data(), s.size());
                f(RpcReply(std::move(meta), session), std::forward<P1>(p1),std::forward<P2>(p2),std::forward<P3>(p3),std::forward<P4>(p4),std::forward<P5>(p5));
            }
            else
            {
                auto [p0, p1, p2,p3,p4,p5] = gb::msgpack::unpack<PP0, PP1, PP2, PP3, PP4,PP5>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2),std::forward<P3>(p3),std::forward<P4>(p4),std::forward<P5>(p5));
            }
        };
    }
};












template <class R, class C, class F>
struct RpcFunctionaTraits<R (C::*)() const, F>
{
    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            f();
        };
    }
};


template <class R, class C, class F, class P0>
struct RpcFunctionaTraits<R (C::*)(P0) const, F>
{
    typedef std::decay<P0>::type PP0;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            if constexpr (std::is_base_of<google::protobuf::Message, PP0>::value)
            {
                PP0 p0;
                if (p0.ParsePartialFromZeroCopyStream(buffer.get()))
                {
                    f(std::forward<P0>(p0));
                }
            }
            else if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                f(RpcReply(std::move(meta), session));
            }
            else
            {
                std::string s = "";
                GetMsgData(meta, buffer, meta_size, data_size,s);
                auto [p0] = gb::msgpack::unpack<PP0>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0));
            }
        };
    }
};

template <class R, class C, class F, class P0, class P1>
struct RpcFunctionaTraits<R (C::*)(P0, P1) const, F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {

                if constexpr (std::is_base_of<google::protobuf::Message, PP1>::value)
                {
                    PP1 p1;
                    if (p1.ParsePartialFromZeroCopyStream(buffer.get()))
                    {
                        f(RpcReply(std::move(meta), session), std::forward<P1>(p1));
                    }
                }
                else
                {
                    std::string s = "";
                    GetMsgData(meta, buffer, meta_size, data_size,s);
                    auto [p1] = gb::msgpack::unpack<PP1>((uint8_t*)s.data(), s.size());
                    f(RpcReply(std::move(meta), session), std::forward<P1>(p1));
                }
            }
            else
            {
                std::string s = "";
                GetMsgData(meta, buffer, meta_size, data_size,s);
                auto [p0, p1] = gb::msgpack::unpack<PP0, PP1>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1));
            }
        };
    }
};



template <class R,class C, class F, class P0, class P1, class P2>
struct RpcFunctionaTraits<R (C::*)(P0, P1, P2), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;
    typedef std::decay<P2>::type PP2;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            std::string s = "";
            GetMsgData(meta, buffer, meta_size, data_size,s);
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                auto [p1, p2] = gb::msgpack::unpack<PP1, PP2>((uint8_t*)s.data(), s.size());
                f(RpcReply(std::move(meta), session), std::forward<P1>(p1),std::forward<P2>(p2));
            }
            else
            {
                auto [p0, p1, p2] = gb::msgpack::unpack<PP0,PP1, PP2>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2));
            }
        };
    }
};


template <class R, class C,class F, class P0, class P1, class P2, class P3>
struct RpcFunctionaTraits<R (C::*)(P0, P1, P2, P3), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;
    typedef std::decay<P2>::type PP2;
    typedef std::decay<P3>::type PP3;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            std::string s = "";
            GetMsgData(meta, buffer, meta_size, data_size,s);
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                auto [p1, p2,p3] = gb::msgpack::unpack<PP1, PP2,PP3>((uint8_t*)s.data(), s.size());
                f(RpcReply(std::move(meta), session), std::forward<P1>(p1),std::forward<P2>(p2),std::forward<P2>(p3));
            }
            else
            {
                auto [p0, p1, p2,p3] = gb::msgpack::unpack<PP0,PP1, PP2,PP3>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2),std::forward<P2>(p3));
            }
        };
    }
};



template <class R,class C, class F, class P0, class P1, class P2, class P3,class P4>
struct RpcFunctionaTraits<R (C::*)(P0, P1, P2, P3, P4), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;
    typedef std::decay<P2>::type PP2;
    typedef std::decay<P3>::type PP3;
    typedef std::decay<P4>::type PP4;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            std::string s = "";
            GetMsgData(meta, buffer, meta_size, data_size,s);
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                auto [p1, p2,p3,p4] = gb::msgpack::unpack<PP1, PP2, PP3, PP4>((uint8_t*)s.data(), s.size());
                f(RpcReply(std::move(meta), session), std::forward<P1>(p1),std::forward<P2>(p2),std::forward<P3>(p3),std::forward<P4>(p4));
            }
            else
            {
                auto [p0, p1, p2,p3,p4] = gb::msgpack::unpack<PP0, PP1, PP2, PP3, PP4>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2),std::forward<P3>(p3),std::forward<P4>(p4));
            }
        };
    }
};


template <class R, class C,class F, class P0, class P1, class P2, class P3,class P4,class P5>
struct RpcFunctionaTraits<R (C::*)(P0, P1, P2, P3, P4, P5), F>
{
    typedef std::decay<P0>::type PP0;
    typedef std::decay<P1>::type PP1;
    typedef std::decay<P2>::type PP2;
    typedef std::decay<P3>::type PP3;
    typedef std::decay<P4>::type PP4;
    typedef std::decay<P5>::type PP5;

    static rpc_listen_fun make(F f)
    {
        return [f](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) -> void {
            std::string s = "";
            GetMsgData(meta, buffer, meta_size, data_size,s);
            if constexpr (std::is_same<RpcReply, PP0>::value)
            {
                auto [p1,p2,p3,p4,p5] = gb::msgpack::unpack<PP1, PP2, PP3, PP4, PP5>((uint8_t*)s.data(),s.size());
                f(RpcReply(std::move(meta), session), std::forward<P1>(p1),std::forward<P2>(p2),std::forward<P3>(p3),std::forward<P4>(p4),std::forward<P5>(p5));
            }
            else
            {
                auto [p0, p1, p2, p3, p4, p5] = gb::msgpack::unpack<PP0, PP1, PP2, PP3, PP4, PP5>((uint8_t*)s.data(), s.size());
                f(std::forward<P0>(p0), std::forward<P1>(p1), std::forward<P2>(p2),std::forward<P3>(p3),std::forward<P4>(p4),std::forward<P5>(p5));
            }
        };
    }
};


template <typename T, typename F>
static rpc_listen_fun RpcLambdaFunc(T lambda, F)
{
    return RpcFunctionaTraits<typename std::decay<F>::type, T>::make(lambda);
}

} // namespace gb
