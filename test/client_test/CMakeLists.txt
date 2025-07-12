include_guard()
project(client_test)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(def)
include(base)
cmake_policy(SET CMP0144 NEW)
add_executable(${PROJECT_NAME})
add_definitions(-DPROTOBUF_USE_DLLS)  # protobuf使用动态库
add_compile_definitions(BOOST_ASIO_HAS_STD_THREAD) # asio使用std::thread


if(NOT COMMON_CPP_DIR)
    set(COMMON_CPP_DIR ${CMAKE_SOURCE_DIR}/src)
endif()
set(ALL_FILE ${COMMON_CPP_DIR})
collectHeadFiles(ALL_INC ALL_FILE)
collectSrcFiles(ALL_SRC ALL_FILE)
collectAllSrcFiles(ALL_INC_SRC ALL_FILE)

if(NOT MAIN_CPP_DIR)
    set(MAIN_CPP_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()
set(ALL_MY_FILE_MAIN ${MAIN_CPP_DIR})

collectHeadFiles(MY_INC_MAIN ALL_MY_FILE_MAIN)
collectSrcFiles(MY_INC_SRC_MAIN ALL_MY_FILE_MAIN)
collectAllSrcFiles(MY_INC_SRC_MAIN ALL_MY_FILE_MAIN)

if(LINUX)
else()
    #windown下分组
    source_group(${PROJECT_NAME} TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${MY_INC_SRC_MAIN})
    source_group(${PROJECT_NAME} TREE ${COMMON_CPP_DIR} FILES ${ALL_INC_SRC})
endif()

target_sources(${PROJECT_NAME} PRIVATE
    ${ALL_INC_SRC}
    ${MY_INC_SRC_MAIN}
)

set(OPENSSL_INC_DIR ${CMAKE_SOURCE_DIR}/../openssl)
set(PROTOBUF_INC_DIR ${CMAKE_SOURCE_DIR}/../google-protobuf)
target_include_directories(${PROJECT_NAME} PRIVATE
${CMAKE_SOURCE_DIR}/3rd
${CMAKE_SOURCE_DIR}/3rd/lua
${CMAKE_SOURCE_DIR}/3rd/zlib
${OPENSSL_INC_DIR}/include
${PROTOBUF_INC_DIR}/include
${CMAKE_SOURCE_DIR}/3rd/asio
${CMAKE_SOURCE_DIR}/3rd/spdlog/include
${CMAKE_SOURCE_DIR}/3rd/mimalloc/include
${CMAKE_SOURCE_DIR}/3rd/gbnet
${CMAKE_SOURCE_DIR}/src
)

# 设置 Boost 安装目录
set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/../boost)  
# 设置 Boost 头文件目录
set(BOOST_INCLUDEDIR ${CMAKE_SOURCE_DIR}/../boost/include)  
# 查找 Boost
find_package(Boost 1.87 REQUIRED COMPONENTS charconv)

# 如果 Boost 被找到，配置项目
if(Boost_FOUND)
	message("Boost find success")
	message(${Boost_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS})  # 添加 Boost 头文件目录
endif()


set(OpenSSL_ROOT_DIR ${CMAKE_SOURCE_DIR}/../openssl)
#find_package(OpenSSL REQUIRED)
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
set(Protobuf_ROOT ${CMAKE_SOURCE_DIR}/../google-protobuf)
find_package(Protobuf REQUIRED)

if(LINUX)
	set(LIB_NAMES "zlib" "spdlog" "lua" "mimalloc" "gb_net")
    appendSuffixInList(LIB_NAME_USE "${LIB_NAMES}" ".so")
else()
	set(LIB_NAMES "zlib" "spdlog" "lua" "mimalloc" "gb_net")
    appendSuffixInList(LIB_NAME_USE "${LIB_NAMES}" ".lib")
endif()

#设置 库查询目录
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
target_link_libraries(${PROJECT_NAME} PRIVATE 
${LIB_NAME_USE}
${Boost_LIBRARIES}
${OPENSSL_LIBS}
protobuf::libprotobuf
)