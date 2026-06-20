cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("chess")

add_executable(main main.cpp)
target_compile_options(main PRIVATE -Wall -Wextra)

add_executable(chess_test test.cpp)
target_compile_options(chess_test PRIVATE -Wall -Wextra -std=c++20)
