#include "comet.h"
#include "cometlib.h"

void init_stdlib(VM *vm)
{
    VALUE obj_klass = bootstrapNativeClass("Object", NULL, NULL);
    init_string(obj_klass);
    init_object(obj_klass);
    init_exception(vm);
    init_file(vm);
    init_iterable(vm);
    init_list(vm);
    init_hash(vm);
}
