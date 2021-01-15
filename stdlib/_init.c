#include "comet.h"
#include "cometlib.h"

void init_stdlib(VM *vm)
{
    VALUE obj_klass = bootstrapNativeClass(vm, "Object", NULL, NULL);
    init_string(vm, obj_klass);
    init_nil(vm);
    init_exception(vm);
    init_file(vm);
    init_functions(vm);
    init_iterable(vm);
    init_list(vm);
    init_hash(vm);
    init_socket(vm);
    init_thread(vm);
}
