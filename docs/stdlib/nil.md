[up](index.md)

## Nil
inherits [Iterable](iterable.md)
final

Nil is a special class with exactly one instance - it cannot be instantiated.  It can be treated as a permanently empty iterable that will never have contents to iterate, but does not require checking for nil? before using in a foreach loop.

### methods
- `nil?()` Returns true
- `to_string()` Returns the empty string "" (because checking for `nil?` can be cumbersome)
- `empty?()` returns true

### iterable
Nil is an iterable that is permanently empty (because checking for `nil?` can be cumbersome)
