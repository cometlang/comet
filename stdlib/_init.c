#include "comet.h"
#include "cometlib.h"

void init_stdlib(void)
{
    init_functions();
    init_object();
    init_nil();
    init_exception();
    init_file();
    init_iterable();
    init_list();
    init_hash();
    //init_string();
}
