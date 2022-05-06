cmake_minimum_required (VERSION 3.0)
project (clox)
enable_language(C)
enable_language(CXX)
enable_testing()

if (WIN32)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -ggdb3 -Wall -Wextra -Werror")
endif (WIN32)
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS}")

set(MEMORYCHECK_COMMAND_OPTIONS "--track-origins=yes --leak-check=full --errors-for-leak-kinds=all --error-exitcode=1 --log-fd=2")

add_subdirectory(lexer)
add_subdirectory(compiler)

set(VM_SOURCE
chunk.c
chunk.h
common.h
import.cpp
import.h
mem.c
mem.h
native.c
native.h
objects.c
objects.h
table.c
table.h
value.c
value.h
vm_globals.c
vm.c
vm.h
)

add_library(vmlib STATIC ${VM_SOURCE})
target_include_directories(vmlib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../comet
)

set_property(TARGET vmlib PROPERTY CXX_STANDARD 20)

target_link_libraries(vmlib stdlib compiler lexer Threads::Threads)
target_include_directories(vmlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
