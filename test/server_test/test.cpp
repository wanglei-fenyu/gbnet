#include "test.h"
#include "common/worker/worker_manager.h"

void hello(const std::shared_ptr<gb::Session>& session)
{
	LOG_INFO("Hello")
}

void World(const std::shared_ptr<gb::Session>& session,TestMsg& msg)
{
	Meta meta;
	meta.set_id(2);
	meta.set_type(1);
	session->Send(&meta,&msg);
	LOG_INFO("wrold {}",msg.msg())
}

void test_rpc(gb::RpcReply reply)
{
    LOG_INFO("test_rpc");
    reply.Invoke();
}



void test_rpc2(int a)
{
	LOG_INFO("test_rpc {}",a)
}


void square(gb::RpcReply reply, int a)
{
	LOG_INFO("square {}",a)
	reply.Invoke(a*a);
}

void test_ret_args(gb::RpcReply reply,int a,std::string b)
{
    LOG_INFO("{}:{}", a, b);
    reply.GetMeta().set_compress_type(MsgCompressType::CompressNone);
    reply.Invoke(a*2, b+" hello");
}

void SessionMsg(const gb::SessionPtr& session,TestMsg& msg)
{
	LOG_INFO("Session")
}


void Test_Register()
{
    auto wm = gb::WorkerManager::Instance();
	for (auto w : wm->GetWorkers())
	{
        w->Post([]() {
			gb::Register("test_rpc", test_rpc);
			gb::Register("test_rpc2", test_rpc2);
			gb::Register("square", square);
			gb::Register("test_ret_args", test_ret_args);
        });
	}
}
