#include "comet.h"
#include "cometlib.h"

void init_stdlib(void)
{
    init_object();
    init_file();
    init_iterable();
    init_nil();
    init_list();
}
