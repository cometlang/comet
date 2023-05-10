[up](index.md)

## Directory
inherits [Object](object.md)

### methods
- `parent()` converts the directory to an absolute path and returns the parent Directory
- `absolute()` gets the Directory representing the absolute path
- `to_string()` returns the path of the directory as a [String](string.md)

### static methods
- `list(path)` returns a [List](list.md) of [Strings](string.md) containing the contents of the directory.  Does not include the pseudo paths of '.' and '..'
- `directory?(path)` returns a [Boolean](boolean.md) if the given path is a directory
- `remove(path, [recursively])` deletes a directory from the filesystem, optionally deleting the contents first
- `delete(path, [recursively])` alias for remove
- `create(path)` creates the given directory and all parent directories required
