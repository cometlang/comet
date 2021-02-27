#include "comet.h"
#include "comet_stdlib.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static VALUE socket_type;
static VALUE address_family;

typedef struct {
    int sock_fd;
} SocketData;

VALUE socket_init(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    int af = enumvalue_get_value(arguments[0]);
    data->sock_fd = socket(af, enumvalue_get_value(arguments[1]), 0);
    return NIL_VAL;
}

VALUE socket_open(VM *vm, VALUE klass, int UNUSED(arg_count), VALUE *arguments)
{
    VALUE instance = OBJ_VAL(newInstance(vm, AS_CLASS(klass)));
    push(vm, instance);
    socket_init(vm, instance, arg_count, arguments);
    return pop(vm);
}

VALUE socket_close(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    if (data->sock_fd > 0)
    {
        close(data->sock_fd);
    }
    data->sock_fd = -1;
    return NIL_VAL;
}


/**
 * @arg1 ADDRESS_FAMILY
 * @arg2 IP Address as a String
 * @arg3 Port number
 */
VALUE socket_connect(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    int af = enumvalue_get_value(arguments[0]);
    struct sockaddr address;
    inet_pton(af, string_get_cstr(arguments[1]), &address);
    uint16_t port = number_get_value(arguments[2]);
    socklen_t length;
    if (af == AF_INET)
    {
        ((struct sockaddr_in *) &address)->sin_port = port;
        length = sizeof(struct sockaddr_in);
    }
    else if (af == AF_INET6)
    {
        ((struct sockaddr_in6 *) &address)->sin6_port = port;
        length = sizeof(struct sockaddr_in6);
    }

    if (connect(data->sock_fd, &address, length) != 0)
    {
        throw_exception_native(vm, "SocketException", "Could not connect: %s", strerror(errno));
    }
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

VALUE socket_listen(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, OBJ_VAL(self));
    int queued_connections = number_get_value(arguments[0]);
    if (listen(data->sock_fd, queued_connections) < 0)
    {
        throw_exception_native(vm, "SocketException", "Unable to open socket: %s\n", strerror(errno));
    }
    return NIL_VAL;
}

void init_socket(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Socket", NULL, NULL, NULL, CLS_SOCKET);
    defineNativeMethod(vm, klass, &socket_init, "init", 2, false);
    defineNativeMethod(vm, klass, &socket_open, "open", 2, true);
    defineNativeMethod(vm, klass, &socket_bind, "bind", 1, false);
    defineNativeMethod(vm, klass, &socket_accept, "accept", 0, false);
    defineNativeMethod(vm, klass, &socket_listen, "listen", 1, false);
    defineNativeMethod(vm, klass, &socket_connect, "connect", 1, false);

    socket_type = enum_create(vm);
    push(vm, socket_type);
    addGlobal(copyString(vm, "SOCKET_TYPE", 11), socket_type);
    enum_add_value(vm, socket_type, "TCP", SOCK_STREAM);
    enum_add_value(vm, socket_type, "UDP", SOCK_DGRAM);
    enum_add_value(vm, socket_type, "RAW", SOCK_RAW);
    pop(vm);

    address_family = enum_create(vm);
    push(vm, address_family);
    addGlobal(copyString(vm, "ADDRESS_FAMILY", 13), address_family);
    enum_add_value(vm, address_family, "UNIX", AF_UNIX);
    enum_add_value(vm, address_family, "IPv4", AF_INET);
    enum_add_value(vm, address_family, "IPv6", AF_INET6);
    enum_add_value(vm, address_family, "NETLINK", AF_NETLINK);
    pop(vm);
}