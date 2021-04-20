cmake_minimum_required(VERSION 3.10)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(hzUdpServer)

add_executable(hzUdpServer main.cpp)

target_link_libraries(hzUdpServer PUBLIC pthread boost_thread)
