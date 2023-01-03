cmake_minimum_required (VERSION 3.0)
enable_language(C)
enable_language(CXX)
project (stdlib)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  ../comet
  ../vmlib
  ../eral
  ../stb)

set(SOURCE 
  _init.c
  boolean.c
  datetime.cpp
  directory.cpp
  duration.cpp
  enum.c
  env.cpp
  exception.c
  file_common.cpp
  functions.c
  hash.cpp
  image.c
  iterable.c
  list.cpp
  module.c
  nil.c
  number.c
  object.c
  set.c
  socket.c
  string.c
  system.c
  thread.c
  thread_synchronisation_common.c
)

if (WIN32)
list(APPEND SOURCE file_win.c thread_synchronisation_win.c)
else()
list(APPEND SOURCE file_unix.c thread_synchronisation_unix.c)
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

add_subdirectory(comet)
