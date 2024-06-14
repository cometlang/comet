cmake_minimum_required (VERSION 3.5)
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
  directory.cpp
  duration.cpp
  enum.cpp
  env.cpp
  exception.c
  file_common.cpp
  function.c
  hash.cpp
  image.c
  iterable.c
  list.cpp
  module.c
  native_functions.c
  nil.c
  number.c
  object.c
  process.cpp
  set.cpp
  socket.c
  string.c
  string_builder.c
  system.c
  thread.c
  thread_synchronisation_common.c
  colour.cpp
)

if (WIN32)
add_compile_definitions(WIN32=1)
list(APPEND SOURCE file_win.cpp thread_synchronisation_win.c datetime_win.cpp)
else()
list(APPEND SOURCE file_unix.cpp thread_synchronisation_unix.c datetime_unix.cpp)
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
