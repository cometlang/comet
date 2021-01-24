cmake_minimum_required (VERSION 3.0)
project (comet)
enable_language(C)
enable_testing()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -ggdb3 -Wall -Wextra -Werror -flto")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS}")

set(MEMORYCHECK_COMMAND_OPTIONS "--track-origins=yes --leak-check=full --errors-for-leak-kinds=all --error-exitcode=1 --log-fd=2")
add_subdirectory(utf8proc-2.6.1)
add_subdirectory(stdlib)
add_subdirectory(vmlib)
add_subdirectory(comet)
