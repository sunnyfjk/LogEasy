cmake_minimum_required(VERSION 3.20)
project(LogEasy C)

set(CMAKE_C_STANDARD 99)

execute_process(
        COMMAND date +"%Y-%m-%d %H:%M:%S"
        OUTPUT_VARIABLE BUILD_DATETIME
        OUTPUT_STRIP_TRAILING_WHITESPACE #去除最后的换行符号
        ERROR_QUIET
        WORKING_DIRECTORY ..
)

execute_process(
        COMMAND git log -1 --format=%H
        OUTPUT_VARIABLE BUILD_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE #去除最后的换行符号
        ERROR_QUIET
        WORKING_DIRECTORY ..
)

add_definitions(-DLOG_EASY_PATH="${CMAKE_INSTALL_PREFIX}/etc")

configure_file(
        "config/version.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}Version.h"
)

configure_file(
        "config/version.c.in"
        "${CMAKE_CURRENT_BINARY_DIR}/source/${PROJECT_NAME}Version.c"
)
#当前工程头文件路径
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/source/private_log_easy/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
)
#当前工程源码目录
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source/private_log_easy/source SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_BINARY_DIR}/source SRC_LIST)

add_library(LogEasy SHARED ${SRC_LIST})

