cmake_minimum_required (VERSION 3.0)
enable_language(C)
project (stdlib)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  ../comet
  ../vmlib
  ../eral)

set(SOURCE 
  _init.c
  boolean.c
  # datetime.c
  enum.c
  exception.c
  file.c
  functions.c
  hash.c
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

add_library(stdlib STATIC "${SOURCE}")
target_link_libraries(stdlib vmlib utf8proc)
target_include_directories(stdlib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private")
target_include_directories(stdlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(test)
