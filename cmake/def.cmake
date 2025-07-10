include_guard()



set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  #防止使用c++20之下的编译器
set(CMAKE_CXX_EXTENSIONS OFF)        #禁止使用编译器的扩展

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CONFIGURATION_TYPES "debug;release") # 限定构建模式 统一跨平台大小写

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX true)
else()

endif()

# 环境信息
message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}")
message("CMAKE_VERSION: ${CMAKE_VERSION} CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE} CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
message("CMAKE_CONFIGURATION_TYPES: ${CMAKE_CONFIGURATION_TYPES}")


if("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
    message("OpenType: generate VS Sln")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
else()
    if(NOT CMAKE_BUILD_TYPE)
        message(FATAL_ERROR "If you are using single-config generator, you should set CMAKE_BUILD_TYPE=<debug|release>")
    endif()
endif()

if(LINUX)
    add_compile_definitions(LINUX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
    set(CMAKE_CXX_FLAGS_RELEASE "-g -O2")
    set(CMAKE_CXX_FLAGS_ASAN "-g -Og -fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer -fsanitize=leak")
    string(APPEND CMAKE_CXX_FLAGS "  -pthread -fcoroutines -Wall -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wunused-result ")
elseif(WIN32)

else()
    message(FATAL_ERROR "unsupportted OS")
endif()

SET(LIB_FIND_DIR "${CMAKE_CURRENT_LIST_DIR}/../$<CONFIG>/lib/")

if(NOT EXE_DIR)
    if(WIN32)
        set(EXE_DIR "${CMAKE_CURRENT_LIST_DIR}/../bin/win/$<CONFIG>")
    else()
        set(EXE_DIR "${CMAKE_CURRENT_LIST_DIR}/../bin/linux/$<CONFIG>")
    endif()
endif()

if(NOT LIB_DIR)
    if(WIN32)
        set(LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/win")
    else()
        set(LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/linux")
    endif()
endif()


#add_compile_definitions(-DASIO_STANDALONE)
#add_compile_definitions(-DMYSQL_SEPARATE_COMPILATION)
#add_compile_definitions(-DBOOST_MYSQL_SEPARATE_COMPILATION)