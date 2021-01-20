## String
inherits [Object](object.md)

Will eventually be UTF-8, but for now it's ASCII.

### methods
- left_trim()
- right_trim()
- split()
- replace(value)
- starts_with?(value)
- ends_with?(value)
- to_lower()
- to_upper()
- to_string() Returns self

### operators
- == compares if the two strings are equal in a case-sensitive manner
- + concatenates an object (calling its to_string() method) onto this string, returning a new String.  The existing string is left unchanged
