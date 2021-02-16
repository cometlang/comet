[up](index.md)

## Number
inherits [Object](object.md)

### methods
- `to_string()` returns a [String](string.md) representing the number

### static methods
- `parse()` takes a string and parses it into a `Number`.  Returns [nil](nil.md) if the string couldn't be parsed into a number.

### operators
- `==` compares the two objects to see if they're the same instance
- `-`  performs the mathmatical subtraction operation
- `+`  performs the mathmatical addition operation
- `*`  performs the mathmatical multiplication operation
- `/`  performs the mathmatical division operation (floating point)
- `%`  performs the mathmatical remainder operation (casts both operands to uint64_t)
- `>`  performs the strict greater than comparision
- `>=` performs the greater than or equal to comparision
- `<`  performs the strict less than comparision
- `<=` performs the less than or equal to comparision

Whoops - don't have bitwise operators...