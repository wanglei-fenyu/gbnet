#pragma once
#include "common/def.h"
#include "common/define.h"
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/handshake_params.hpp>
#include <boost/mysql/results.hpp>
#include <boost/mysql/tcp_ssl.hpp>

#include <boost/system/system_error.hpp>
#include <boost/asio/ssl/context.hpp>

#include <iostream>


class MysqlConnect
{
public:
    MysqlConnect(boost::asio::io_context& ctx, std::string ip,std::string user ,std::string passwd, std::string db_name);
    ~MysqlConnect();

public:
    void Connect();
    void Exection(boost::mysql::results& result,std::string sql);


private:
    std::string              ip_;
    std::string              user_;
    std::string              passwd_;
    std::string              db_name_;


    Asio::io_context& io_context_;
    Asio::ssl::context        ssl_ctx_;
    boost::mysql::tcp_ssl_connection conn_;
};

