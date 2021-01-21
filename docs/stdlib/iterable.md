## Iterable
inherits [Object](object.md)

Functions as an interface in other languages, where it doesn't provide any of its own functionality aside from `instanceof` and showing which methods need to be implemented for iterable functionality (like `foreach` loops)

### methods
- `contains?(value)` returns `true` if the value is contained in the iterable
- `empty?()` returns `true` if the iterable has no items
- `iterator()` returns an [Iterator](#iterator)


## Iterator
inherits [Object](object.md)

Again, this is just an interface, but shows what should be returned by the `iterator` method of [Iterable](#iterable)

### methods
- `has_next?()` returns `true` if there is another item in the sequence
- `get_next()` returns the next item in the sequence
