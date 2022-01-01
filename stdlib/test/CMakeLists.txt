enable_testing()

set(COMET_STDLIB_TEST_SOURCES
    main.c
    test_list.c
)

add_executable(stdlib_tests "${COMET_STDLIB_TEST_SOURCES}")
target_link_libraries(stdlib_tests PRIVATE vmlib stdlib)
target_include_directories(stdlib_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

add_test(stdlib_tests stdlib_tests)
