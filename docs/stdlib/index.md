# Classes

## Object
The base class for all Objects
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object
### operators
- == compares if the two objects are the same instance

## Boolean
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object
### operators
- == compares if the two objects are the same instance

## Datetime
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object
### operators
- == compares if the two objects are the same instance

## Enum
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object
### operators
- == compares if the two objects are the same instance

## Exception
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object
### operators
- == compares if the two objects are the same instance

## File
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object
### operators
- == compares if the two objects are the same instance

## Hash
This is known collectively as a Hash, Hash Table, Dictionary and probably other names.  It can be instantiated with a literal `{}` or statically initalized with `{key: value, ...}`
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object

### operators
- == compares if the two objects are the same instance
- [] takes an object as the key and returns the associated value, e.g. `hash[some_object]`
- []= assigns a value to the given key, e.g. `hash[some_object] = "some string"`

## Iterable
## List
This is technically an ArrayList, i.e. the memory is contiguous which makes indexing faster, at the cost of adding to the list might trigger a reallocation and that performance will linearly degrade as the list size increases.  It can be instantiated with a literal `[]` or statically initialized with `[value, "some string", ...]`
### constructor
- `List([initial_capacity])`
If given an `initial_capactiy` then the list will be pre-allocated to contain that number of values.  This means the index-assign operator can then be used to sparsely populate the list.
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object
### operators
- == compares if the two objects are the same instance
- [] returns the value at the given index (must be an integer)
- []= assigns the given value to the given index (which must be an integer)

## Nil
Nil is a special class with exactly one instance - no further instances can be instantiated.
### methods
- nil?() Returns true
- hash() Returns a hash value for this object
- to_string() Returns a String representation of this object
### operators
- == compares if the two objects are the same instance

## Number
## Set
## Socket

## String
Will eventually be UTF-8, but for now it's ASCII.
### methods
- nil?() Returns a Boolean as to whether the instance is nil or not
- hash() Returns a hash value for this object
- to_string() Returns self
### operators
- == compares if the two strings are equal in a case-sensitive manner
- + concatenates an object (calling its to_string() method) onto this string, returning a new String.  The existing string is left unchanged

## Thread
Intended to be a real OS-level thread.  The architecture mostly allows for it (GC is my big worry here).
### methods

# Functions
## clock
## print
