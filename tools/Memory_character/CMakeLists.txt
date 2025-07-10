add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>")
# 设置vs默认启动项
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT cmake_exec)
