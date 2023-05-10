[up](index.md)

## Duration
inherits [Object](object.md)

Represents a span of time, but not a point in time.


### constructor
 - `Duration([years], [months], [days], [hours], [minutes], [seconds], [milliseconds])` creates a duration representing the given time periods. If none of the time periods are given, a duration of 0 is created.   

### methods
 - `to_string()` returns a representation of the duration in nanoseconds with the ns unit appended

### static methods
 - `from_years(years)` creates a duration representing the number of years given
 - `from_months(months)` creates a duration representing the number of months given
 - `from_days(days)` creates a duration representing the number of days given
 - `from_hours(hours)` creates a duration representing the number of hours given
 - `from_minutes(minutes)` creates a duration representing the number of minutes given
 - `from_seconds(seconds)` creates a duration representing the number of seconds given
 - `from_milliseconds(milliseconds)` creates a duration representing the number of milliseconds given

### operators
 - `+` adds either a datetime or a duration to the existing duration.  The returned type is the same as the operand.
