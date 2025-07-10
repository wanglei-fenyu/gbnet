#pragma once

enum CompressType : int
{
    CompressTypeNone = 0,
    CompressTypeGzip = 1,
    CompressTypeZlib = 2,
    CompressTypeLZ4  = 3,
};