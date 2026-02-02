include_guard()

# ------------------------------------------------------------
# 基础设置
# ------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_VERBOSE_MAKEFILE ON)

# ------------------------------------------------------------
# 构建配置（注意：标准大小写）
# ------------------------------------------------------------
# multi-config 生成器（VS / Xcode / Ninja Multi-Config）
set(CMAKE_CONFIGURATION_TYPES
    "Debug;Release;ASAN"
    CACHE STRING "Build configurations"
    FORCE
)

# ------------------------------------------------------------
# 平台判断
# ------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
elseif(WIN32)
    set(WINDOWS TRUE)
else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

# ------------------------------------------------------------
# 环境信息输出（调试 CMake 用）
# ------------------------------------------------------------
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")
message(STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CONFIGURATION_TYPES: ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")

# ------------------------------------------------------------
# 生成器判断
# ------------------------------------------------------------
if("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
    message(STATUS "Generator: Visual Studio")

    add_compile_options(
        "$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>"
        "$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
    )
else()
    # 单配置生成器（Makefile / Ninja）
    if(NOT CMAKE_BUILD_TYPE)
        message(FATAL_ERROR
            "Single-config generator detected. "
            "Please specify -DCMAKE_BUILD_TYPE=Debug|Release|ASAN"
        )
    endif()
endif()

# ------------------------------------------------------------
# Linux 编译参数
# ------------------------------------------------------------
if(LINUX)
    add_compile_definitions(LINUX)

    # ===== Debug：真正可调试（强烈推荐）=====
    set(CMAKE_CXX_FLAGS_DEBUG
        "-g -O0 -fno-inline -fno-omit-frame-pointer"
    )

    # ===== Release：性能优先，保留符号 =====
    set(CMAKE_CXX_FLAGS_RELEASE
        "-g -O2"
    )

    # ===== ASAN：调试 + 内存检测 =====
    set(CMAKE_CXX_FLAGS_ASAN
        "-g -O0 -fno-inline -fno-omit-frame-pointer
         -fsanitize=address
         -fsanitize=leak"
    )

    # 通用编译选项（所有配置）
    add_compile_options(
        -pthread
        -fcoroutines
        -Wall
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        -Wno-unused-function
        -Wunused-result
    )
endif()

#add_compile_definitions(-DASIO_STANDALONE)
#add_compile_definitions(-DMYSQL_SEPARATE_COMPILATION)
#add_compile_definitions(-DBOOST_MYSQL_SEPARATE_COMPILATION)