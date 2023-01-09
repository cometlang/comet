[up](index.md)

## String
inherits [Iterable](iterable.md)
final

All strings are UTF-8 encoded

### methods
- `left_trim()` returns a new string with all whitespace characters from the left of the string removed
- `right_trim()` returns a new string with all whitespace characters from the right of the string removed
- `trim()` returns a new string with all whitespace characters from the left and right sides removed
- `split(str)` returns a [List](list.md) of the string portions in between any instances of `str`
- `replace(value, replacement)` returns a new string with all instances of `value` replaced by `replacement`
- `starts_with?(value)` returns `true` if the string begins exactly with value, otherwise `false`
- `ends_with?(value)` returns `true` if the string ends exactly with value, otherwise `false`
- `to_lower()` returns a new string with all uppercase letters replaced with their lowercase counterparts
- `to_upper()` returns a new string with all lowercase letters replaced with their uppercase counterparts
- `to_string()` returns self
- `value()` returns the numerical value of the string.  e.g. `'a'.value()` returns `97`
- `substring(start, [length])` returns the substring starting at the (zero-based) index of the codepoint of either the length specified or to the end of the string
- `length()` returns the number of codepoints (~letters) in the string.
- `count()` alias of length()
- `number?()` returns true if the string _only_ contains characters that can be parsed into a single number

### static methods
- `format(msg, ...)` formats a string, replacing instances of `{n}` with the nth (zero-based) index of
    argument after msg.  e.g. `String.format('this is {0} string with {0} replacement', 'a')` results in
    `'this is a string with a replacement'`
    a `{` or `}` can be escaped by doubling them up.  e.g. `String.format('this is {{0}} string with {0} replacement', a)` results in `'this is {0} string with a replacement'`

### operators
- `==` compares if the two strings are equal in a case-sensitive manner
- `+` concatenates an object (calling its `to_string()` method) onto this string, returning a new String.  The existing string is left unchanged
- `[]` gets the character (codepoint) at the given index, returned as a new string