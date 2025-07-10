import sys
import glob
import os
from google.protobuf import descriptor_pb2
from google.protobuf.descriptor import FieldDescriptor


def generate_descriptor(proto_file):
    descriptor_file = proto_file.replace('.proto', '_descriptor.pb')
    protoc_command = [
        'protoc',
        '--descriptor_set_out={}'.format(descriptor_file),
        '--proto_path=.',
        proto_file
    ]
    result = subprocess.run(protoc_command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running protoc: {result.stderr}")
        sys.exit(1)
    return descriptor_file

def parse_proto_file(proto_file):
    with open(proto_file, 'rb') as file:
        file_content = file.read()
    
    file_desc_set = descriptor_pb2.FileDescriptorSet()
    file_desc_set.ParseFromString(file_content)  # 使用 ParseFromString 解析二进制内容
    
    if not file_desc_set.file:
        raise ValueError(f"No file descriptors found in the provided file: {proto_file}")
    
    return file_desc_set.file  # 返回所有文件描述

#提取头文件
def get_header_filenames(file_descs):
    header_files = []
    for file_desc in file_descs:
        proto_file_name = file_desc
        header_file_name = proto_file_name.replace('.proto', '.pb.h')
        header_files.append(header_file_name)
    return header_files

def generate_sol2_code(proto_files):
    all_enums = []
    all_messages = []
    
    for proto_file in proto_files:
        file_descs = parse_proto_file(proto_file)
        
        for file_desc in file_descs:
            for enum_type in file_desc.enum_type:
                all_enums.append(enum_type)
        
            for message_type in file_desc.message_type:
                all_messages.append(message_type)
    
    vContents = []
    
    def traverse_enum(enum_desc, package, vContents):
        content = f'scriptPtr->new_enum("{enum_desc.name}",\n'
        for value in enum_desc.value:
            content += f'    "{value.name}", {enum_desc.name}::{value.name},\n'
        content = content.rstrip(',\n') + '\n);\n\n'
        vContents.append(content)
    
    def traverse_message(message_desc, package, vContents):
        for enum_desc in message_desc.enum_type:
            traverse_enum(enum_desc, package, vContents)
        
        content = f'scriptPtr->new_usertype<{message_desc.name}>(\"{message_desc.name}\"\n\t,sol::base_classes\n\t,sol::bases<google::protobuf::Message>()\n\t,sol::call_constructor\n\t,[](google::protobuf::Message* o)->{message_desc.name}*{{ return static_cast<{message_desc.name}*>(o); }}\n\t'
        
        for field in message_desc.field:
            key = field.name.lower()
            if field.label == FieldDescriptor.LABEL_REPEATED:
                if field.type == FieldDescriptor.TYPE_MESSAGE:
                    type_name = field.type_name.split('.')[-1]
                    content += f',\"{key}_size\",&{message_desc.name}::{key}_size\n\t'
                    content += f',\"clear_{key}\",&{message_desc.name}::clear_{key}\n\t'
                    content += f',\"add_{key}\",&{message_desc.name}::add_{key}\n\t'
                    content += f',\"mutable_{key}\",[]({message_desc.name}& o, int index)->{type_name}* {{ return o.mutable_{key}(index); }}\n\t'
                    content += f',\"{key}\",[](const {message_desc.name}& o, int index)->const {type_name}& {{ return o.{key}(index); }}\n\t'
                elif field.type == FieldDescriptor.TYPE_STRING:
                    content += f',\"{key}_size\",&{message_desc.name}::{key}_size\n\t'
                    content += f',\"clear_{key}\",&{message_desc.name}::clear_{key}\n\t'
                    content += f',\"add_{key}\",[]({message_desc.name}& o, int index, const char* str)->void {{ o.add_{key}(str); }}\n\t'
                    content += f',\"{key}\",[](const {message_desc.name}& o, int index)->const std::string& {{ return o.{key}(index); }}\n\t'
                else:
                    type_mapping = {
                        FieldDescriptor.TYPE_DOUBLE: "double",
                        FieldDescriptor.TYPE_FLOAT: "float",
                        FieldDescriptor.TYPE_INT64: "int64_t",
                        FieldDescriptor.TYPE_UINT64: "uint64_t",
                        FieldDescriptor.TYPE_INT32: "int32_t",
                        FieldDescriptor.TYPE_FIXED64: "uint64_t",
                        FieldDescriptor.TYPE_FIXED32: "uint32_t",
                        FieldDescriptor.TYPE_BOOL: "bool",
                        FieldDescriptor.TYPE_UINT32: "uint32_t",
                        FieldDescriptor.TYPE_ENUM: "int",
                        FieldDescriptor.TYPE_SFIXED32: "int32_t",
                        FieldDescriptor.TYPE_SFIXED64: "int64_t",
                        FieldDescriptor.TYPE_SINT32: "int32_t",
                        FieldDescriptor.TYPE_SINT64: "int64_t",
                    }
                    type_name = type_mapping.get(field.type, "unknown")
                    content += f',\"{key}_size\",&{message_desc.name}::{key}_size\n\t'
                    content += f',\"clear_{key}\",&{message_desc.name}::clear_{key}\n\t'
                    content += f',\"{key}\",[](const {message_desc.name}& o, int index)->{type_name} {{ return o.{key}(index); }}\n\t'
                    content += f',\"set_{key}\",&{message_desc.name}::set_{key}\n\t'
                    content += f',\"add_{key}\",&{message_desc.name}::add_{key}\n\t'
            else:
                if field.type == FieldDescriptor.TYPE_MESSAGE:
                    type_name = field.type_name.split('.')[-1]
                    content += f',\"has_{key}\",&{message_desc.name}::has_{key}\n\t'
                    content += f',\"clear_{key}\",&{message_desc.name}::clear_{key}\n\t'
                    content += f',\"{key}\",&{message_desc.name}::{key}'
                    content += f',\"release_{key}\",&{message_desc.name}::release_{key}\n\t'
                    content += f',\"mutable_{key}\",&{message_desc.name}::mutable_{key}\n\t'
                    content += f',\"set_allocated_{key}\",&{message_desc.name}::set_allocated_{key}\n\t'
                elif field.type == FieldDescriptor.TYPE_STRING:
                    content += f',\"clear_{key}\",&{message_desc.name}::clear_{key}\n\t'
                    content += f',\"{key}\",&{message_desc.name}::{key}\n\t'
                    content += f',\"set_{key}\",[]({message_desc.name}& o, const char* str)->void {{ o.set_{key}(str); }}\n\t'
                else:
                    type_mapping = {
                        FieldDescriptor.TYPE_DOUBLE: "double",
                        FieldDescriptor.TYPE_FLOAT: "float",
                        FieldDescriptor.TYPE_INT64: "int64_t",
                        FieldDescriptor.TYPE_UINT64: "uint64_t",
                        FieldDescriptor.TYPE_INT32: "int32_t",
                        FieldDescriptor.TYPE_FIXED64: "uint64_t",
                        FieldDescriptor.TYPE_FIXED32: "uint32_t",
                        FieldDescriptor.TYPE_BOOL: "bool",
                        FieldDescriptor.TYPE_UINT32: "uint32_t",
                        FieldDescriptor.TYPE_ENUM: "int",
                        FieldDescriptor.TYPE_SFIXED32: "int32_t",
                        FieldDescriptor.TYPE_SFIXED64: "int64_t",
                        FieldDescriptor.TYPE_SINT32: "int32_t",
                        FieldDescriptor.TYPE_SINT64: "int64_t",
                    }
                    type_name = type_mapping.get(field.type, "unknown")
                    content += f',\"clear_{key}\",&{message_desc.name}::clear_{key}\n\t'
                    content += f',\"{key}\",&{message_desc.name}::{key}\n\t'
                    content += f',\"set_{key}\",&{message_desc.name}::set_{key}\n\t'
        
        content += ");\n\n"
        vContents.append(content)
        
        for nested_type in message_desc.nested_type:
            traverse_message(nested_type, package, vContents)
    
    for enum_desc in all_enums:
        traverse_enum(enum_desc, file_desc.package, vContents)
    
    for message_desc in all_messages:
        traverse_message(message_desc, file_desc.package, vContents)
    
    return vContents

def main():
    descriptor_file = "descriptors.pb"   
    descriptor_files = glob.glob(descriptor_file)
    if not descriptor_files:
        print(f'No files matched the pattern: {descriptor_file}')
        sys.exit(1)
    sol2_code = generate_sol2_code(descriptor_files)

    proto_file = "*.proto"
    proto_files = glob.glob(proto_file)
    header_files = get_header_filenames(proto_files)
    
    output_file = '../../src/script/register_proto_msg.cpp'
    with open(output_file, 'w') as file:
        file.write('#include "script.h"\n')
        for head_file in header_files:
            file.write(f'#include "../protobuf/{head_file}"\n')
        file.write('\nvoid register_proto_msg(std::shared_ptr<Script>& scriptPtr) {\n')
        for line in sol2_code:
            file.write(line)
        file.write('}\n')
    
    print(f'Sol2 registration code generated in {output_file}')

if __name__ == '__main__':
    main()
