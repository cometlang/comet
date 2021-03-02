[up](../index.md)

# Classes

- [Object](object.md)
- [Boolean](boolean.md)
- [Datetime](datetime.md)
- [Enum](enum.md)
- [Exception](exception.md)
- [File](file.md)
- [Hash](hash.md)
- [Iterable](iterable.md)
- [List](list.md)
- [Nil](nil.md)
- [Number](number.md)
- [Set](set.md)
- [Socket](socket.md)
- [String](string.md)
- [Thread](thread.md)
- [Thread Synchronisation Primitives](thread_synchronisation.md)

# Functions
## clock
- `clock()` returns a [Number](number.md) representing the fractional number of seconds of CPU time the process has used

## print
- `print([...])` print every argument by first calling `to_string()` on it first, ending with a newline

## assert
- `assert(value)` if value is falsy, then an AssertionException is thrown

## callable?
- `callable?(value)` returns `true` if the value can be called (like a function)

## sleep
- `sleep(time)` stops the execution of the current thread for at least the fractional seconds provided.
