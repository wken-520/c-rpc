set(MPRPC_SOURCES
    logger.cc
    mprpcapplication.cc
    mprpcchannel.cc
    mprpcconfig.cc
    mprpccontroller.cc
    mprpcprovider.cc
    rpcheader.pb.cc
    zookeeperuil.cc)

add_library(mprpc_protocol STATIC mprpcprotocol.cc)
set_target_properties(mprpc_protocol PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(mprpc_protocol
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
if(WIN32)
    target_link_libraries(mprpc_protocol PUBLIC ws2_32)
endif()

add_library(mprpc SHARED ${MPRPC_SOURCES})
add_library(mprpc::mprpc ALIAS mprpc)
target_compile_definitions(mprpc PRIVATE THREADED)
target_include_directories(mprpc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${MPRPC_MUDUO_INCLUDE_DIR}
        ${MPRPC_ZOOKEEPER_INCLUDE_DIR})
target_link_libraries(mprpc
    PUBLIC
        mprpc_protocol
        protobuf::libprotobuf
        Threads::Threads
    PRIVATE
        mprpc_muduo_net
        mprpc_muduo_base
        mprpc_zookeeper)

install(TARGETS mprpc mprpc_protocol
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
