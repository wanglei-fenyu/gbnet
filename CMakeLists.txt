cmake_minimum_required(VERSION 3.16)

# 添加宏定义
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
project(server_project)

#设置静态库和动态库的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/$<CONFIG>/lib>)  #动态库默认要在exe同路径 所以生成后是在bin下
# 设置静态库的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/$<CONFIG>/lib>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/$<CONFIG>/bin>)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(./3rd/spdlog)
#add_subdirectory(./3rd/openssl)
add_subdirectory(./3rd/zlib)
add_subdirectory(./3rd/lua)
add_subdirectory(./3rd/mimalloc)
add_subdirectory(./3rd/gbnet)
#add_subdirectory(./src)
add_subdirectory(./test/server_test)
add_subdirectory(./test/client_test)
#set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})