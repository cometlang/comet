cmake_minimum_required (VERSION 3.0)
enable_language(C)

add_executable(comet main.c)
target_link_libraries(comet PRIVATE stdlib vmlib m c)
target_include_directories(comet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS comet
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
