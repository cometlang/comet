[up](index.md)

## Number
inherits [Object](object.md)
final

A number is a 64 bit floating point number.

### methods
- `to_string()` returns a [String](string.md) representing the number
- `square_root()` returns the square root of the number
- `even?()` returns `true` if the number is evenly divisible by 2 
- `floor()` returns the nearest whole integer, searching lower numbers
- `ceiling()` returns the nearest whole integer, searching higher numbers
- `power(n)` returns number raised to the power of `n`

### static methods
- `parse()` takes a string and parses it into a `Number`.  Returns [nil](nil.md) if the string couldn't be parsed into a number.
- `max(n, m)` returns the maximum value between `n` and `m` or `m` if they are the same
- `min(n, m)` returns the minimum value between `n` and `m` or `m` if they are the same
- `random([seed])` returns a random value between 0 and 1.0 - this is NOT suitable for cryptography. Optionally
allows a seed to be specified. The seed should be able to fit into a 32 bit integer.

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
- `~` bitwise negates the number
- `<<` bitwise left shift the argument number of bits
- `>>` bitwise right shift the argument number of bits
- `|` bitwise inclusive or
- `&` bitwise and
- `^` bitwise exclusive or

All bitwise operations are performed as though the number is a signed 64 bit integer