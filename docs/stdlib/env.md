[up](index.md)

## ENV
A singleton object that gives access to the environment variables in the system.

## operators
- `[varname]` returns a string with the value of the environment variable.  Throws an [Exception](exception.md) if there was no environment variable with that name.
- `[varname]=value` Sets an environment variable of name `varname` to `value`.  Throws an [Exception](exception.md) if the value is not a [String](string.md). If `value` is [nil](nil.md) it unsets the environment variable