[up](index.md)

## Object
The base class for all Objects

### methods
- `nil?()` Returns a Boolean as to whether the instance is nil or not
- `hash()` Returns a hash value for this object
- `to_string()` Returns a String representation of this object
- `methods()` Returns a [List](list.md) of [Strings](string.md) with the names of all callables on the object
- `fields()` Returns a [List](list.md) of [Strings](string.md) of all the fields on the object

### static methods
- `to_string()` Returns a string representation of the class
- `hash()` This is identical to the non-static method, but added such that classes can be used as a key to a hash

### operators
- `==` compares if the two objects are the same instance
