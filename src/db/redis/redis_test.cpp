#include "redis_test.h"
#include "log/log_help.h"
#include <boost/redis/src.hpp>


RedisConnect::RedisConnect(Asio::io_context& ctx, boost::redis::config cfg) :
    io_context_(ctx), conn_(io_context_), cfg_(cfg)
{
}

RedisConnect::~RedisConnect()
{
    conn_.cancel();
}

void RedisConnect::Connect()
{
    try
    {
       conn_.async_run(cfg_, {}, Asio::detached);
    }
    catch (...)
    {
}
    }

void RedisConnect::Exection(boost::redis::request& req,boost::redis::response<std::string>& resp)
{
    try
    {
		//conn_.async_exec(req, resp,Asio::deferred);
		conn_.async_exec(req, resp,[&](auto ec, auto)
		{
			if (!ec)
				LOG_INFO("redis: get name  --> {}", std::get<0>(resp).value());
		});
    }
    catch (std::exception const& e)
    {
        LOG_ERROR(e.what());
    }
}



