cmake_minimum_required(VERSION 3.27)
project(C_language_solution)

set(CMAKE_CXX_STANDARD 14)

include_directories("chapter 2")

add_executable(C_language_solution
        "chapter 2/chapter2_9.cpp"
        "chapter 2/chapter2_9.h"
        "chapter 2/library.cpp"
        "chapter 2/library.h"
        "chapter 5/chapter5_5.cpp"
        "chapter 5/Main.cpp")
