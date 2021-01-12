#include "comet.h"
#include "cometlib.h"

void init_stdlib(void)
{
    VALUE obj_klass = bootstrapNativeClass("Object", NULL, NULL);
    init_string(obj_klass);
    completeNativeClassDefinition(obj_klass, "Object");
    init_object(obj_klass);
    init_functions();
    init_nil();
    init_exception();
    init_file();
    init_iterable();
    init_list();
    init_hash();
}
