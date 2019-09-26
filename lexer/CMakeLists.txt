cmake_minimum_required (VERSION 3.0)
enable_language(C)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/../eral"
)

set(SOURCE 
    tokeniser.c
    tokenList.c
    tokenList.h
    tokens.c
    tokens.h
)

add_library(lexer STATIC ${SOURCE})
target_link_libraries(lexer PRIVATE eral)

# add_subdirectory(test)
