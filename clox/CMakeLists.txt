cmake_minimum_required (VERSION 3.0)
project (clox)
enable_language(C)
enable_testing()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -ggdb3 -Wall -Wextra -Werror --coverage -fprofile-arcs -ftest-coverage -flto")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} --coverage")

set(MEMORYCHECK_COMMAND_OPTIONS "--track-origins=yes --leak-check=full --errors-for-leak-kinds=all --error-exitcode=1 --log-fd=2")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/../eral"
)

set(SOURCE
chunk.c
chunk.h
common.h
compiler.c
compiler.h
debug.c
debug.h
main.c
memory.c
memory.h
object.c
object.h
scanner.c
scanner.h
table.c
table.h
value.c
value.h
vm.c
vm.h
)

add_executable(clox ${SOURCE})
target_link_libraries(clox PUBLIC c m)

