set(SC_LIST callerserver.cc ../user.pb.cc)
add_executable(consumer ${SC_LIST})
target_link_libraries(consumer mprpc protobuf  muduo_net muduo_base pthread zookeeper_mt)