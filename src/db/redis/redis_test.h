#pragma once
#include "common/def.h"
#include "common/define.h"
#include <boost/redis/connection.hpp>
class  RedisConnect
{
public:
    RedisConnect(Asio::io_context& ctx,boost::redis::config cfg);
    ~RedisConnect();
    
	void Connect();
    void Exection(boost::redis::request& req, boost::redis::response<std::string>& resp);

private:
    Asio::io_context& io_context_;
    boost::redis::connection  conn_;

    boost::redis::config cfg_;

};
