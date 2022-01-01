#include <stdio.h>
#include "tests.h"
#include "comet.h"

#define LIST_SIZE 64

static VM vm;
static VALUE already_sorted;
static VALUE reverse_sorted;
static VALUE jumbled;

static VALUE sort_func_name_string;

static int jumbled_ints[LIST_SIZE] = {
    35, 13, 10, 41, 64, 36, 12, 51,
     3, 42, 56, 34, 55, 19,  9, 31,
    24,  8,  6, 21, 40, 54, 17, 61,
    58, 47, 18, 11,  4, 26, 48, 14,
    53, 30, 57, 52, 45, 20, 29, 46,
    63, 27, 59, 49, 43, 16,  2,  1,
    23, 39, 38,  7,  5, 37, 28, 44,
    33, 62, 32, 22, 15, 60, 50, 25,
};

void test_list_setup(void)
{
    init_comet(&vm);
    VALUE created;
    already_sorted = list_create(&vm);
    push(&vm, already_sorted);
    for (int i = 0; i < LIST_SIZE; i++) {
        created = create_number(&vm, i + 1);
        push(&vm, created);
        list_add(&vm, already_sorted, 1, &created);
        pop(&vm);
    }

    reverse_sorted = list_create(&vm);
    push(&vm, reverse_sorted);
    for (int i = LIST_SIZE; i > 0; i--) {
        created = create_number(&vm, i);
        push(&vm, created);
        list_add(&vm, reverse_sorted, 1, &created);
        pop(&vm);
    }

    jumbled = list_create(&vm);
    push(&vm, jumbled);
    for (int i = 0; i < LIST_SIZE; i++) {
        created = create_number(&vm, jumbled_ints[i]);
        push(&vm, created);
        list_add(&vm, jumbled, 1, &created);
        pop(&vm);
    }

    sort_func_name_string = string_create(&vm, "sort", 4);
    push(&vm, sort_func_name_string);
}

void test_list_teardown(void)
{
    already_sorted = NIL_VAL;
    reverse_sorted = NIL_VAL;
    jumbled = NIL_VAL;
    sort_func_name_string = NIL_VAL;
    deregister_thread(&vm);
}

static void assert_list_is_sorted(VALUE list)
{
    for (int i = 0; i < LIST_SIZE; i++)
    {
        VALUE index = create_number(&vm, i);
        push(&vm, index);
        VALUE num = list_get_at(&vm, list, 1, &index);
        pop(&vm);
        int UNUSED(num_val) = number_get_value(num);
        DEBUG_ASSERT((i + 1) == num_val);
    }
}

void test_list_sort_already_sorted(void)
{
    // act
    call_function(already_sorted, sort_func_name_string, 0, NULL);

    //assert
    assert_list_is_sorted(already_sorted);
}

void test_list_sort_reverse_sorted(void)
{
    // act
    call_function(reverse_sorted, sort_func_name_string, 0, NULL);

    //assert
    assert_list_is_sorted(reverse_sorted);
}

void test_list_sort_jumbled(void)
{
    // act
    call_function(jumbled, sort_func_name_string, 0, NULL);

    //assert
    assert_list_is_sorted(jumbled);
}
