[up](index.md)

## json
This is a pure comet json parser.  It might one day grow to include a json serialiser.

### functions

- `parse_from_string(str)` parse the json using an in-memory string
- `parse_from_file(filename)` parse the json using the given filename to open the file

Both of these functions return a hash for objects and a list for arrays.  It does NOT detect datetimes
and parse those - they will be returned as strings.