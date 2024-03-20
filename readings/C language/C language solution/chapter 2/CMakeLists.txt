cmake_minimum_required(VERSION 3.27)
project(chapter_2)

set(CMAKE_CXX_STANDARD 17)

add_library(chapter_2 STATIC library.cpp
        chapter2_9.cpp
        chapter2_9.h)
