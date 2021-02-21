#include "comet.h"
#include "comet_stdlib.h"

VALUE socket_init(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE socket_open(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE socket_close(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE socket_write(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE socket_read(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}


void init_socket(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Socket", NULL, NULL, NULL, CLS_SOCKET);
    defineNativeMethod(vm, klass, &socket_init, "init", 0, false);

    VALUE socket_type = enum_create(vm);
    push(vm, socket_type);
    addGlobal(copyString(vm, "SOCKET_TYPE", 11), socket_type);
    enum_add_value(vm, socket_type, "TCP", 0);
    enum_add_value(vm, socket_type, "UDP", 1);
    enum_add_value(vm, socket_type, "UNIX", 2);
    enum_add_value(vm, socket_type, "RAW", 3);
    pop(vm);
}