[up](index.md)

## Enum
inherits [Iterable](iterable.md)

Not intended to be instantiated directly, but rather through the `enum NAME {}` syntax

### methods
- `add(name, value)` adds a value to the enum with the given name string and number value
- `length()` returns the number of values in the enum
- `contains?(value)` returns `true` if the given [EnumValue](#enumvalue) is part of this enum

### static methods
- `parse(value)` parses either an enum name, or an integer to the corresponding [EnumValue](#enumvalue).  Returns `nil` if no corresponding [EnumValue](#enumvalue) could be found in this enum.


## EnumValue
inherits [Number](number.md)

Not intended to be instantiated directly.

Acts like a number, because that's pretty useful in an enum value, but has a couple of extra methods / fields.

### constructor
- `EnumValue(name, value)` sets the corresponding fields name and value, along with initialising the number

### fields
- `name` returns the [String](string.md) representing the enum name
- `value` returns the [Number](number.md) representing the enum value

### methods
- `to_string()` returns a string of the form `[name]:[value]`, e.g. `my_enum_value:64`
