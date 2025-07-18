// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: meta.proto

#include "meta.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG
constexpr Meta::Meta(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : mode_(0)

  , id_(0)
  , method_(int64_t{0})
  , type_(0)
  , compress_type_(0)

  , sequence_(int64_t{0}){}
struct MetaDefaultTypeInternal {
  constexpr MetaDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~MetaDefaultTypeInternal() {}
  union {
    Meta _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT MetaDefaultTypeInternal _Meta_default_instance_;
static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_meta_2eproto[1];
static const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* file_level_enum_descriptors_meta_2eproto[2];
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_meta_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_meta_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::Meta, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::Meta, mode_),
  PROTOBUF_FIELD_OFFSET(::Meta, id_),
  PROTOBUF_FIELD_OFFSET(::Meta, type_),
  PROTOBUF_FIELD_OFFSET(::Meta, method_),
  PROTOBUF_FIELD_OFFSET(::Meta, sequence_),
  PROTOBUF_FIELD_OFFSET(::Meta, compress_type_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::Meta)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::_Meta_default_instance_),
};

const char descriptor_table_protodef_meta_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\nmeta.proto\"\203\001\n\004Meta\022\026\n\004mode\030\001 \001(\0162\010.Ms"
  "gMode\022\n\n\002id\030\002 \001(\005\022\014\n\004type\030\003 \001(\005\022\016\n\006metho"
  "d\030\004 \001(\003\022\020\n\010sequence\030\005 \001(\003\022\'\n\rcompress_ty"
  "pe\030\006 \001(\0162\020.MsgCompressType*X\n\017MsgCompres"
  "sType\022\020\n\014CompressNone\020\000\022\020\n\014CompressGzip\020"
  "\001\022\020\n\014CompressZlib\020\002\022\017\n\013CompressLZ4\020\003*-\n\007"
  "MsgMode\022\007\n\003Msg\020\000\022\013\n\007Request\020\001\022\014\n\010Respons"
  "e\020\002b\006proto3"
  ;
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_meta_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_meta_2eproto = {
  false, false, 291, descriptor_table_protodef_meta_2eproto, "meta.proto", 
  &descriptor_table_meta_2eproto_once, nullptr, 0, 1,
  schemas, file_default_instances, TableStruct_meta_2eproto::offsets,
  file_level_metadata_meta_2eproto, file_level_enum_descriptors_meta_2eproto, file_level_service_descriptors_meta_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* descriptor_table_meta_2eproto_getter() {
  return &descriptor_table_meta_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY static ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptorsRunner dynamic_init_dummy_meta_2eproto(&descriptor_table_meta_2eproto);
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* MsgCompressType_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_meta_2eproto);
  return file_level_enum_descriptors_meta_2eproto[0];
}
bool MsgCompressType_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
      return true;
    default:
      return false;
  }
}

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* MsgMode_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_meta_2eproto);
  return file_level_enum_descriptors_meta_2eproto[1];
}
bool MsgMode_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
      return true;
    default:
      return false;
  }
}


// ===================================================================

class Meta::_Internal {
 public:
};

Meta::Meta(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:Meta)
}
Meta::Meta(const Meta& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::memcpy(&mode_, &from.mode_,
    static_cast<size_t>(reinterpret_cast<char*>(&sequence_) -
    reinterpret_cast<char*>(&mode_)) + sizeof(sequence_));
  // @@protoc_insertion_point(copy_constructor:Meta)
}

void Meta::SharedCtor() {
::memset(reinterpret_cast<char*>(this) + static_cast<size_t>(
    reinterpret_cast<char*>(&mode_) - reinterpret_cast<char*>(this)),
    0, static_cast<size_t>(reinterpret_cast<char*>(&sequence_) -
    reinterpret_cast<char*>(&mode_)) + sizeof(sequence_));
}

Meta::~Meta() {
  // @@protoc_insertion_point(destructor:Meta)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void Meta::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
}

void Meta::ArenaDtor(void* object) {
  Meta* _this = reinterpret_cast< Meta* >(object);
  (void)_this;
}
void Meta::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void Meta::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void Meta::Clear() {
// @@protoc_insertion_point(message_clear_start:Meta)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ::memset(&mode_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&sequence_) -
      reinterpret_cast<char*>(&mode_)) + sizeof(sequence_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* Meta::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // .MsgMode mode = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          ::PROTOBUF_NAMESPACE_ID::uint64 val = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
          _internal_set_mode(static_cast<::MsgMode>(val));
        } else
          goto handle_unusual;
        continue;
      // int32 id = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 16)) {
          id_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // int32 type = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 24)) {
          type_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // int64 method = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 32)) {
          method_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // int64 sequence = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 40)) {
          sequence_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // .MsgCompressType compress_type = 6;
      case 6:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 48)) {
          ::PROTOBUF_NAMESPACE_ID::uint64 val = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
          _internal_set_compress_type(static_cast<::MsgCompressType>(val));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* Meta::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:Meta)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // .MsgMode mode = 1;
  if (this->_internal_mode() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnumToArray(
      1, this->_internal_mode(), target);
  }

  // int32 id = 2;
  if (this->_internal_id() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteInt32ToArray(2, this->_internal_id(), target);
  }

  // int32 type = 3;
  if (this->_internal_type() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteInt32ToArray(3, this->_internal_type(), target);
  }

  // int64 method = 4;
  if (this->_internal_method() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteInt64ToArray(4, this->_internal_method(), target);
  }

  // int64 sequence = 5;
  if (this->_internal_sequence() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteInt64ToArray(5, this->_internal_sequence(), target);
  }

  // .MsgCompressType compress_type = 6;
  if (this->_internal_compress_type() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnumToArray(
      6, this->_internal_compress_type(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:Meta)
  return target;
}

size_t Meta::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:Meta)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // .MsgMode mode = 1;
  if (this->_internal_mode() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::EnumSize(this->_internal_mode());
  }

  // int32 id = 2;
  if (this->_internal_id() != 0) {
    total_size += ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int32SizePlusOne(this->_internal_id());
  }

  // int64 method = 4;
  if (this->_internal_method() != 0) {
    total_size += ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int64SizePlusOne(this->_internal_method());
  }

  // int32 type = 3;
  if (this->_internal_type() != 0) {
    total_size += ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int32SizePlusOne(this->_internal_type());
  }

  // .MsgCompressType compress_type = 6;
  if (this->_internal_compress_type() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::EnumSize(this->_internal_compress_type());
  }

  // int64 sequence = 5;
  if (this->_internal_sequence() != 0) {
    total_size += ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int64SizePlusOne(this->_internal_sequence());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData Meta::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    Meta::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*Meta::GetClassData() const { return &_class_data_; }

void Meta::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to,
                      const ::PROTOBUF_NAMESPACE_ID::Message& from) {
  static_cast<Meta *>(to)->MergeFrom(
      static_cast<const Meta &>(from));
}


void Meta::MergeFrom(const Meta& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:Meta)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_mode() != 0) {
    _internal_set_mode(from._internal_mode());
  }
  if (from._internal_id() != 0) {
    _internal_set_id(from._internal_id());
  }
  if (from._internal_method() != 0) {
    _internal_set_method(from._internal_method());
  }
  if (from._internal_type() != 0) {
    _internal_set_type(from._internal_type());
  }
  if (from._internal_compress_type() != 0) {
    _internal_set_compress_type(from._internal_compress_type());
  }
  if (from._internal_sequence() != 0) {
    _internal_set_sequence(from._internal_sequence());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void Meta::CopyFrom(const Meta& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:Meta)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Meta::IsInitialized() const {
  return true;
}

void Meta::InternalSwap(Meta* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(Meta, sequence_)
      + sizeof(Meta::sequence_)
      - PROTOBUF_FIELD_OFFSET(Meta, mode_)>(
          reinterpret_cast<char*>(&mode_),
          reinterpret_cast<char*>(&other->mode_));
}

::PROTOBUF_NAMESPACE_ID::Metadata Meta::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_meta_2eproto_getter, &descriptor_table_meta_2eproto_once,
      file_level_metadata_meta_2eproto[0]);
}

// @@protoc_insertion_point(namespace_scope)
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::Meta* Arena::CreateMaybeMessage< ::Meta >(Arena* arena) {
  return Arena::CreateMessageInternal< ::Meta >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
