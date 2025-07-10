cmake_minimum_required(VERSION 3.10)
project(LuaSocket)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin/socket)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)

include_directories("${CMAKE_CURRENT_LIST_DIR}/../../3rd/lua/")
add_compile_options(-fPIC)

IF(WIN32)
    SET(luasocket_PLATFORM_SPECIFIC
            ${LuaSocket_SOURCE_DIR}/src/wsocket.h
            ${LuaSocket_SOURCE_DIR}/src/wsocket.c
            )
ELSEIF(UNIX)
    SET(luasocket_PLATFORM_SPECIFIC
            ${LuaSocket_SOURCE_DIR}/src/usocket.h
            ${LuaSocket_SOURCE_DIR}/src/usocket.c
            )
ENDIF(WIN32)

SET(luasocket_SOCKET
        ${LuaSocket_SOURCE_DIR}/src/luasocket.c
        ${LuaSocket_SOURCE_DIR}/src/inet.h
        ${LuaSocket_SOURCE_DIR}/src/inet.c
        ${LuaSocket_SOURCE_DIR}/src/tcp.h
        ${LuaSocket_SOURCE_DIR}/src/tcp.c
        ${LuaSocket_SOURCE_DIR}/src/udp.h
        ${LuaSocket_SOURCE_DIR}/src/udp.c
        ${LuaSocket_SOURCE_DIR}/src/except.h
        ${LuaSocket_SOURCE_DIR}/src/except.c
        ${LuaSocket_SOURCE_DIR}/src/select.h
        ${LuaSocket_SOURCE_DIR}/src/select.c
        ${LuaSocket_SOURCE_DIR}/src/timeout.h
        ${LuaSocket_SOURCE_DIR}/src/timeout.c
        ${LuaSocket_SOURCE_DIR}/src/buffer.h
        ${LuaSocket_SOURCE_DIR}/src/buffer.c
        ${LuaSocket_SOURCE_DIR}/src/io.h
        ${LuaSocket_SOURCE_DIR}/src/io.c
        ${LuaSocket_SOURCE_DIR}/src/auxiliar.h
        ${LuaSocket_SOURCE_DIR}/src/auxiliar.c
        ${LuaSocket_SOURCE_DIR}/src/options.h
        ${LuaSocket_SOURCE_DIR}/src/options.c
        ${luasocket_PLATFORM_SPECIFIC}
        )

SET(luasocket_PUBLIC_HEADERS
        ${LuaSocket_SOURCE_DIR}/src/luasocket.h
        )

SET(luasocket_RESOURCE_FILES
        ${LuaSocket_SOURCE_DIR}/src/http.lua
        ${LuaSocket_SOURCE_DIR}/src/tp.lua
        ${LuaSocket_SOURCE_DIR}/src/ftp.lua
        ${LuaSocket_SOURCE_DIR}/src/smtp.lua
        ${LuaSocket_SOURCE_DIR}/src/url.lua
        ${LuaSocket_SOURCE_DIR}/src/mbox.lua
        ${LuaSocket_SOURCE_DIR}/src/headers.lua

        )

SET(luasocketcommon_RESOURCE_FILES
        ${LuaSocket_SOURCE_DIR}/src/ltn12.lua
        ${LuaSocket_SOURCE_DIR}/src/socket.lua
        ${LuaSocket_SOURCE_DIR}/src/mime.lua
        )

ADD_LIBRARY(luasocket SHARED ${luasocket_PUBLIC_HEADERS} ${luasocket_SOCKET})

SET_TARGET_PROPERTIES(luasocket PROPERTIES
        PREFIX ""
        OUTPUT_NAME "core"
        COMPILE_FLAGS "${LUASOCKET_C_FLAGS}"
        )



IF(WIN32)
find_library (LUA_LIB lua "${CMAKE_CURRENT_LIST_DIR}/../../lib/win/")
TARGET_LINK_LIBRARIES(luasocket ${LUASOCKET_LINK_FLAGS} lua ws2_32.lib)
ELSSEIF
ENDIF
