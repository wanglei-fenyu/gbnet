cmake_minimum_required(VERSION 3.10)
project(lua_project)

# 编译器选项（Windows 处理 UTF-8 编码）
if(WIN32)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>")
endif()

# 包含当前目录作为头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 获取所有 Lua 源码文件
file(GLOB SRC ./*.h ./*.c)

# 生成 Lua 共享库
add_library(lua SHARED ${SRC})

# 设置 Windows 生成 `lua.dll` 和 `lua.lib`
set_target_properties(lua PROPERTIES
    OUTPUT_NAME "lua"
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 确保 MSVC 生成 .lib
)

# Linux 需要 `-fPIC` 让 .so 共享库正确导出符号
if(UNIX AND NOT APPLE)
    set_target_properties(lua PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Linux 需要手动链接 `m`（数学库）
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(lua PRIVATE m)
endif()