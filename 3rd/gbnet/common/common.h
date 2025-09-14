#pragma once

#if defined(_MSC_VER) && defined(_DEBUG)
    #define MY_DEBUG_MODE 1
#elif defined(__GNUC__) && !defined(__OPTIMIZE__)
    #define MY_DEBUG_MODE 1
#else
    #define MY_DEBUG_MODE 0
#endif

#define NON_COPYABLE(TypeName)    \
    TypeName(const TypeName&)                  = delete; \
    const TypeName& operator=(const TypeName&) = delete;
