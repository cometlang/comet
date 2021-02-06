include(src/sources.cmake)
add_library(compiler STATIC "${COMPILER_SOURCES}")

target_include_directories(
    compiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../comet/
)
target_link_libraries(compiler lexer comet)
