[up](index.md)

## Enum - SOCKET_TYPE
 - `TCP`
 - `UDP`
 - `RAW`

## Enum - ADDRESS_FAMILY
 - `UNIX`
 - `IPv4`
 - `IPv6`
 - `NETLINK`

## Socket

### constructor
Socket(SOCKET_TYPE, ADDRESS_FAMILY)

## static methods
 - `open(SOCKET_TYPE, ADDRESS_FAMILY)`

## methods
 - `close()` Closes the socket
 - `bind(address, port)` binds the socket to an address and port. `address` is a string-based representation of the address.  The format must match the address family specified when creating the socket.
 - `accept()` accepts incoming connections, returning a new [Socket](#socket) for each successful connection
 - `listen(queue_length)` marks the socket as listening, ready to `accept` incoming connections.
 - `connect(address, port)` make an outbound connection to a specific endpoint
 - `read()` returns a [String](string.md) of any values read
 - `write(string)` writes a [String](string.md) to the socket
