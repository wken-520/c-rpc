cmake_minimum_required(VERSION 3.5)
project(mprpc)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#设置项目可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置项目库文件输出的路径
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(MUDUO_ROOT "/home/wenl/code/build/release-install-cpp11" 
    CACHE PATH "muduo install prefix")
#设置项目头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
include_directories(${MUDUO_ROOT}/include)
link_directories(${MUDUO_ROOT}/lib)
#设置项目库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(src)
add_subdirectory(example)