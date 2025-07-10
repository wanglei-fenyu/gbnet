#include "script.h"
#include "protobuf/meta.pb.h"
#include "protobuf/msg.pb.h"

void register_proto_msg(std::shared_ptr<Script>& scriptPtr) {
scriptPtr->new_enum("CompressType",
    "CompressTypeNone", Proto::CompressType::CompressTypeNone,
    "CompressTypeGzip", Proto::CompressType::CompressTypeGzip,
    "CompressTypeZlib", Proto::CompressType::CompressTypeZlib,
    "CompressTypeLZ4", Proto::CompressType::CompressTypeLZ4
);

scriptPtr->new_enum("MsgMode",
    "Msg", Proto::MsgMode::Msg,
    "Request", Proto::MsgMode::Request,
    "Response", Proto::MsgMode::Response
);

scriptPtr->new_usertype<Proto::Meta>("Proto.Meta"
	,sol::base_classes
	,sol::bases<google::protobuf::Message>()
	,sol::call_constructor
	,[](google::protobuf::Message* o)->Proto::Meta*{ return static_cast<Proto::Meta*>(o); }
	,"clear_mode",&Proto::Meta::clear_mode
	,"mode",&Proto::Meta::mode
	,"set_mode",&Proto::Meta::set_mode
	,"clear_id",&Proto::Meta::clear_id
	,"id",&Proto::Meta::id
	,"set_id",&Proto::Meta::set_id
	,"clear_type",&Proto::Meta::clear_type
	,"type",&Proto::Meta::type
	,"set_type",&Proto::Meta::set_type
	,"clear_method",&Proto::Meta::clear_method
	,"method",&Proto::Meta::method
	,"set_method",&Proto::Meta::set_method
	,"clear_sequence",&Proto::Meta::clear_sequence
	,"sequence",&Proto::Meta::sequence
	,"set_sequence",&Proto::Meta::set_sequence
	,"clear_compress_type",&Proto::Meta::clear_compress_type
	,"compress_type",&Proto::Meta::compress_type
	,"set_compress_type",&Proto::Meta::set_compress_type
	);

scriptPtr->new_usertype<Proto::TestMsg>("Proto.TestMsg"
	,sol::base_classes
	,sol::bases<google::protobuf::Message>()
	,sol::call_constructor
	,[](google::protobuf::Message* o)->Proto::TestMsg*{ return static_cast<Proto::TestMsg*>(o); }
	,"clear_msg",&Proto::TestMsg::clear_msg
	,"msg",&Proto::TestMsg::msg
	,"set_msg",[](Proto::TestMsg& o, const char* str)->void { o.set_msg(str); }
	,"clear_index",&Proto::TestMsg::clear_index
	,"index",&Proto::TestMsg::index
	,"set_index",&Proto::TestMsg::set_index
	,"person_size",&Proto::TestMsg::person_size
	,"clear_person",&Proto::TestMsg::clear_person
	,"add_person",[](Proto::TestMsg& o, int index, const char* str)->void { o.add_person(str); }
	,"person",[](const Proto::TestMsg& o, int index)->const std::string& { return o.person(index); }
	);

}
