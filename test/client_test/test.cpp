#include "test.h"
#include "network.h"
void hello(TestMsg& msg)
{
	LOG_INFO("index:{}  msg{}",msg.index(), msg.msg());
}

void Test_Register()
{
    gb::Listen(1, 2, hello);
}

void SendMsg1(std::shared_ptr<gb::Client> client)
{
    TestMsg msg;
    msg.set_index(111);
    msg.set_msg("gb gb gb");

    Meta meta;
    meta.set_mode(MsgMode::Msg);
    meta.set_type(1);
    meta.set_id(2);
    meta.set_compress_type(MsgCompressType::CompressZlib);

    client->Send(gb::CONNECT_TYPE::CT_GATEWAY, &meta, &msg);
}




void SendRpc(std::shared_ptr<gb::Client> client)
{
	gb::RpcCallPtr call = std::make_shared<gb::RpcCall>();
	call->SetSession(client->GetSession(gb::CONNECT_TYPE::CT_GATEWAY));
	call->SetCallBack([](int a,std::string str) {
		LOG_INFO("test lua reply: {} {}",a, str);
	});
	gb::Call(call, "test_ret_args", 2, "asadsadsadsdaefasgajf中国人大大撒大苏打 ddbgasufgsajbasadsadsadsdaefasgajf中国人大大撒大苏打 ddbgasufgsajbfasvfafasvfa");
	
}



//async_simple::coro::Lazy<> test_coro(gb::SessionPtr& session)
//{
//	gb::RpcCall call;
//	call.SetSession(session);
//	//co_await Net::CoRpcCall<std::string, std::string>(call, "lua_rpc_test_args", "helo");
//	auto str = co_await gb::CoRpcCall<std::string>(call,"lua_rpc_test_args","helo");
//	LOG_INFO("CORO_TEST  {}", str);
//    
//	auto [a, b] = co_await gb::CoRpcCall<std::tuple<int, std::string>>(call, "test_ret_args", 1, "world");
//	LOG_INFO("CORO_TEST_2  {} {}", a,b);
//
//}


//async_simple::coro::Lazy<> test_coro_2(gb::SessionPtr& session)
//{
//	gb::RpcCall call;
//	call.SetSession(session);
//	//co_await Net::CoRpcCall<std::string, std::string>(call, "lua_rpc_test_args", "helo");
//    auto str = co_await gb::CoRpc<int>::execute(call, "hello");
//	LOG_INFO("CORO_TEST  {}", str);
//    
//	auto [a, b] = co_await gb::CoRpcCall<std::tuple<int, std::string>>(call, "test_ret_args", 1, "world");
//	LOG_INFO("CORO_TEST_2  {} {}", a,b);
//
//}
