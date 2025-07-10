#pragma once
#include "protobuf/meta.pb.h"
#include "protobuf/msg.pb.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "network/session/session.h"
#include "network/network.h"
#include "log/log_help.h"
#include "network.h"
#include "network/net/client.h"
void hello(TestMsg& msg);

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

void Test_Register();

void SendMsg1(std::shared_ptr<gb::Client> client);
void SendRpc(std::shared_ptr<gb::Client> client);
