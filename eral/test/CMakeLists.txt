cmake_minimum_required (VERSION 3.0)

include(CTest)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_executable(test_FileBuffer test_FileBuffer.c)
target_link_libraries(test_FileBuffer eral)
add_test(test_FileBuffer test_FileBuffer)

add_executable(test_HashTable test_HashTable.c)
target_link_libraries(test_HashTable eral)
add_test(test_HashTable test_HashTable)

add_executable(test_List test_List.c)
target_link_libraries(test_List eral)
add_test(test_List test_List)

add_executable(test_Stack test_Stack.c)
target_link_libraries(test_Stack eral)
add_test(test_Stack test_Stack)

add_executable(test_StringBuffer test_StringBuffer.c)
target_link_libraries(test_StringBuffer eral)
add_test(test_StringBuffer test_StringBuffer)
