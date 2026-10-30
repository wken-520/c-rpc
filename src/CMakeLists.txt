set(SRC_LIST mprpcapplication.cc mprpcconfig.cc mprpcprovider.cc 
                rpcheader.pb.cc mprpcchannel.cc mprpccontroller.cc
                logger.cc zookeeperuil.cc)
#include_directories(${CMAKE_CURRENT_BINARY_DIR}/rpcheader.pb.h)
add_library(mprpc SHARED ${SRC_LIST})
target_compile_definitions(mprpc PRIVATE THREADED)