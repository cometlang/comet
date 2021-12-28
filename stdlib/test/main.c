#include <stdlib.h>

#include "common.h"
#include "tests.h"

int main(int UNUSED(argc), char UNUSED(**argv))
{
    test_list_setup();

    test_list_sort_already_sorted();
    test_list_sort_reverse_sorted();
    test_list_sort_jumbled();

    test_list_teardown();

    return EXIT_SUCCESS;
}