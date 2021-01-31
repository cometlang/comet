#include "comet.h"
#include "cometlib.h"

void init_stdlib(VM *vm)
{
    VALUE obj_klass = bootstrapNativeClass(vm, "Object", NULL, NULL);
    bootstrap_number(vm);
    bootstrap_iterable(vm);
    init_string(vm, obj_klass);
    complete_number(vm);
    init_nil(vm);
    init_enum(vm);
    init_boolean(vm);
    init_exception(vm);
    init_file(vm);
    init_functions(vm);
    init_list(vm);
    init_hash(vm);
    init_socket(vm);
    init_thread(vm);
}
