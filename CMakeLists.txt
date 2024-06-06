cmake_minimum_required(VERSION 3.0)
project(kvfs)

set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)

include_directories(
	${PROJECT_SOURCE_DIR}/include
)

add_compile_options(
	-std=c++17
	-O2
	-g
    -Wall
    # for rpclib.
    -Wno-class-memaccess
    -Wno-unused-but-set-variable
)

add_definitions(
    # -DNDEBUG # use this to disable all "assert"s and "DLOG"s.
    -DFUSE_USE_VERSION=31
)

link_libraries(
    fuse3
    rocksdb
)

file(GLOB FS_SOURCES src/fuse_ops.cpp src/main.cpp)

add_subdirectory(third_party/rpclib)

add_executable(main ${FS_SOURCES})
target_link_libraries(main rpc)
add_executable(data_server src/data_server.cpp)
target_link_libraries(data_server rpc)
