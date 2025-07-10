#include "script.h"
#include "log/log_help.h"
#include "lua_pb_parse.h"

void Script::StackDump()
{
	lua_State* L = this->lua_state();
	int i;
	int top = lua_gettop(L);
	LOG_INFO("------start----- :{}",top);

	for (i = 1; i <= top; i++)
	{
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TSTRING:
			LOG_INFO("type:{} value:{}", lua_typename(L, t),lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
		{
			std::string _bool(lua_toboolean(L, i) ? "true" : "false");
			LOG_INFO("type:{} value:{}",lua_typename(L, t),_bool.c_str());
			break;
		}
		case LUA_TNUMBER:
			LOG_INFO("type:{} value:{}",lua_typename(L, t),lua_tonumber(L, i));
			break;
		case LUA_TTABLE:
			PrintTable(L, i);
			break;
		default:
			LOG_INFO("type:{}", lua_typename(L, t));
			break;
		}
	}
	LOG_INFO("------end----");
}

void Script::PrintTable(lua_State* pState, int index)
{
	int top = lua_gettop(pState);

	/// 把栈上给定索引处的元素作一个副本压栈。
	lua_pushvalue(pState, index);

	fprintf(stdout, "{\n");

	lua_pushnil(pState);
	int len = 0;
	while (lua_next(pState, -2))
	{
		len = lua_gettop(pState);
		fprintf(stdout, "\t");
		int type = lua_type(pState, -2);
		switch (type)
		{
		case LUA_TNUMBER:
			fprintf(stdout, "%g", lua_tonumber(pState, -2));
			break;
		case LUA_TBOOLEAN:
			fprintf(stdout, "%d", int(lua_toboolean(pState, -2)));
			break;
		case LUA_TSTRING:
			fprintf(stdout, "%s", lua_tostring(pState, -2));
			break;
		default:
			fprintf(stdout, "%s:%p", lua_typename(pState, type), lua_topointer(pState, -2));
			break;
		}

		fprintf(stdout, "\t\t=\t");

		type = lua_type(pState, -1);
		switch (type)
		{
		case LUA_TNUMBER:
			fprintf(stdout, "%g", lua_tonumber(pState, -1));
			break;
		case LUA_TBOOLEAN:
			fprintf(stdout, "%d", int(lua_toboolean(pState, -1)));
			break;
		case LUA_TSTRING:
			fprintf(stdout, "%s", lua_tostring(pState, -1));
			break;
		case  LUA_TFUNCTION:
			fprintf(stdout, "%s:%p", lua_typename(pState, type), lua_topointer(pState, -1));
			break;
		case LUA_TUSERDATA:
			fprintf(stdout, "%s:%s", lua_typename(pState, type), lua_tostring(pState, -1));
			break;
		default:
			fprintf(stdout, "%s:%p", lua_typename(pState, type), lua_topointer(pState, -1));
		}

		fprintf(stdout, "\n");

		lua_pop(pState, 1);
	}

	fprintf(stdout, "}\n");

	lua_settop(pState, top);
}


sol::table Script::ProtobufTransTable(const google::protobuf::Message&& msg)
{
	protobuf_new_table(this->lua_state(), std::forward<const google::protobuf::Message*>(&msg));
	sol::table table(this->lua_state());
	return std::move(table);
}


google::protobuf::Message* Script::TableTransProtobuf(const char* protoName, sol::table& table)
{
//	this->set(table);
	sol::object obj = sol::make_object(this->lua_state(), table);
	sol::stack::push(this->lua_state(),obj);
//	StackDump();
	google::protobuf::Message* msg = create_message(protoName);
	fill_lua_table(this->lua_state(), -1, msg);
	traverse_message(msg);
	sol::stack::pop_n(lua_state(), -1);
	return msg;

	
}

sol::table Script::ProtoDecode(const char* proto, std::string_view data)
{
	google::protobuf::Message* msg = create_message(proto);
	msg->ParseFromString(data.data());
	return ProtobufTransTable(std::move(*msg));
 }


sol::table Script::ProtoDecode(const char* proto, const gb::ReadBufferPtr& buffer)
 {
     google::protobuf::Message* msg = create_message(proto);
     msg->ParseFromZeroCopyStream(buffer.get());
	 return ProtobufTransTable(std::move(*msg));
 }

 std::string Script::ProtoEncodeToString(const char* proto, sol::table& tbl)
 {
	auto msg = TableTransProtobuf(proto, tbl);
	if (msg != nullptr)
	{
		return msg->SerializeAsString();
	}
	return "";
 }

 gb::ReadBufferPtr Script::ProtoEncode(const char* proto, sol::table& tbl)
 {
	auto msg = TableTransProtobuf(proto, tbl);
	if (msg != nullptr)
	{
        gb::WriteBuffer   write_buffer;
		gb::ReadBufferPtr read_buffer(new gb::ReadBuffer());
		if (msg->SerializeToZeroCopyStream(&write_buffer))
		{
            write_buffer.SwapOut(read_buffer.get());
            return read_buffer;
		}
	}
    return nullptr;
 }

void Script::Load(std::string filename)
{
	this->safe_script_file(filename,[](lua_State*, sol::protected_function_result pfr) {
		sol::error err = pfr;
		LOG_ERROR("[LUA]: {}", err.what());
		return pfr;
	});
}

void OnScriptError(sol::error&& err)
{
	LOG_ERROR("[script error]: {}", err.what());
}
