#include "comet.h"
#include "cometlib.h"

void init_comet(VM* vm)
{
    initGlobals();
    initVM(vm);
    init_stdlib(vm);
    common_strings[STRING_INIT] = copyString(vm, "init", 4);
    common_strings[STRING_HASH] = copyString(vm, "hash", 4);
    common_strings[STRING_TO_STRING] = copyString(vm, "to_string", 9);
    common_strings[STRING_LAMBDA] = copyString(vm, "(|lambda|)", 10);
    common_strings[STRING_EMPTY_Q] = copyString(vm, "empty?", 6);
    common_strings[STRING_NUMBER] = copyString(vm, "Number", 6);
}

void init_stdlib(VM *vm)
{
    VALUE obj_klass = bootstrapNativeClass(vm, "Object", NULL, NULL, CLS_OBJECT, 0, false);
    bootstrap_number(vm);
    bootstrap_iterable(vm);
    init_string(vm, obj_klass);
    complete_number(vm);
    init_nil(vm);
    init_enum(vm);
    init_boolean(vm);
    init_exception(vm);
    init_file(vm);
    init_module(vm);
    init_functions(vm);
    init_list(vm);
    init_hash(vm);
    init_image(vm);
    init_datetime(vm);
    init_duration(vm);
    init_socket(vm);
    init_thread(vm);
    init_thread_sync(vm);
    init_env(vm);
    init_set(vm);
    init_directory(vm);
    init_string_builder(vm);
}
