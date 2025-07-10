#include "script.h"
#include "register_script.h"
#include "../network/network.h"
#include "../network/msgpack/msgpack.hpp"
#include "../network/session/session.h"
#include "log/log_help.h"
#include <filesystem>
using namespace gb;
static void register_net(std::shared_ptr<Script>& scriptPtr)
{
	auto network		= scriptPtr->create_table("net");
	network["Listen"]	= [](int type, int id, sol::function  f,std::string protoName = "") {  gb::Listen(type, id, f, protoName); };
	network["UnListen"] = [](int type, int id, std::string signal, int level = 0) { gb::UnListen(type, id, signal, level); };
    network["Send"]     = sol::overload([](Session* session, int type, int id, std::string protoName, sol::object lua_msg) {
											google::protobuf::Message* messgae = lua_msg.as<google::protobuf::Message*>();
											if (messgae)
											{
												gb::Send(session, type, id, *messgae);
											}
										},
										[](std::shared_ptr<Session> session, int type, int id, std::string protoName, sol::object lua_msg) {
                                        google::protobuf::Message* messgae = lua_msg.as<google::protobuf::Message*>();
											if (messgae)
											{
												gb::Send(session, type, id, *messgae);
											}
										});
	network["Register"]	= [](std::string method, sol::function f) { gb::Register(method, f); };
	network["Call"]		= [](RpcCallPtr call,std::string method,sol::variadic_args args) {gb::Call(call, method, args);};


	scriptPtr->new_usertype<Session>("Session");
	scriptPtr->new_usertype<RpcCall>("RpcCall",
									"new",sol::constructors<RpcCall>(),
									"SetSession",&RpcCall::SetSession,
									"SetCallBack",&RpcCall::SetCallBack<sol::function>);

	scriptPtr->new_usertype<RpcReply>("RpcReply",
									"new",sol::constructors<RpcReply(Meta&, const std::shared_ptr<Session>&), RpcReply(Meta&&, const std::shared_ptr<Session>&)>(),
									"Invoke",sol::overload(static_cast<void(RpcReply::*)(sol::variadic_args)>(&RpcReply::Invoke))  );

	scriptPtr->script(R"(   
	function create_msg(proto_name)
		message = _G[proto_name]
		if message then
			return message.new()
		else
			log.Error("message not found for "..proto_name)
			return nil
		end
	end
    )");

}


static void register_log(std::shared_ptr<Script>& scriptPtr)
{
    auto logger = spdlog::get(LOG_NAME);
    if (!logger)
        return;
    auto log    = scriptPtr->create_table("log");
    log["Info"] = [&scriptPtr](std::string str) {
		sol::state_view lua(scriptPtr->lua_state());
        sol::table debug_info = lua["debug"]["getinfo"](2, "Sl");
		std::string file_path = debug_info["short_src"];
		std::string file_name = std::filesystem::path(file_path).filename().string();
        int line = debug_info["currentline"];
        LOG_INFO(file_name+":"+std::to_string(line)+ "|" + str)
    };
    log["Error"] = [&scriptPtr](std::string str) {
		sol::state_view lua(scriptPtr->lua_state());
        sol::table debug_info = lua["debug"]["getinfo"](2, "Sl");
		std::string file_path = debug_info["short_src"];
		std::string file_name = std::filesystem::path(file_path).filename().string();
        int line = debug_info["currentline"];
        LOG_ERROR(file_name + ":" + std::to_string(line) + "|" + str)
    };
    log["Warning"] = [&scriptPtr](std::string str) {
		sol::state_view lua(scriptPtr->lua_state());
        sol::table debug_info = lua["debug"]["getinfo"](2, "Sl");
		std::string file_path = debug_info["short_src"];
		std::string file_name = std::filesystem::path(file_path).filename().string();
        int line = debug_info["currentline"];
        LOG_WARN( file_name + ":" + std::to_string(line) + "|" + str)
    };



}

static void register_msgpack(std::shared_ptr<Script>& scriptPtr)
{
	using namespace gb::msgpack;
	auto msgpack = scriptPtr->create_table("msgpack");
	msgpack["pack"] = sol::overload(
		[](sol::variadic_args args)->std::vector<uint8_t> {return std::move(pack(args)); },
		[](sol::variadic_args&& args)->std::vector<uint8_t> {return pack(std::forward<sol::variadic_args&&>(args)); },
		[](sol::protected_function_result& args)->std::vector<uint8_t> {return std::move(pack(args)); }
		//[](sol::protected_function_result&& args)->std::vector<uint8_t> {return std::move(pack(std::move(args))); }
	);
	msgpack["unpack"] = sol::overload(
	/*	[](sol::state_view& state, const uint8_t* dataStart, const std::size_t size)->sol::variadic_args {return unpack(state, dataStart, size); },
		[](sol::state_view& state, const uint8_t* dataStart, const std::size_t size, std::error_code& ec)->sol::variadic_args {return unpack(state, dataStart, size,ec); },
		[](sol::state_view& state, const std::vector<uint8_t>& data)->sol::variadic_args {return unpack(state, data); },
		[](sol::state_view& state, const std::vector<uint8_t>& data,  std::error_code& ec)->sol::variadic_args {return unpack(state, data, ec); }*/
		[scriptPtr](const uint8_t* dataStart, const std::size_t size)->sol::variadic_args {return unpack((sol::state_view&)(*scriptPtr), dataStart, size); },
		[scriptPtr](const uint8_t* dataStart, const std::size_t size, std::error_code& ec)->sol::variadic_args {return unpack((sol::state_view&)(*scriptPtr), dataStart, size,ec); },
		[scriptPtr](const std::vector<uint8_t>& data)->sol::variadic_args {return unpack((sol::state_view&)(*scriptPtr), data); },
		[scriptPtr](const std::vector<uint8_t>& data,  std::error_code& ec)->sol::variadic_args {return unpack((sol::state_view&)(*scriptPtr), data, ec); }

	);
	
	scriptPtr->new_usertype<std::vector<uint8_t>>("vec_uint8");
}

extern void register_proto_msg(std::shared_ptr<Script>& scriptPtr);

void register_script(std::shared_ptr<Script>& scriptPtr)
{
	register_log(scriptPtr);
	register_msgpack(scriptPtr);
    register_proto_msg(scriptPtr);
	register_net(scriptPtr);
	
}
