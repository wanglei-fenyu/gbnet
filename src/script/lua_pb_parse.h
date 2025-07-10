#pragma once
#include "lua.hpp"
#include "google/protobuf/message.h"

void protobuf_new_table(lua_State* L, const google::protobuf::Message* msg);
int lua_to_protobuf(lua_State* L);
void fill_lua_table(lua_State* L, int idx, google::protobuf::Message* msg);
void traverse_message(const google::protobuf::Message* msg);
google::protobuf::Message* create_message(const char* type_name);
