cmake_minimum_required(VERSION 3.16.3)
project(iotop_cpp)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

aux_source_directory(src SRC)

add_executable(iotop_cpp ${SRC})
