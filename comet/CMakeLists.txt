cmake_minimum_required (VERSION 3.0)
enable_language(C)

add_executable(comet main.c)
if (WIN32)
add_compile_definitions(WIN32=1)
target_link_libraries(comet PRIVATE stdlib vmlib)
SET_TARGET_PROPERTIES(comet PROPERTIES LINK_FLAGS "/link setargv.obj")
else()
target_link_libraries(comet PRIVATE stdlib vmlib m c)
endif (WIN32)
target_include_directories(comet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

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
  "$<TARGET_FILE:comet>"
    "${toplevel_source_dir}/stdlib/comet/unittest.cmt"
    "--coverage"
    "${toplevel_source_dir}/test_scripts/*.cmt"
WORKING_DIRECTORY "${toplevel_source_dir}"
DEPENDS comet
)

add_custom_target(memtest
${CMAKE_COMMAND} -E env "COMET_LIB_DIR=${toplevel_source_dir}/stdlib/comet"
  "valgrind"
  "$<TARGET_FILE:comet>"
    "${toplevel_source_dir}/stdlib/comet/unittest.cmt"
    "--coverage"
    "${toplevel_source_dir}/test_scripts/*.cmt"
WORKING_DIRECTORY "${toplevel_source_dir}"
DEPENDS comet
)