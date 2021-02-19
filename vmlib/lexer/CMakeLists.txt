include(src/sources.cmake)

add_library(lexer STATIC "${LEXER_SOURCES}")
target_include_directories(
    lexer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    add_subdirectory(test)
endif()
