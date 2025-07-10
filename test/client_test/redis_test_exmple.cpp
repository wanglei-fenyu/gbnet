//#include "log/log_help.h"
//#include "redis_test_exmple.h"
//
//void test_create_redis_connect(boost::asio::io_context& ctx)
//{
//    boost::redis::config cfg;
//    cfg.addr = {"192.168.31.149","6379"};
//    RedisConnect *conn = new RedisConnect(ctx, cfg);
//    
//    conn->Connect();
//    
//    boost::redis::request req;
//    req.push("GET", "name");
//    conn->Exection(req);
//
// //   LOG_INFO("redis: get name  --> {}",std::get<0>(resoult).value());
//
//}