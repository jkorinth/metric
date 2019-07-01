cmake_minimum_required(VERSION 3.13)
project(metric)
enable_testing()

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_CLANG)
  add_compile_options(-Wall -Werror -g -pedantic)
endif()

add_library(metric INTERFACE)
target_include_directories(metric INTERFACE include/)

add_subdirectory(test)
add_subdirectory(doc)
