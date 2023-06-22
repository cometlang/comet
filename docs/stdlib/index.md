[up](../index.md)

# Classes

- [Object](object.md)
- [ArgumentException](argument_exception.md)
- [Boolean](boolean.md)
- [Datetime](datetime.md)
- [Directory](directory.md)
- [Duration](duration.md)
- [Enum](enum.md)
- [ENV](env.md)
- [Exception](exception.md)
- [File](file.md)
- [Function](function.md)
- [Hash](hash.md)
- [Image](image.md)
- [Iterable](iterable.md)
- [List](list.md)
- [Module](module.md)
- [Nil](nil.md)
- [Number](number.md)
- [Set](set.md)
- [Socket](socket.md)
- [String](string.md)
- [StringBuilder](string_builder.md)
- [Thread](thread.md)
- [Thread Synchronisation Primitives](thread_synchronisation.md)
- [UnitTest](unittest.md)

# Modules
- [csv](csv.md)
- [json](json.md)
- [unittest](unittest.md)

# Functions
## clock
- `clock()` returns a [Number](number.md) representing the fractional number of seconds of CPU time the process has used

## print
- `print([...])` print every argument by first calling `to_string()` on it first, ending with a newline

## print_to
- `print_to(STD_STREAM, [...])` print every argument by first calling `to_string()` on it first, ending with a newline.  It will print it to the given stream.  The options are STD_STREAM.OUT and STD_STREAM.ERR

## input
- `input([prompt])` print the prompt (no newline) if given and then wait for a line of input, returning it as a [String](string.md).  Gets a maximum of 512 bytes in one line.

## get_password
- `get_password([prompt])` print the prompt (no newline) if given and then wait for a line of input, returning it as a [String](string.md).  Does not echo the input.  Gets a maximum of 512 bytes in one line.

## assert
- `assert(value)` if value is falsy, then an AssertionException is thrown

## callable?
- `callable?(value)` returns `true` if the value can be called (like a function)

## sleep
- `sleep(time)` stops the execution of the current thread for at least the fractional seconds provided.

## exit
- `exit(exit_status)` simple wrapper for the C function to exit the program with the given return code.  Makes no attempt to clean anything up.

## get_imported_modules
- `get_imported_modules()` gets all of the [Modules](module.md) currently imported to the interpreter

## call_function
- `call_function(receiver, method, [arguments, ...])` calls the method on the receiver.  If the method is a function object, then the receiver may be `nil`.  If the method is a name of a method (a string), then the receiver must be an instance.
