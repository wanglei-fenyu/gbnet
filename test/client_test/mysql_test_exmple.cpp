#include "mysql_test_exmple.h"
#include "log/log_help.h"
void test_create_connect(boost::asio::io_context& ctx)
{
    MysqlConnect mysql_connect(ctx, "192.168.31.149", "root", "wanglei", "hello");
    boost::mysql::results res;
    mysql_connect.Connect();
    mysql_connect.Exection(res, "select * from demo;");
 
    for (auto e : res.rows())
    { 
        LOG_INFO("| {} |",e.back().as_string());
    }
}