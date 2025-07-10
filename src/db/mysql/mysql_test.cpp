#include "mysql_test.h"
#include <boost/asio/ip/address.hpp>

MysqlConnect::MysqlConnect(boost::asio::io_context& ctx, std::string ip,std::string user, std::string passwd, std::string db_name):
    io_context_(ctx), ip_(ip),user_(user), passwd_(passwd), db_name_(db_name), ssl_ctx_(boost::asio::ssl::context::tls_client), conn_(ctx.get_executor(), ssl_ctx_)
{
}


MysqlConnect::~MysqlConnect(){
    conn_.close();
}

void MysqlConnect::Connect()
{
    
    //boost::asio::ip::tcp::resolver resolver(io_context_.get_executor());

    //auto                           endpoints = resolver.resolve(ip_, boost::mysql::default_port_string);
    Asio::ip::tcp::endpoint endpoint(Asio::ip::make_address(ip_.c_str()),3306);
    boost::mysql::handshake_params params(user_, passwd_, db_name_);
    conn_.connect(endpoint, params);
    
}

void MysqlConnect::Exection(boost::mysql::results& result, std::string sql)
{
    conn_.execute(sql, result);
}


