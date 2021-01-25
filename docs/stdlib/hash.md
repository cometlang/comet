[up](index.md)

## Hash
inherits [Iterable](iterable.md)

This is known collectively as a Hash, Hash Table, Dictionary and probably other names.  It can be instantiated with a literal `{}` or statically initalized with `{key: value, ...}`

### methods
- `add(key, value)` add a value to the hash with key
- `remove(key)` removes the value and key from the hash
- `to_string()` returns a String representation of the hash

### operators
- `==` iterates through the keys and values of each hash and returns true if they have the same contents (or are the same instance)
- `[]` takes an object as the key and returns the associated value, e.g. `hash[some_object]`
- `[]=` assigns a value to the given key, e.g. `hash[some_object] = "some string"`
