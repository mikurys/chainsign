project(chainsign)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
set(CMAKE_CXX_FLAGS "-std=c++0x -g3 -O0")
find_package(OpenSSL REQUIRED)
#INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
#LIST(APPEND ADDITIONAL_LIBS ${OPENSSL_LIBRARIES})
add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} crypto cryptopp boost_system boost_filesystem boost_program_options pthread)

add_subdirectory(send_msg)