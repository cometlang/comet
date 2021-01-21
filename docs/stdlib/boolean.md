## Boolean
inherits [Object](object.md)

There are two singleton instances of Boolean in the interpreter accessible via the keywords `true` and `false`

### constructor
Boolean(value)
Takes a single argument, and converts to a boolean, based on the truthiness of the value.

### methods
- `to_string()` returns either "true" or "false"

### static methods
- `parse(value)` parses either a string (case insensitively) or an integer (following the C rules) or falling back to the truthiness of the value

### operators
- `==` compares by truthiness
