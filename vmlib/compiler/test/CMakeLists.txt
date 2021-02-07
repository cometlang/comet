enable_testing()

set(TEST_SOURCES
    main.c
    test_pratt_parser.c
)

add_executable(compiler_tests "${TEST_SOURCES}")
target_link_libraries(compiler_tests PRIVATE compiler vmlib)
target_include_directories(compiler_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

add_test(compiler_tests compiler_tests)
