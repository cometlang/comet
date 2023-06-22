[up](index.md)

## Module
inherits [Object](object.md)
final

Not intended to be instantiated directly.  All imported modules will be an instance of this class.

### methods
- `functions()` returns a [List](list.md) of the functions defined in the module.  This is defined as all fields that are callable and everything defined with the `function` keyword.  Does not include classes, even though they are considered callable.
- `fields()` returns a [List](list.md) of names ([Strings](string.md)) of the fields defined in the module.
- `filename()` returns a [String](string.md) representing the absolute path of the module

### operators
- `[key]` an index to the fields and functions using the name of the function (as a string) as the key
