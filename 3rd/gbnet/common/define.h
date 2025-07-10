#pragma once
#include <cstdint>
#include <atomic>
#include "def.h"
#include "../log/net_log_help.h"

#if USE_STANDALONE_ASIO
#    include <functional>
#    include <asio/ssl.hpp>
#    define asio_bind std::bind
#    include <asio/asio.hpp>
	 namespace Asio = asio;
	 namespace ec   = asio;
#    define _(i)      std::placeholders::_##i

#else
    #include<boost/asio.hpp>
    #include<boost/asio/ssl.hpp>
    #include <boost/bind.hpp>
    namespace Asio = boost::asio;
    namespace ec   = boost::system;
    #define asio_bind boost::bind
    #define _(i)      _##i
#endif 

namespace gb
{

enum class NET_STSTUS : int8_t
{
    STATUS_INIT       = 0,
    STATUS_CONNECTING = 1,
    STATUS_CONNECTED  = 2,
    STATUS_CLOSED     = 3,
};


enum class NET_TYPE :int8_t
{
    NT_SERVER = 0,
    NT_CLIENT = 1,
    NT_NATS   = 2,        //消息中间件
};

enum class CONNECT_TYPE :int8_t
{
    CT_GATEWAY = 0,
    CT_LOGIN   = 1,
};


enum NET_ErrorCode {
    NET_SUCCESS = 0,
    NET_ERROR_PARSE_REQUEST_MESSAGE = 1,
    NET_ERROR_PARSE_RESPONSE_MESSAGE = 2,
    NET_ERROR_UNCOMPRESS_MESSAGE = 3,
    NET_ERROR_COMPRESS_TYPE = 4,
    NET_ERROR_NOT_SPECIFY_METHOD_NAME = 5,
    NET_ERROR_PARSE_METHOD_NAME = 6,
    NET_ERROR_FOUND_SERVICE = 7,
    NET_ERROR_FOUND_METHOD = 8,
    NET_ERROR_CHANNEL_BROKEN = 9,
    NET_ERROR_CONNECTION_CLOSED = 10,
    NET_ERROR_REQUEST_TIMEOUT = 11, // request timeout
    NET_ERROR_REQUEST_CANCELED = 12, // request canceled
    NET_ERROR_SERVER_UNAVAILABLE = 13, // server un-healthy
    NET_ERROR_SERVER_UNREACHABLE = 14, // server un-reachable
    NET_ERROR_SERVER_SHUTDOWN = 15,
    NET_ERROR_SEND_BUFFER_FULL = 16,
    NET_ERROR_SERIALIZE_REQUEST = 17,
    NET_ERROR_SERIALIZE_RESPONSE = 18,
    NET_ERROR_RESOLVE_ADDRESS = 19,
    NET_ERROR_CREATE_STREAM = 20,
    NET_ERROR_NOT_IN_RUNNING = 21,
    NET_ERROR_SERVER_BUSY = 22,

    // error code for listener
    NET_ERROR_TOO_MANY_OPEN_FILES = 101,

    NET_ERROR_UNKNOWN = 999,
    NET_ERROR_FROM_USER = 1000,
}; 


//是否使用资源计数
#define USE_RESOURCE_COUNTER true

#ifdef USE_RESOURCE_COUNTER

#    define DEF_RESOURCE_COUNTER(name)                       \
        static std::atomic<int>& g_resource_counter_##name() \
        {                                                    \
            static std::atomic<int> counter;                 \
            return counter;                                  \
        }

#    define RESOURCE_COUNTER_INC(name) g_resource_counter_##name().fetch_add(1)

#    define RESOURCE_COUNTER_DEC(name) g_resource_counter_##name().fetch_sub(1)

#    define GET_RESOURCE_COUNTER(name)                 \
        []() -> int {                                  \
            return g_resource_counter_##name().load(); \
        }();

#else

#    define DEF_RESOURCE_COUNTER(name) // 空定义

#    define RESOURCE_COUNTER_INC(name) // 空定义

#    define RESOURCE_COUNTER_DEC(name) // 空定义

#    define GET_RESOURCE_COUNTER(name) 0 // 返回0，表示计数器未定义

#endif

DEF_RESOURCE_COUNTER(ByteStream);
DEF_RESOURCE_COUNTER(SSLByteStream);
DEF_RESOURCE_COUNTER(Listener);


//是否使用网络日志
#define USE_NETWORK_LOG_TRACK true

#if USE_NETWORK_LOG_TRACK   
    #define NETWORK_LOG(...)     NET_LOG_INFO(__VA_ARGS__);
#else                       
    #define NETWORK_LOG(...);
#endif 




using time_point_t = std::chrono::steady_clock::time_point;
using duration_t = std::chrono::steady_clock::duration;
using IoService = Asio::io_context;
using SSLContext = Asio::ssl::context;
using IoServiceStrand = Asio::io_context::strand;
using Socket = Asio::ip::tcp::socket;
using SSLSocket = Asio::ssl::stream<Asio::ip::tcp::socket>;
using Endpoint = Asio::ip::tcp::endpoint;
using Resolver = Asio::ip::tcp::resolver; 
using Acceptor = Asio::ip::tcp::acceptor;
using Error_code = ec::error_code;


}

