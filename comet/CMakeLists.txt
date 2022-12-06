cmake_minimum_required (VERSION 3.0)
enable_language(C)

add_executable(comet main.c)
if (WIN32)
target_link_libraries(comet PRIVATE stdlib vmlib)
else()
target_link_libraries(comet PRIVATE stdlib vmlib m c)
endif (WIN32)
target_include_directories(comet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS comet
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)

if (NOT VERSION)
set (git_cmd "git")
execute_process(COMMAND "git" "describe" "--tags"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE VERSION)
endif()

target_compile_definitions(comet PRIVATE VERSION_STRING=${VERSION})

message(NOTICE "CURRENT BIN DIR: ${CMAKE_CURRENT_BINARY_DIR}")