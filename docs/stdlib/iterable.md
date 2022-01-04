[up](index.md)

## Iterable
inherits [Object](object.md)

Functions as an abstract class in other languages, where it has only minimal functionality and relies on the concrete implementation
to provide the basic methods.

### methods
- `contains?(value)` (abstract) returns `true` if the value is contained in the iterable
- `empty?()` (abstract) returns `true` if the iterable has no items
- `iterator()` (abstract) returns an [Iterator](#iterator)
- `min()` returns the minimum value as determined by the `<` operator
- `max()` returns the maximum value as determined by the `<` operator

## Iterator
inherits [Object](object.md)

This is just an interface, but shows what should be returned by the `iterator` method of [Iterable](#iterable)

### methods
- `has_next?()` (abstract) returns `true` if there is another item in the sequence
- `get_next()` (abstract) returns the next item in the sequence
