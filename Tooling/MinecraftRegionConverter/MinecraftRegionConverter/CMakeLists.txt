cmake_minimum_required(VERSION 3.31)
project(mre)

set(CMAKE_CXX_STANDARD 20)

add_executable(mre main.cpp
        miniz.cpp
        miniz.h
        enkimi.cpp
        enkimi.h)
