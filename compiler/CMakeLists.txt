cmake_minimum_required (VERSION 3.0)
enable_language(C)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/../eral"
    "${CMAKE_CURRENT_SOURCE_DIR}/../lexer"
)

set(SOURCE
    ast.c
    ast.h
    expression.c
    expression.h
)

add_library(compiler STATIC ${SOURCE})
target_link_libraries(compiler PRIVATE eral lexer)

# add_subdirectory(test)
