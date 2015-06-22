project(chainsign)
cmake_minimum_required(VERSION 2.8)

add_executable(send_msg send_msg.cpp)
target_link_libraries(send_msg boost_system pthread rt)