#pragma once
#include <lua.hpp>
#include <sol/sol.hpp>
#include <google/protobuf/message.h>
#include <gbnet/buffer/buffer.h>

void OnScriptError(sol::error&& err);
class Script : public sol::state
{
public:
	Script() = default;
public:
    std::string ProtoEncodeToString(const char* proto, sol::table& tbl);
    gb::ReadBufferPtr ProtoEncode(const char* proto, sol::table& tbl);
    sol::table ProtoDecode(const char* proto, std::string_view data);
    sol::table ProtoDecode(const char* proto, const gb::ReadBufferPtr& buffer);
public:
	void Load(std::string filename);
public:
	sol::table ProtobufTransTable(const google::protobuf::Message&& msg);
	google::protobuf::Message* TableTransProtobuf(const char* protoName,sol::table& table);
	void StackDump();

private:
	void PrintTable(lua_State* pState, int index);

};

