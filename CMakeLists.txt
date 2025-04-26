cmake_minimum_required(VERSION 3.10)

project(proxy-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(proxy-server
    src/main.cpp
    src/server.cpp
)
