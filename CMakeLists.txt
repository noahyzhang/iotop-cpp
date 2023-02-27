cmake_minimum_required(VERSION 3.16.3)
project(iotop_cpp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMKAE_CXX_FLAGS} -g -Wall -Werror -Wextra")

include_directories(
    src
)

file(GLOB_RECURSE SRCS src/*.cpp)

add_executable(iotop_cpp ${SRCS})

target_link_libraries(iotop_cpp
    ncurses
)
