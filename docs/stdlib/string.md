[up](index.md)

## String
inherits [Object](object.md)

Will eventually be UTF-8, but for now it's ASCII.

### methods
- `left_trim()` returns a new string with all whitespace characters from the left of the string removed
- `right_trim()` returns a new string with all whitespace characters from the right of the string removed
- `split(str)` returns a [List](list.md) of the string portions in between any instances of `str`
- `replace(value, replacement)` returns a new string with all instances of `value` replaced by `replacement`
- `starts_with?(value)` returns `true` if the string begins exactly with value, otherwise `false`
- `ends_with?(value)` returns `true` if the string ends exactly with value, otherwise `false`
- `to_lower()` returns a new string with all uppercase letters replaced with their lowercase counterparts
- `to_upper()` returns a new string with all lowercase letters replaced with their uppercase counterparts
- `to_string()` returns self

### operators
- `==` compares if the two strings are equal in a case-sensitive manner
- `+` concatenates an object (calling its `to_string()` method) onto this string, returning a new String.  The existing string is left unchanged
