#pragma once
#include "network/network.h"
#include "protobuf/msg.pb.h"

void hello(const std::shared_ptr<gb::Session>& session);

void World(const std::shared_ptr<gb::Session>& session, TestMsg& msg);

void test_rpc(gb::RpcReply reply);

void test_rpc2(int a);

void square(gb::RpcReply reply, int a);

void test_ret_args(gb::RpcReply reply, int a, std::string b);

void SessionMsg(const gb::SessionPtr& session, TestMsg& msg);



struct test_msg : public PACKER_BASE
{
	int age;
	std::string name;
	double wit;
	REGISTER_PACKER(age, name);
};


struct test_msg2 : public PACKER_BASE
{
	int age;
	std::string name;
	REGISTER_PACKER(age, name);
};


void Test_Register();