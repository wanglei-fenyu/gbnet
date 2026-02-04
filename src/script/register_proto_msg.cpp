#include "script.h"
#include "../protobuf/msg.pb.h"

void register_proto_msg(std::shared_ptr<Script>& scriptPtr) {
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
