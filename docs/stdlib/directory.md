[up](index.md)

## File
inherits [Object](object.md)

### methods


### static methods
- `list(path)` returns a [List](list.md) of [Strings](string.md) containing the contents of the directory.  Does not include the pseudo paths of '.' and '..'
- `directory?(path)` returns a [Boolean](boolean.md) if the given path is a directory
- `remove(path, [recursively])` deletes a directory from the filesystem, optionally deleting the contents first
- `delete(path, [recursively])` alias for remove
- `create(path)` creates the given directory and all parent directories required
