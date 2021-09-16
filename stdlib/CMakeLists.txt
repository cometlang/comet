cmake_minimum_required (VERSION 3.0)
enable_language(C)
enable_language(CXX)
project (stdlib)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  ../comet
  ../vmlib
  ../eral)

set(SOURCE 
  _init.c
  boolean.c
  datetime.cpp
  enum.c
  env.c
  exception.c
  file_common.c
  functions.c
  hash.c
  iterable.c
  list.c
  module.c
  nil.c
  number.c
  object.c
  set.c
  socket.c
  string.c
  system.c
  thread.c
)

if (WIN32)
list(APPEND SOURCE file_win.c thread_synchronisation_win.c)
else()
list(APPEND SOURCE file.c thread_synchronisation.c)
endif()

add_library(stdlib STATIC "${SOURCE}")

if (WIN32)
target_link_libraries(stdlib vmlib utf8proc ws2_32 date date-tz)
else()
target_link_libraries(stdlib vmlib utf8proc m date date-tz)
endif (WIN32)

target_include_directories(stdlib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private" Threads::Threads)
target_include_directories(stdlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(test)
