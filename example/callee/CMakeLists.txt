add_executable(provider userserver.cc ../user.pb.cc)
target_include_directories(provider PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(provider PRIVATE mprpc::mprpc)
