cmake_minimum_required(VERSION 3.6)
project(Json_Checker_ConstExpr)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wextra -pedantic -Wall")
set(SOURCE_FILES main.cpp)

add_executable(Json_Checker_ConstExpr ${SOURCE_FILES})