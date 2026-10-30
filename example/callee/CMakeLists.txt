set(SRC_LIST userserver.cc ../user.pb.cc)
add_executable(provider ${SRC_LIST})
target_link_libraries(provider mprpc protobuf  muduo_net muduo_base pthread zookeeper_mt)