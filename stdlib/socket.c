#include "comet.h"
#include "comet_stdlib.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static VALUE socket_type;
static VALUE socket_family;

typedef struct {
    int sock_fd;
    struct addrinfo *address;
} SocketData;

VALUE socket_open(VM *vm, VALUE klass, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *instance = (ObjNativeInstance *) newInstance(vm, AS_CLASS(klass));
    SocketData UNUSED(*data) = GET_NATIVE_INSTANCE_DATA(SocketData, OBJ_VAL(instance));
    return NIL_VAL;
}

VALUE socket_close(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, OBJ_VAL(self));
    if (data->sock_fd > 0)
    {
        close(data->sock_fd);
    }
    data->sock_fd = -1;
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

VALUE socket_bind(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE socket_accept(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE socket_listen(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, OBJ_VAL(self));
    int queued_connections = number_get_value(arguments[0]);
    if (listen(data->sock_fd, queued_connections) < 0)
    {
        throw_exception_native(vm, "IOException", "Unable to open socket: %s\n", strerror(errno));
    }
    return NIL_VAL;
}

void init_socket(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Socket", NULL, NULL, NULL, CLS_SOCKET);
    defineNativeMethod(vm, klass, &socket_open, "open", 1, true);
    defineNativeMethod(vm, klass, &socket_bind, "bind", 1, false);
    defineNativeMethod(vm, klass, &socket_accept, "accept", 0, false);
    defineNativeMethod(vm, klass, &socket_listen, "listen", 1, false);

    socket_type = enum_create(vm);
    push(vm, socket_type);
    addGlobal(copyString(vm, "SOCKET_TYPE", 11), socket_type);
    enum_add_value(vm, socket_type, "TCP", SOCK_STREAM);
    enum_add_value(vm, socket_type, "UDP", SOCK_DGRAM);
    enum_add_value(vm, socket_type, "RAW", SOCK_RAW);
    pop(vm);

    socket_family = enum_create(vm);
    push(vm, socket_family);
    addGlobal(copyString(vm, "SOCKET_FAMILY", 13), socket_family);
    enum_add_value(vm, socket_type, "UNIX", AF_UNIX);
    enum_add_value(vm, socket_type, "IPv4", AF_INET);
    enum_add_value(vm, socket_type, "IPv6", AF_INET6);
    enum_add_value(vm, socket_type, "NETLINK", AF_NETLINK);
    pop(vm);
}