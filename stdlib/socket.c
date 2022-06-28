#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "comet.h"
#include "comet_stdlib.h"

static VALUE socket_type;
static VALUE address_family;

typedef struct {
    ObjNativeInstance obj;
    int sock_fd;
    int address_family;
    int sock_type;
} SocketData;

VALUE socket_init(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    data->sock_type = enumvalue_get_value(arguments[0]);
    data->address_family = enumvalue_get_value(arguments[1]);
    data->sock_fd = socket(data->address_family, data->sock_type, 0);
    if (data->sock_fd < 0)
    {
        throw_exception_native(vm, "SocketException", "Could not open socket: %s", gai_strerror(data->sock_fd));
    }
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

static struct addrinfo *get_address_info(VM *vm, SocketData *data, const char *ip_address, uint16_t port)
{
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    char port_string[6];
    snprintf(port_string, 6, "%u", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = data->address_family;
    hints.ai_socktype = data->sock_type;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(ip_address, port_string, &hints, &servinfo)) != 0) {
        throw_exception_native(
            vm, "SocketException", "Failed to get Address Info: %s", gai_strerror(status));
        return NULL;
    }
    return servinfo;
}

/**
 * @arg1 IP Address as a String
 * @arg2 Port number as a Number
 */
VALUE socket_connect(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    const char *ip_address = string_get_cstr(arguments[0]);
    uint16_t port = number_get_value(arguments[1]);
    struct addrinfo *address = get_address_info(vm, data, ip_address, port);

    if (address != NULL)
    {
        if (connect(data->sock_fd, address->ai_addr, address->ai_addrlen) != 0)
        {
            throw_exception_native(vm, "SocketException", "Could not connect: %s", strerror(errno));
        }
        freeaddrinfo(address);
    }
    return NIL_VAL;
}

VALUE socket_write(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    const char *string = string_get_cstr(arguments[0]);
    size_t length = strlen(string);
    size_t sent = 0;
    while (sent < length)
    {
        sent += send(data->sock_fd, &string[sent], length - sent, 0);
    }
    return NIL_VAL;
}

VALUE socket_read(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    char received[2048];
    size_t actual = recv(data->sock_fd, received, 2048, 0);
    return copyString(vm, received, actual);
}

VALUE socket_bind(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    const char *ip_address = string_get_cstr(arguments[0]);
    uint16_t port = ((uint16_t) number_get_value(arguments[1])) & 0xFFFF;
    struct addrinfo *address = get_address_info(vm, data, ip_address, port);

    if (address != NULL)
    {
        if (bind(data->sock_fd, address->ai_addr, address->ai_addrlen) != 0)
        {
            throw_exception_native(
                vm, "SocketException", "Could not bind to address %s:%u", ip_address, port);
        }
        freeaddrinfo(address);
    }
    return NIL_VAL;
}

VALUE socket_accept(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SocketData *data = GET_NATIVE_INSTANCE_DATA(SocketData, self);
    struct sockaddr peer;
    socklen_t peer_len;
    int connection_fd = accept(data->sock_fd, &peer, &peer_len);
    if (connection_fd > 0)
    {
        VALUE connection = OBJ_VAL(newInstance(vm, AS_INSTANCE(self)->klass));
        SocketData *conn_data = GET_NATIVE_INSTANCE_DATA(SocketData, connection);
        conn_data->address_family = peer.sa_family;
        conn_data->sock_fd = connection_fd;
        conn_data->sock_type = data->sock_type;
        return OBJ_VAL(connection);
    }
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
#ifdef WIN32
    WSADATA wsa;
    SOCKET s;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "Could not initialise sockets");
        exit(1);
    }
#endif

    VALUE klass = defineNativeClass(vm, "Socket", NULL, NULL, NULL, NULL, CLS_SOCKET, sizeof(SocketData), false);
    defineNativeMethod(vm, klass, &socket_init, "init", 2, false);
    defineNativeMethod(vm, klass, &socket_open, "open", 2, true);
    defineNativeMethod(vm, klass, &socket_close, "close", 0, false);
    defineNativeMethod(vm, klass, &socket_bind, "bind", 1, false);
    defineNativeMethod(vm, klass, &socket_accept, "accept", 0, false);
    defineNativeMethod(vm, klass, &socket_listen, "listen", 1, false);
    defineNativeMethod(vm, klass, &socket_connect, "connect", 1, false);
    defineNativeMethod(vm, klass, &socket_read, "read", 0, false);
    defineNativeMethod(vm, klass, &socket_write, "write", 1, false);

    socket_type = enum_create(vm);
    push(vm, socket_type);
    addGlobal(copyString(vm, "SOCKET_TYPE", 11), socket_type);
    enum_add_value(vm, socket_type, "TCP", SOCK_STREAM);
    enum_add_value(vm, socket_type, "UDP", SOCK_DGRAM);
    enum_add_value(vm, socket_type, "RAW", SOCK_RAW);
    pop(vm);

    address_family = enum_create(vm);
    push(vm, address_family);
    addGlobal(copyString(vm, "ADDRESS_FAMILY", 14), address_family);
    enum_add_value(vm, address_family, "UNIX", AF_UNIX);
    enum_add_value(vm, address_family, "IPv4", AF_INET);
    enum_add_value(vm, address_family, "IPv6", AF_INET6);
#ifndef WIN32
    enum_add_value(vm, address_family, "NETLINK", AF_NETLINK);
#endif
    pop(vm);
}

#ifdef WIN32
void socket_cleanup(void)
{
    WSACleanup();
}
#endif