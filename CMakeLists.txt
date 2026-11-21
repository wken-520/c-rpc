cmake_minimum_required(VERSION 3.16)

project(mprpc VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MPRPC_BUILD_EXAMPLES "Build RPC provider and consumer examples" ON)
option(MPRPC_BUILD_TESTS "Build unit tests" ON)

set(MPRPC_MUDUO_ROOT "" CACHE PATH "Muduo installation prefix")
set(MPRPC_ZOOKEEPER_ROOT "" CACHE PATH "ZooKeeper C client installation prefix")

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

find_path(MPRPC_MUDUO_INCLUDE_DIR
    NAMES muduo/net/TcpServer.h
    HINTS "${MPRPC_MUDUO_ROOT}"
    PATH_SUFFIXES include)
find_library(MPRPC_MUDUO_NET_LIBRARY
    NAMES muduo_net
    HINTS "${MPRPC_MUDUO_ROOT}"
    PATH_SUFFIXES lib lib64)
find_library(MPRPC_MUDUO_BASE_LIBRARY
    NAMES muduo_base
    HINTS "${MPRPC_MUDUO_ROOT}"
    PATH_SUFFIXES lib lib64)

find_path(MPRPC_ZOOKEEPER_INCLUDE_DIR
    NAMES zookeeper/zookeeper.h
    HINTS "${MPRPC_ZOOKEEPER_ROOT}"
    PATH_SUFFIXES include)
find_library(MPRPC_ZOOKEEPER_LIBRARY
    NAMES zookeeper_mt
    HINTS "${MPRPC_ZOOKEEPER_ROOT}"
    PATH_SUFFIXES lib lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Muduo REQUIRED_VARS
    MPRPC_MUDUO_INCLUDE_DIR MPRPC_MUDUO_NET_LIBRARY MPRPC_MUDUO_BASE_LIBRARY)
find_package_handle_standard_args(ZooKeeper REQUIRED_VARS
    MPRPC_ZOOKEEPER_INCLUDE_DIR MPRPC_ZOOKEEPER_LIBRARY)

add_library(mprpc_muduo_net UNKNOWN IMPORTED GLOBAL)
set_target_properties(mprpc_muduo_net PROPERTIES
    IMPORTED_LOCATION "${MPRPC_MUDUO_NET_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MPRPC_MUDUO_INCLUDE_DIR}")

add_library(mprpc_muduo_base UNKNOWN IMPORTED GLOBAL)
set_target_properties(mprpc_muduo_base PROPERTIES
    IMPORTED_LOCATION "${MPRPC_MUDUO_BASE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MPRPC_MUDUO_INCLUDE_DIR}")

add_library(mprpc_zookeeper UNKNOWN IMPORTED GLOBAL)
set_target_properties(mprpc_zookeeper PROPERTIES
    IMPORTED_LOCATION "${MPRPC_ZOOKEEPER_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MPRPC_ZOOKEEPER_INCLUDE_DIR}")

add_subdirectory(src)

if(MPRPC_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(BUILD_TESTING AND MPRPC_BUILD_TESTS)
    add_subdirectory(tests)
endif()
