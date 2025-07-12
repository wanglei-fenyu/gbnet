cmake_minimum_required(VERSION 3.16)
project(gb_net CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(def)
include(base)


add_definitions(-DPROTOBUF_USE_DLLS)  # protobuf使用动态库
add_compile_definitions(BOOST_ASIO_HAS_STD_THREAD) # asio使用std::thread

if(NOT COMMON_CPP_DIR)
    set(COMMON_CPP_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()
set(ALL_FILE ${COMMON_CPP_DIR})

collectHeadFiles(ALL_INC ALL_FILE)
collectSrcFiles(ALL_SRC ALL_FILE)
collectAllSrcFiles(ALL_INC_SRC ALL_FILE)
message("SOURCE FILES")
message(${ALL_INC_SRC})
if(LINUX)
else()
    # Windows 下分组
    source_group(${PROJECT_NAME} TREE ${COMMON_CPP_DIR} FILES ${ALL_INC_SRC})
endif()

add_library(${PROJECT_NAME} SHARED ${ALL_INC_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 确保 MSVC 生成 .lib
)

# 设置 Boost 路径
set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/../boost")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")

# 查找 Boost
find_package(Boost 1.87 REQUIRED)
if(Boost_FOUND)
    message("Boost find success")
    message(${Boost_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
endif()

# 强制指定 OpenSSL 路径，禁止系统路径搜索
set(OpenSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/../openssl")
#find_package(OpenSSL 1.1 REQUIRED NO_DEFAULT_PATH NO_CMAKE_SYSTEM_PATH)
#if(NOT OpenSSL_FOUND)
#    message(FATAL_ERROR "OpenSSL 1.1 not found in ${OpenSSL_ROOT_DIR}")
#endif()
if(LINUX)
    set(OPENSSL_LIBS
        "${CMAKE_SOURCE_DIR}/../openssl/lib/libssl.so"
        "${CMAKE_SOURCE_DIR}/../openssl/lib/libcrypto.so"
    )
else()
    set(OPENSSL_LIBS
        "${CMAKE_SOURCE_DIR}/../openssl/lib/ssl-1_1-x64.lib"
        "${CMAKE_SOURCE_DIR}/../openssl/lib/crypto-1_1-x64.lib"
    )
endif()

# 设置 Protobuf 路径并查找
set(Protobuf_ROOT "${CMAKE_SOURCE_DIR}/../google-protobuf")
find_package(Protobuf REQUIRED)

# 头文件包含
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd
    ${OpenSSL_ROOT_DIR}/include
    ${Protobuf_ROOT}/include
    ${CMAKE_SOURCE_DIR}/3rd/zlib
    ${CMAKE_SOURCE_DIR}/3rd/asio
    ${MY_INC}
)

target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

if(LINUX)
    set(LIB_NAMES "zlib")
    appendSuffixInList(LIB_NAME_USE "${LIB_NAMES}" ".so")
    message("LIB_NAME_USE: ${LIB_NAME_USE}")
else()
    set(LIB_NAMES "zlib")
    appendSuffixInList(LIB_NAME_USE "${LIB_NAMES}" ".lib")
endif()

message("LIB_NAME_USE: ${LIB_NAME_USE}")

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIB_NAME_USE}
    ${OPENSSL_LIBS}
    protobuf::libprotobuf
)

