#if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
#	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/linux/)
#else()
#	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
#	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>")
#	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/win/)
#endif()


# 设置 zlib 的源码文件
set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

# 设置头文件
set(ZLIB_HDRS
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    trees.h
    zutil.h
)

# 生成 zconf.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h)

# 构建 zlib 库
add_library(zlib SHARED ${ZLIB_SRCS} ${ZLIB_HDRS})
#set_target_properties(zlib PROPERTIES OUTPUT_NAME "zlib")

set_target_properties(zlib PROPERTIES
    OUTPUT_NAME "zlib"
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 确保 MSVC 生成 .lib
)
# 设置包含目录
target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )