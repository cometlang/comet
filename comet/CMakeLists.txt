cmake_minimum_required (VERSION 3.0)
enable_language(C)

add_executable(comet main.c coverage.cpp)
if (WIN32)
add_compile_definitions(WIN32=1)
target_link_libraries(comet PRIVATE stdlib vmlib)
else()
target_link_libraries(comet PRIVATE stdlib vmlib m c)
endif (WIN32)
target_include_directories(comet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(comet PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private")

install(TARGETS comet
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)

set (git_cmd "git")
execute_process(COMMAND "git" "describe" "--tags"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE VERSION)

target_compile_definitions(comet PRIVATE VERSION_STRING=${VERSION})

set (toplevel_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/..")
add_custom_target(stdlib_test
${CMAKE_COMMAND} -E env "COMET_LIB_DIR=${toplevel_source_dir}/stdlib/comet"
  "${CMAKE_CURRENT_BIN_DIR}/$<TARGET_FILE:comet>"
    "--coverage"
    "${toplevel_source_dir}/stdlib/comet/unittest.cmt"
    "${toplevel_source_dir}/test_scripts/*.cmt"
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
DEPENDS comet
)