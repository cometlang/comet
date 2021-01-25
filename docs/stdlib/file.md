[up](index.md)

## File
inherits [Object](object.md)

### methods
- `close()` closes the file
- `write(value)` writes the value to the file
- `read()` reads the entire content of the file into a [String](string.md) and return it
- `sync()` calls the system call `fsync` to synchronize the filesystem into which the file is written
- `flush()` calls the system call `fflush` to ensure the file buffers are written to the physical media

### static methods
- `open(path, mode)` opens a file with the given mode string (per the C file modes)
- `exists?(path)` returns a [Boolean](boolean.md) value if the current process can see the existence of the path, per the rules of the OS/filesystem
- `directory?(path)` returns a [Boolean](boolean.md) if the given path is a directory
- `file?(path)` returns a [Boolean](boolean.md) if the given path is a regular file
- `read_all_lines(path)` opens the path for reading as text, returning a [List](list.md) of the individual lines, closing the file when finished
