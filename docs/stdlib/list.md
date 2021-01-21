## List
inherits [Iterable](iterable.md)

This is technically an ArrayList, i.e. the memory is contiguous which makes indexing faster, at the cost of adding to the list might trigger a reallocation and that performance will linearly degrade as the list size increases.  It can be instantiated with a literal `[]` or statically initialized with `[value, "some string", ...]`

### constructor
- `List([initial_capacity])`
If given an `initial_capactiy` then the list will be pre-allocated to contain that number of values.  This means the index-assign operator can then be used to sparsely populate the list.

### methods
- `add(value)` appends the value to the end of the list
- `push(value)` alias for `add`
- `pop()` removes and returns the value at the end of the list
- `get_at(index)` returns the value at the given index, but does not remove it from the list
- `to_string()` returns a string representation of the list
- `size()` returns the number of items stored in the list
- `length()` alias for `size()`
- `sort()` returns a sorted shallow copy of the list
- `sort!()` sorts the list in-place, returning `self`

### operators
- `==` compares the contents of the list to another list to see if the contents (and order!) are identical
- `[]` returns the value at the given index (must be an integer)
- `[]=` assigns the given value to the given index (which must be an integer)