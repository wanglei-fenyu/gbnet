#pragma once

#include "def.h"



#define NON_COPYABLE(TypeName)    \
    TypeName(const TypeName&)                  = delete; \
    const TypeName& operator=(const TypeName&) = delete;
