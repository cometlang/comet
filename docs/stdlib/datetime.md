[up](index.md)

## Datetime
inherits [Object](object.md)

## methods
- `init([year],[month],[day],[hour],[minute],[seconds],[milliseconds])` Create a datetime in the current timezone from the given values.
- `to_string()` outputs a string representation of the datetime in a json-compliant ISO8601 format.  yyyy-mm-ddTHH:MM:ss.mmmZ
- `year()` get the year portion of the datetime
- `month()` get the month portion of the datetime
- `day()` get the day portion of the datetime
- `hours()` get the hours portion of the datetime
- `minutes()` get the minutes portion of the datetime
- `seconds()` get the seconds portion of the datetime
- `milliseconds()` get the milliseconds portion of the datetime

### static methods
- `now()` returns a DateTime representing the current local time
- `parse('to_parse', ['format'])` parse a [String](string.md) into a DateTime. See [here](https://en.cppreference.com/w/cpp/chrono/parse) for format string specifiers.  `format` defaults to '%Y-%m-%dT%H:%M:%6S%z' which will parse the output of `DateTime::to_string()`

### operators
- `-` subtracts either a datetime or a duration to the existing datetime.  The returned type is the same as the operand.
- `==` compares two datetimes and returns true if they both represent the same point in time
