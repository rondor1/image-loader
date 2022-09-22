cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
project(image-loader VERSION 0.0.1
        LANGUAGES CXX)

include(cmake/setup.cmake)
include(cmake/conan.cmake)

add_subdirectory(utils)
add_subdirectory(imageloader)
add_subdirectory(example)