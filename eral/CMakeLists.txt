cmake_minimum_required (VERSION 3.0)
enable_language(C)
project (eral)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCE 
  fileBuffer.c
  fileBuffer.h
  fileOpener.c
  hash_table.c
  hash_table.h
  list.c
  list.h
  stack.c
  stack.h
  stringBuffer.c
  stringBuffer.h
)

add_library(eral STATIC ${SOURCE})

add_subdirectory(test)
