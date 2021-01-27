cmake_minimum_required (VERSION 3.0)
project (clox)
enable_language(C)
enable_testing()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -ggdb3 -Wall -Wextra -Werror")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS}")

set(MEMORYCHECK_COMMAND_OPTIONS "--track-origins=yes --leak-check=full --errors-for-leak-kinds=all --error-exitcode=1 --log-fd=2")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../comet
)

set(VM_SOURCE
chunk.c
chunk.h
common.h
compiler.c
compiler.h
debug.c
debug.h
import.c
import.h
mem.c
mem.h
native.c
native.h
objects.c
objects.h
scanner.c
scanner.h
table.c
table.h
value.c
value.h
vm_globals.c
vm.c
vm.h
)

add_library(vmlib STATIC ${VM_SOURCE})
target_link_libraries(vmlib stdlib)
target_include_directories(vmlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
