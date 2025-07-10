set SRC_PROTO=*.proto
"../../tools/protoc.exe" --proto_path=. --descriptor_set_out=descriptors.pb %SRC_PROTO%
"../../tools/protoc.exe" --proto_path=. --cpp_out=../../src/protobuf/ %SRC_PROTO%