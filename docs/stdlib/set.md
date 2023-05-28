[up](index.md)

## Set
inherits [Iterable](iterable.md)
final

WARNING: known to be broken

### methods
- `add(value)` adds a value to a set.  Returns `true` if the value was added, `false` if the value was already contained in the set
- `remove(value)` removes a value from a set. Silently does nothing if the value was never in the set
- `union(set)` Returns a new Set with the combined values of both sets
- `intersect(set)` Returns a new Set containing the values that were in both sets
- `difference(set)` Returns a new Set with the values in the callee that are not in the argument Set
- `to_list()` Returns a list of the values in the Set. Not guaranteed to be in any specific order
- `to_string()` Returns a string representation of the Set.
- `empty?()` Returns `true` if there are no values in the Set.
- `count()` Returns the number of values in the Set.`

### operators
- `|` performs a union
- `&` performs an intersection
- `-` performs a difference

