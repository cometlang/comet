cmake_minimum_required (VERSION 3.0)
enable_language(C)
project (eral)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCE 
  string.c
)

add_library(eral ${SOURCE})

add_subdirectory(test)
