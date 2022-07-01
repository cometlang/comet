[up](index.md)

## csv
This is currently an aspirational csv parser.  It is aspirational, because the implementation
as it stands, is `string.split(',')`.  It will get better.

### functions

- `parse_from_string(str, has_header = false)` parse the csv using an in-memory string
- `parse_from_file(filename, has_header = false)` parse the csv using the given filename to open the file

Both of these functions return an iterable containing a `CsvRow` which is itself iterable, and will maintain
the order as specified in the source.