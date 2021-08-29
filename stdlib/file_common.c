#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"
#include "file_common.h"

VALUE fopen_params;

void init_file_common(VM *vm)
{
    fopen_params = enum_create(vm);
    push(vm, fopen_params);
    addGlobal(copyString(vm, "FOPEN", 5), fopen_params);
    enum_add_value(vm, fopen_params, "READ_ONLY", FOPEN_READ_ONLY);
    enum_add_value(vm, fopen_params, "READ_WRITE", FOPEN_READ_WRITE);
    enum_add_value(vm, fopen_params, "APPEND", FOPEN_APPEND);
    enum_add_value(vm, fopen_params, "BINARY", FOPEN_BINARY);
    pop(vm);
}
