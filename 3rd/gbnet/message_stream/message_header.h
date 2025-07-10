#pragma once
#include <cstdint>

#define MESSAGE_HEAD_MAGIC 322122u

namespace gb
{


	struct MessageHeader {
    union {
        char    magic_str[4];
        uint32_t  magic_str_value;
    };                      // 4 bytes
    int32_t   meta_size;    // 4 bytes
    int64_t   data_size;    // 8 bytes
    int64_t   message_size; // 8 bytes: message_size = meta_size + data_size, for check

    MessageHeader() :
        magic_str_value(MESSAGE_HEAD_MAGIC)
        , meta_size(0), data_size(0), message_size(0) {}

    bool CheckMagicString() const
    {
        return magic_str_value == MESSAGE_HEAD_MAGIC;
    }
};

}