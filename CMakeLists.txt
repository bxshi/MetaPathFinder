cmake_minimum_required(VERSION 2.8.4)
project(MetaPathFinder)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(MetaPathFinder ${SOURCE_FILES})