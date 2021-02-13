enable_testing()

set(TEST_SOURCES
    main.c
    test_scanner.c
)

add_executable(lexer_tests "${TEST_SOURCES}")
target_link_libraries(lexer_tests PRIVATE lexer vmlib)
target_include_directories(lexer_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

add_test(lexer_tests lexer_tests)
