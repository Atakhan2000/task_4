cmake_minimum_required(VERSION 3.19)

add_library(hash hash.h mm.h test_hash.cpp)
set_property(TARGET hash PROPERTY CXX_STANDARD 17)

project(testHash)

add_executable(testHash
  test_hash.cpp
)