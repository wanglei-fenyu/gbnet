#include "script.h"
#include "../protobuf/meta.pb.h"
#include "../protobuf/msg.pb.h"

void register_proto_msg(std::shared_ptr<Script>& scriptPtr) {
scriptPtr->new_enum("MsgCompressType",
    "CompressNone", MsgCompressType::CompressNone,
    "CompressGzip", MsgCompressType::CompressGzip,
    "CompressZlib", MsgCompressType::CompressZlib,
    "CompressLZ4", MsgCompressType::CompressLZ4
);

scriptPtr->new_enum("MsgMode",
    "Msg", MsgMode::Msg,
    "Request", MsgMode::Request,
    "Response", MsgMode::Response
);

scriptPtr->new_usertype<Meta>("Meta"
	,sol::base_classes
	,sol::bases<google::protobuf::Message>()
	,sol::call_constructor
	,[](google::protobuf::Message* o)->Meta*{ return static_cast<Meta*>(o); }
	,"clear_mode",&Meta::clear_mode
	,"mode",&Meta::mode
	,"set_mode",&Meta::set_mode
	,"clear_id",&Meta::clear_id
	,"id",&Meta::id
	,"set_id",&Meta::set_id
	,"clear_type",&Meta::clear_type
	,"type",&Meta::type
	,"set_type",&Meta::set_type
	,"clear_method",&Meta::clear_method
	,"method",&Meta::method
	,"set_method",&Meta::set_method
	,"clear_sequence",&Meta::clear_sequence
	,"sequence",&Meta::sequence
	,"set_sequence",&Meta::set_sequence
	,"clear_compress_type",&Meta::clear_compress_type
	,"compress_type",&Meta::compress_type
	,"set_compress_type",&Meta::set_compress_type
	);

scriptPtr->new_usertype<TestMsg>("TestMsg"
	,sol::base_classes
	,sol::bases<google::protobuf::Message>()
	,sol::call_constructor
	,[](google::protobuf::Message* o)->TestMsg*{ return static_cast<TestMsg*>(o); }
	,"clear_msg",&TestMsg::clear_msg
	,"msg",&TestMsg::msg
	,"set_msg",[](TestMsg& o, const char* str)->void { o.set_msg(str); }
	,"clear_index",&TestMsg::clear_index
	,"index",&TestMsg::index
	,"set_index",&TestMsg::set_index
	,"person_size",&TestMsg::person_size
	,"clear_person",&TestMsg::clear_person
	,"add_person",[](TestMsg& o, int index, const char* str)->void { o.add_person(str); }
	,"person",[](const TestMsg& o, int index)->const std::string& { return o.person(index); }
	);

}
