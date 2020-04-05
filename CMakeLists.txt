cmake_minimum_required(VERSION 3.3)
project(syncvlc)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-g)
add_compile_options(-Werror -Wall -Wextra)

add_executable(syncvlc ./src/syncvlc.c ./src/network.c ./src/vlc.c) 

add_executable(syncvlc_relay ./src/syncvlc_relay.c ./src/network.c)
target_link_libraries(syncvlc_relay pthread)