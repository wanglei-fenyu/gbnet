#pragma once
#include "openssl\e_os2.h"

enum CompressType : int8_t
{
    CompressTypeNone = 0,
    CompressTypeGzip = 1,
    CompressTypeZlib = 2,
    CompressTypeLZ4  = 3,
};