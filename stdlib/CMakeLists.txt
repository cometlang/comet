cmake_minimum_required (VERSION 3.0)
enable_language(C)
project (stdlib)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  ../comet
  ../clox
  ../eral)
set(SOURCE 
  _init.c
  # boolean.c
  # datetime.c
  file.c
  # hash.c
  iterable.c
  list.c
  nil.c
  number.c
  object.c
  set.c
  socket.c
  string.c
  system.c
  thread.c
)

add_library(stdlib STATIC ${SOURCE})

add_subdirectory(test)
