#include <stdio.h>

#include "comet.h"
#include "cometlib.h"
#include "comet_stdlib.h"
#include "file_common.h"

VALUE fopen_params;

VALUE file_static_rename(VM *vm, VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    const char *old_name = string_get_cstr(arguments[0]);
    const char *new_name = string_get_cstr(arguments[1]);
    rename(old_name, new_name);
    return NIL_VAL;
}

void init_file(VM* vm)
{
    VALUE klass = defineNativeClass(vm, "File", &file_constructor, &file_destructor, NULL, "Object", CLS_FILE, sizeof(FileData), false);
    defineNativeMethod(vm, klass, &file_static_open, "open", 2, true);
    defineNativeMethod(vm, klass, &file_close, "close", 0, false);
    defineNativeMethod(vm, klass, &file_write, "write", 1, false);
    defineNativeMethod(vm, klass, &file_read, "read", 0, false);
    defineNativeMethod(vm, klass, &file_sync, "sync", 0, false);
    defineNativeMethod(vm, klass, &file_flush, "flush", 0, false);
    defineNativeMethod(vm, klass, &file_static_exists_q, "exists?", 1, true);
    defineNativeMethod(vm, klass, &file_static_directory_q, "directory?", 1, true);
    defineNativeMethod(vm, klass, &file_static_file_q, "file?", 1, true);
    defineNativeMethod(vm, klass, &file_static_read_all_lines, "read_all_lines", 1, true);
    defineNativeMethod(vm, klass, &file_static_delete, "delete", 1, true);
    defineNativeMethod(vm, klass, &file_static_rename, "rename", 2, true);

    fopen_params = enum_create(vm);
    push(vm, fopen_params);
    addGlobal(copyString(vm, "FOPEN", 5), fopen_params);
    enum_add_value(vm, fopen_params, "READ_ONLY", FOPEN_READ_ONLY);
    enum_add_value(vm, fopen_params, "READ_WRITE", FOPEN_READ_WRITE);
    enum_add_value(vm, fopen_params, "APPEND", FOPEN_APPEND);
    enum_add_value(vm, fopen_params, "BINARY", FOPEN_BINARY);
    pop(vm);
}
