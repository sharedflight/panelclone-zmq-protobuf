// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: xpilot.proto
// Protobuf C++ Version: 5.28.2

#include "xpilot.pb.h"

#include <algorithm>
#include <type_traits>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/generated_message_tctable_impl.h"
#include "google/protobuf/extension_set.h"
#include "google/protobuf/wire_format_lite.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/reflection_ops.h"
#include "google/protobuf/wire_format.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"
PROTOBUF_PRAGMA_INIT_SEG
namespace _pb = ::google::protobuf;
namespace _pbi = ::google::protobuf::internal;
namespace _fl = ::google::protobuf::internal::field_layout;
namespace xpilot {

inline constexpr RcvdMessage::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : message_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR RcvdMessage::RcvdMessage(::_pbi::ConstantInitialized)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(_class_data_.base()),
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(),
#endif  // PROTOBUF_CUSTOM_VTABLE
      _impl_(::_pbi::ConstantInitialized()) {
}
struct RcvdMessageDefaultTypeInternal {
  PROTOBUF_CONSTEXPR RcvdMessageDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~RcvdMessageDefaultTypeInternal() {}
  union {
    RcvdMessage _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 RcvdMessageDefaultTypeInternal _RcvdMessage_default_instance_;
}  // namespace xpilot
static constexpr const ::_pb::EnumDescriptor**
    file_level_enum_descriptors_xpilot_2eproto = nullptr;
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_xpilot_2eproto = nullptr;
const ::uint32_t
    TableStruct_xpilot_2eproto::offsets[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
        protodesc_cold) = {
        ~0u,  // no _has_bits_
        PROTOBUF_FIELD_OFFSET(::xpilot::RcvdMessage, _internal_metadata_),
        ~0u,  // no _extensions_
        ~0u,  // no _oneof_case_
        ~0u,  // no _weak_field_map_
        ~0u,  // no _inlined_string_donated_
        ~0u,  // no _split_
        ~0u,  // no sizeof(Split)
        PROTOBUF_FIELD_OFFSET(::xpilot::RcvdMessage, _impl_.message_),
};

static const ::_pbi::MigrationSchema
    schemas[] ABSL_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
        {0, -1, -1, sizeof(::xpilot::RcvdMessage)},
};
static const ::_pb::Message* const file_default_instances[] = {
    &::xpilot::_RcvdMessage_default_instance_._instance,
};
const char descriptor_table_protodef_xpilot_2eproto[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
    protodesc_cold) = {
    "\n\014xpilot.proto\022\006xpilot\"\036\n\013RcvdMessage\022\017\n"
    "\007message\030\001 \001(\tb\006proto3"
};
static ::absl::once_flag descriptor_table_xpilot_2eproto_once;
PROTOBUF_CONSTINIT const ::_pbi::DescriptorTable descriptor_table_xpilot_2eproto = {
    false,
    false,
    62,
    descriptor_table_protodef_xpilot_2eproto,
    "xpilot.proto",
    &descriptor_table_xpilot_2eproto_once,
    nullptr,
    0,
    1,
    schemas,
    file_default_instances,
    TableStruct_xpilot_2eproto::offsets,
    file_level_enum_descriptors_xpilot_2eproto,
    file_level_service_descriptors_xpilot_2eproto,
};
namespace xpilot {
// ===================================================================

class RcvdMessage::_Internal {
 public:
};

RcvdMessage::RcvdMessage(::google::protobuf::Arena* arena)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(arena, _class_data_.base()) {
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(arena) {
#endif  // PROTOBUF_CUSTOM_VTABLE
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:xpilot.RcvdMessage)
}
inline PROTOBUF_NDEBUG_INLINE RcvdMessage::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from, const ::xpilot::RcvdMessage& from_msg)
      : message_(arena, from.message_),
        _cached_size_{0} {}

RcvdMessage::RcvdMessage(
    ::google::protobuf::Arena* arena,
    const RcvdMessage& from)
#if defined(PROTOBUF_CUSTOM_VTABLE)
    : ::google::protobuf::Message(arena, _class_data_.base()) {
#else   // PROTOBUF_CUSTOM_VTABLE
    : ::google::protobuf::Message(arena) {
#endif  // PROTOBUF_CUSTOM_VTABLE
  RcvdMessage* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_, from);

  // @@protoc_insertion_point(copy_constructor:xpilot.RcvdMessage)
}
inline PROTOBUF_NDEBUG_INLINE RcvdMessage::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : message_(arena),
        _cached_size_{0} {}

inline void RcvdMessage::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
}
RcvdMessage::~RcvdMessage() {
  // @@protoc_insertion_point(destructor:xpilot.RcvdMessage)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void RcvdMessage::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.message_.Destroy();
  _impl_.~Impl_();
}

PROTOBUF_CONSTINIT
PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::google::protobuf::MessageLite::ClassDataFull
    RcvdMessage::_class_data_ = {
        ::google::protobuf::Message::ClassData{
            &_RcvdMessage_default_instance_._instance,
            &_table_.header,
            nullptr,  // OnDemandRegisterArenaDtor
            nullptr,  // IsInitialized
            &RcvdMessage::MergeImpl,
#if defined(PROTOBUF_CUSTOM_VTABLE)
            ::google::protobuf::Message::GetDeleteImpl<RcvdMessage>(),
            ::google::protobuf::Message::GetNewImpl<RcvdMessage>(),
            ::google::protobuf::Message::GetClearImpl<RcvdMessage>(), &RcvdMessage::ByteSizeLong,
                &RcvdMessage::_InternalSerialize,
#endif  // PROTOBUF_CUSTOM_VTABLE
            PROTOBUF_FIELD_OFFSET(RcvdMessage, _impl_._cached_size_),
            false,
        },
        &RcvdMessage::kDescriptorMethods,
        &descriptor_table_xpilot_2eproto,
        nullptr,  // tracker
};
const ::google::protobuf::MessageLite::ClassData* RcvdMessage::GetClassData() const {
  ::google::protobuf::internal::PrefetchToLocalCache(&_class_data_);
  ::google::protobuf::internal::PrefetchToLocalCache(_class_data_.tc_table);
  return _class_data_.base();
}
PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<0, 1, 0, 34, 2> RcvdMessage::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    1, 0,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967294,  // skipmap
    offsetof(decltype(_table_), field_entries),
    1,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    _class_data_.base(),
    nullptr,  // post_loop_handler
    ::_pbi::TcParser::GenericFallback,  // fallback
    #ifdef PROTOBUF_PREFETCH_PARSE_TABLE
    ::_pbi::TcParser::GetTable<::xpilot::RcvdMessage>(),  // to_prefetch
    #endif  // PROTOBUF_PREFETCH_PARSE_TABLE
  }, {{
    // string message = 1;
    {::_pbi::TcParser::FastUS1,
     {10, 63, 0, PROTOBUF_FIELD_OFFSET(RcvdMessage, _impl_.message_)}},
  }}, {{
    65535, 65535
  }}, {{
    // string message = 1;
    {PROTOBUF_FIELD_OFFSET(RcvdMessage, _impl_.message_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kUtf8String | ::_fl::kRepAString)},
  }},
  // no aux_entries
  {{
    "\22\7\0\0\0\0\0\0"
    "xpilot.RcvdMessage"
    "message"
  }},
};

PROTOBUF_NOINLINE void RcvdMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:xpilot.RcvdMessage)
  ::google::protobuf::internal::TSanWrite(&_impl_);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.message_.ClearToEmpty();
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

#if defined(PROTOBUF_CUSTOM_VTABLE)
        ::uint8_t* RcvdMessage::_InternalSerialize(
            const MessageLite& base, ::uint8_t* target,
            ::google::protobuf::io::EpsCopyOutputStream* stream) {
          const RcvdMessage& this_ = static_cast<const RcvdMessage&>(base);
#else   // PROTOBUF_CUSTOM_VTABLE
        ::uint8_t* RcvdMessage::_InternalSerialize(
            ::uint8_t* target,
            ::google::protobuf::io::EpsCopyOutputStream* stream) const {
          const RcvdMessage& this_ = *this;
#endif  // PROTOBUF_CUSTOM_VTABLE
          // @@protoc_insertion_point(serialize_to_array_start:xpilot.RcvdMessage)
          ::uint32_t cached_has_bits = 0;
          (void)cached_has_bits;

          // string message = 1;
          if (!this_._internal_message().empty()) {
            const std::string& _s = this_._internal_message();
            ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
                _s.data(), static_cast<int>(_s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "xpilot.RcvdMessage.message");
            target = stream->WriteStringMaybeAliased(1, _s, target);
          }

          if (PROTOBUF_PREDICT_FALSE(this_._internal_metadata_.have_unknown_fields())) {
            target =
                ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
                    this_._internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
          }
          // @@protoc_insertion_point(serialize_to_array_end:xpilot.RcvdMessage)
          return target;
        }

#if defined(PROTOBUF_CUSTOM_VTABLE)
        ::size_t RcvdMessage::ByteSizeLong(const MessageLite& base) {
          const RcvdMessage& this_ = static_cast<const RcvdMessage&>(base);
#else   // PROTOBUF_CUSTOM_VTABLE
        ::size_t RcvdMessage::ByteSizeLong() const {
          const RcvdMessage& this_ = *this;
#endif  // PROTOBUF_CUSTOM_VTABLE
          // @@protoc_insertion_point(message_byte_size_start:xpilot.RcvdMessage)
          ::size_t total_size = 0;

          ::uint32_t cached_has_bits = 0;
          // Prevent compiler warnings about cached_has_bits being unused
          (void)cached_has_bits;

           {
            // string message = 1;
            if (!this_._internal_message().empty()) {
              total_size += 1 + ::google::protobuf::internal::WireFormatLite::StringSize(
                                              this_._internal_message());
            }
          }
          return this_.MaybeComputeUnknownFieldsSize(total_size,
                                                     &this_._impl_._cached_size_);
        }

void RcvdMessage::MergeImpl(::google::protobuf::MessageLite& to_msg, const ::google::protobuf::MessageLite& from_msg) {
  auto* const _this = static_cast<RcvdMessage*>(&to_msg);
  auto& from = static_cast<const RcvdMessage&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:xpilot.RcvdMessage)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_message().empty()) {
    _this->_internal_set_message(from._internal_message());
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void RcvdMessage::CopyFrom(const RcvdMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:xpilot.RcvdMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}


void RcvdMessage::InternalSwap(RcvdMessage* PROTOBUF_RESTRICT other) {
  using std::swap;
  auto* arena = GetArena();
  ABSL_DCHECK_EQ(arena, other->GetArena());
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.message_, &other->_impl_.message_, arena);
}

::google::protobuf::Metadata RcvdMessage::GetMetadata() const {
  return ::google::protobuf::Message::GetMetadataImpl(GetClassData()->full());
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace xpilot
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::std::false_type
    _static_init2_ PROTOBUF_UNUSED =
        (::_pbi::AddDescriptors(&descriptor_table_xpilot_2eproto),
         ::std::false_type{});
#include "google/protobuf/port_undef.inc"
