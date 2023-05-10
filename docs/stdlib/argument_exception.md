[up](index.md)

## ArgumentException
inherits [Exception](exception.md)

### static methods
- `throw_if_nil()` throw an ArgumentException if the given value is `nil`
- `throw_if_empty()` throw an ArgumentException if the given value returns `true` to `empty?` (which `nil` does)
- `throw_if_nil_or_empty()` throw an ArgumentException if the given value is `nil` or returns `true` to `empty?`
- `throw_if_nil_or_whitespace()` throw an ArgumentException if the given value is `nil`, returns `true` to `empty?` or is a string made up of entirely whitespace characters.  This will also throw an ArgumentException itself if the given value is neither `nil` nor a [String](string.md). 
