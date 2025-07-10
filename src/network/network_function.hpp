#pragma once
#include <functional>
#include <type_traits>
#include "session/session.h"
#include <sol/sol.hpp>
#include <lua.hpp>
#include "../script/script.h"
#include "../protobuf/meta.pb.h"
#include "buffer/compressed_stream.h"



namespace gb
{


//Listener
typedef std::function<void(const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta,int meta_size,int64_t data_size)> net_listen_fun;

template <class Fn, class F = Fn>
struct NetFunctionaTraits
{
	static net_listen_fun make(F fn)
	{
		return nullptr;
	}
};



template<>
struct NetFunctionaTraits<sol::function, sol::function>
{
    static net_listen_fun make(sol::function fn, std::string_view protoName)
    {
        auto lua_state = fn.lua_state();
        sol::state_view lua_view(lua_state);
        return [lua_view, fn, proto = std::string(protoName)](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) {
            if (proto.empty())
            {
                fn(session);
                return;
            }
            int  top        = lua_gettop(lua_view.lua_state());
            auto create_msg = lua_view["create_msg"];

            if (!create_msg.valid())
            {
                lua_settop(lua_view.lua_state(), top);
                return;
            }

            sol::object lua_messgae =  create_msg(proto);
            google::protobuf::Message* msg         = lua_messgae.as<google::protobuf::Message*>();
            if (!msg)
            {
                lua_settop(lua_view.lua_state(), top);
                return;
            }
            if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
            {
				if (msg->ParsePartialFromZeroCopyStream(buffer.get()))
					fn(session,lua_messgae);
            }
            else
            {
                 std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(buffer.get(), (CompressType)meta.compress_type()));
				if (msg->ParsePartialFromZeroCopyStream(is.get()))
					fn(session,lua_messgae);
            }
            lua_settop(lua_view.lua_state(), top);
        };
    }
};

template <class R,class F>
struct NetFunctionaTraits<R (*)(),F>
{
	static net_listen_fun make(F fn)
	{
        return [fn](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) {
			fn();
		};
	}
};

template <class R,class P0,class F>
struct NetFunctionaTraits<R (*)(P0),F>
{
	using tP0  = std::decay<P0>::type;
	static net_listen_fun make(F fn)
	{
        return [fn](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) {
			if constexpr (std::is_base_of<google::protobuf::Message, tP0>::value)
			{
				typename std::decay<P0>::type p0;
                
				if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
				{
					if(p0.ParsePartialFromZeroCopyStream(buffer.get()))
						fn(p0);
				}
				else
				{
                    std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(buffer.get(), (CompressType)meta.compress_type()));
                    if (p0.ParsePartialFromZeroCopyStream(is.get()))
                        fn(p0);
				}
			}
			else if constexpr (std::is_same<std::shared_ptr<Session>,tP0>::value)
			{
				fn(session);
			}
		};

	}
};


template <class R,class P0,class P1, class F>
struct NetFunctionaTraits<R (*)(P0,P1),F>
{
	using tP0 = std::decay<P0>::type;
	using tP1 = std::decay<P1>::type;
    static net_listen_fun make(F fn)
    {
        return [fn](const std::shared_ptr<Session>& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) {
            if constexpr (std::is_same<std::shared_ptr<Session>, tP0>::value && std::is_base_of<google::protobuf::Message, tP1>::value)
            {
                tP1 p1;

				if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
				{
					if (p1.ParsePartialFromZeroCopyStream(buffer.get()))
						fn(session, std::forward<P1>(p1));
				}
				else
				{
                    std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(buffer.get(), (CompressType)meta.compress_type()));
                    if (p1.ParsePartialFromZeroCopyStream(is.get()))
                        fn(session,std::forward<P1>(p1));
				}
            }
            else if constexpr (std::is_base_of<google::protobuf::Message, tP0>::value && std::is_base_of<google::protobuf::Message, tP1>::value)
            {
                typename std::decay<P0>::type p1;
				if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
				{
					if (p1.ParsePartialFromZeroCopyStream(buffer.get()))
						fn(meta, std::forward<P1>(p1));
				}
				else
				{
                    std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(buffer.get(), (CompressType)meta.compress_type()));
                    if (p1.ParsePartialFromZeroCopyStream(is.get()))
                        fn(meta, std::forward<P1>(p1));
				}

            }
        };
    }
};



template <class R, class C, class F>
struct NetFunctionaTraits<R (C::*)() const,F>
{
	static net_listen_fun make(F fn)
	{
        return [fn](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) {
			fn();
		};
	}
};




template <class R,class C,class P0,class F>
struct NetFunctionaTraits<R (C::*)(P0) const,F>
{
	using tP0 = std::decay<P0>::type;
	static net_listen_fun make(F fn)
	{
        return [fn](const SessionPtr& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) {
            if constexpr (std::is_base_of<google::protobuf::Message, tP0>::value)
            {
                typename std::decay<P0>::type p0;

                if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
                {
                    if (p0.ParsePartialFromZeroCopyStream(buffer.get()))
                        fn(p0);
                }
                else
                {
                    std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(buffer.get(), (CompressType)meta.compress_type()));
                    if (p0.ParsePartialFromZeroCopyStream(is.get()))
                        fn(p0);
                }
            }
            else if constexpr (std::is_same<std::shared_ptr<Session>, tP0>::value)
            {
                fn(session);
            }
        };
	}
};


template <class R,class C, class P0,class P1, class F>
struct NetFunctionaTraits<R (C::*)(P0,P1) const,F>
{
	using tP0 = std::decay<P0>::type;
	using tP1 = std::decay<P1>::type;
	static net_listen_fun make(F fn)
	{
        return [fn](const std::shared_ptr<Session>& session, const ReadBufferPtr& buffer, Meta& meta, int meta_size, int64_t data_size) {
            if constexpr (std::is_same<std::shared_ptr<Session>, tP0>::value && std::is_base_of<google::protobuf::Message, tP1>::value)
            {
                typename std::decay<P1>::type p1;

                if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
                {
                    if (p1.ParsePartialFromZeroCopyStream(buffer.get()))
                        fn(session, p1);
                }
                else
                {
                    std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(buffer.get(), (CompressType)meta.compress_type()));
                    if (p1.ParsePartialFromZeroCopyStream(is.get()))
                        fn(session, p1);
                }
            }
            else if constexpr (std::is_base_of<google::protobuf::Message, tP0>::value && std::is_base_of<google::protobuf::Message, tP1>::value)
            {
                typename std::decay<P0>::type p1;
                if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
                {
                    if (p1.ParsePartialFromZeroCopyStream(buffer.get()))
                        fn(meta, std::forward<P1>(p1));
                }
                else
                {
                    std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(buffer.get(), (CompressType)meta.compress_type()));
                    if (p1.ParsePartialFromZeroCopyStream(is.get()))
                        fn(meta, p1);
                }
            }
        };
	}
};





//template <class R,class F>
//struct NetFunctionaTraits<std::function<R()>, F>
//{
//	static net_listen_fun make(F fn)
//	{
//		return [fn](std::shared_ptr<Session>& session, const std::string_view data, Meta& meta) {
//			fn();
//		};
//	}
//};
//
//
//template <class R,class P0,class F>
//struct NetFunctionaTraits<std::function<R(P0)>, F>
//{
//	using tP0 = std::decay<P0>::type;
//	static net_listen_fun make(F fn)
//	{
//		return [fn](std::shared_ptr<Session>& session, const std::string_view data, Meta& meta) {
//			if constexpr (std::is_base_of<google::protobuf::Message, tP0>::value)
//			{
//				typename std::decay<P0>::type p0;
//				p0.ParseFromString(data.data());
//				fn(p0);
//			}
//			else if constexpr (std::is_same<std::shared_ptr<Session>, tP0>::value)
//			{
//				fn(session);
//			}
//		};
//	}
//};
//
//
//template <class R, class P0,class P1, class F>
//struct NetFunctionaTraits<std::function<R(P0,P1)> const, F>
//{
//	using tP0 = std::decay<P0>::type;
//	using tP1 = std::decay<P1>::type;
//	static net_listen_fun make(F fn)
//	{
//		return [fn](std::shared_ptr<Session>& session, const std::string_view data, Meta& meta) {
//			if constexpr (std::is_same<std::shared_ptr<Session>, tP0>::value && std::is_base_of<google::protobuf::Message, tP1>::value)
//			{
//				typename std::decay<P1>::type p1;
//				p1.ParseFromString(data.data());
//				fn(session, p1);
//			}
//			else if constexpr (std::is_same<std::shared_ptr<Session>, tP1>::value && std::is_base_of<google::protobuf::Message, tP0>::value)
//			{
//				typename std::decay<P0>::type p0;
//				p0.ParseFromString(data.data());
//				fn(p0, session);
//			}
//		};
//	}
//};

template <typename T>
class HasInvokeOperator {
    typedef char yes;
    typedef struct {
        char a[2];
    } no;
    template <typename TT>
    static yes HasFunc(decltype(&TT::operator()));
    template <typename TT>
    static no HasFunc(...);

public:
    static const bool value = sizeof(HasFunc<T>(nullptr)) == sizeof(yes);
};

struct ComparePriority
{
	bool operator()(const int& a, const int& b) const
	{
		return a > b;
	}
};


template <typename T,typename F>
static net_listen_fun ServerLambdaFunc(T lambda, F)
{
	return NetFunctionaTraits<typename std::decay<F>::type, T>::make(lambda);
}

}
