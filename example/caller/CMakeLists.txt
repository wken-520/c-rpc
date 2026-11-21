add_executable(consumer callerserver.cc ../user.pb.cc)
target_include_directories(consumer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(consumer PRIVATE mprpc::mprpc)
