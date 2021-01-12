#include "comet.h"
#include "cometlib.h"

void init_stdlib(void)
{
    init_object();
    init_string();
    init_functions();
    init_nil();
    init_exception();
    init_file();
    init_iterable();
    init_list();
    init_hash();
}
