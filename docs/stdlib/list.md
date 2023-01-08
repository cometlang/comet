[up](index.md)

## List
inherits [Iterable](iterable.md)
final

This is technically an ArrayList, i.e. the memory is contiguous which makes indexing faster, at the cost of adding to the list might trigger a reallocation and that performance will linearly degrade as the list size increases.  It can be instantiated with a literal `[]` or statically initialized with `[value, "some string", ...]`

### constructor
- `List([initial_capacity])`
If given an `initial_capactiy` then the list will be pre-allocated to contain that number of values.  This means the index-assign operator can then be used to sparsely populate the list.

### methods
- `add(value)` appends the value(s) to the end of the list - takes multiple arguments and will add them all to the list, in order.
- `push(value)` alias for `add`
- `pop()` removes and returns the value at the end of the list
- `get_at(index)` returns the value at the given index, but does not remove it from the list
- `to_string()` returns a string representation of the list
- `size()` returns the number of items stored in the list
- `length()` alias for `size()`
- `sort()` sorts the list in-place and returns a reference to the list. It uses Timsort, a stable sort running in O(n log n) time.
- `filter()` takes a callable object which is called with every item, returning `true` if the item should be part of the list returned.  The list returned is a new object and the initial list is left unchanged.
- `map(lambda)` returns a list of values as mapped by the lambda, which is called with each item in the list
- `reduce(initial, lambda)` given an intial value, the lambda is called with the current reduction, each item in the list, and the index in the list, e.g. `list.reduce(0, |current, item, index| { return curent + 1 })`

### operators
- `==` compares the contents of the list to another list to see if the contents (and order!) are identical
- `[]` returns the value at the given index (must be an integer)
- `[]=` assigns the given value to the given index (which must be an integer)
- `+` appends all the items in the argument list to the original