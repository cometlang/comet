[up](index.md)

## unittest
A basic unit testing framework

Can be run as a script to run other tests.  All `function`s in the module that start with the name `test_` are determined to be tests to run.

test_script.cmt:
```
import 'unittest' as unittest

function test_a_thing() {
    unittest.Assert.that('this is a string').is_not_nil()
}
```

`comet unittest [--coverage] test_script.cmt`

The script will exit with a non-zero exit code if any of the tests fail.

If the `--coverage` parameter is specified, then a json document with the count of the number of times each line was executed, along with the per-function and per-file totals and percentages called `.coverage.json` in the current directory is created.

### classes
- `Assert`

#### static methods
- `that(value)` returns an instance of `Assert`, initialised with the value to be tested against
- `fail([message])` fails the test, optionally giving a message for why the test failed
- `pass()`

#### methods
- `is_nil(message)` asserts that the value is nil, optionally giving a message when the assertion fails
- `is_not_nil(message)` asserts that the value is not nil, optionally giving a message when the assertion fails
- `is_equal_to(expected, message)` asserts that the value is equal to (as determined by the `==` operator) to the expected, optionally giving a message when the assertion fails
- `is_not_equal_to(expected, message)` asserts that the value is not equal to (as determined by the negation of the `==` operator) to the expected, optionally giving a message when the assertion fails
- `is_true(message)` asserts that the value is true, optionally giving a message when the assertion fails
- `is_false(message)` asserts that the value is false, optionally giving a message when the assertion fails
- `contains(expected, message)` asserts that the value contains the expected (as determined by calling `value.contains?(expected)`), optionally giving a message when the assertion fails
- `does_not_contain(expected, message)` asserts that the value does not contain the expected (as determined by calling `!value.contains?(expected)`), optionally giving a message when the assertion fails
- `matches(expected, message)` asserts that the value matches the expect.  Determined by either calling expected if the expected is callable, returning a boolean or falling back to the `==` operator, optionally giving a message when the assertion fails
- `throws(message)` asserts that the value throws an exception (other than an AssertionException) when it is called, optionally giving a message when the assertion fails. Throws an ArgumentException itself if the value is not callable.
- `does_not_throw(message)` asserts that the value does not throw an exception when called, optionally giving a message when the assertion fails. Throws an ArgumentException itself if the value is not callable.
- `is_of_type(type, message)` asserts that the value `is` the given type, optionally giving a message for when the assertion fails
- `is_empty(message)` asserts that the value returns `true` to `empty?()`, optionally giving a message for when the assertion fails
- `is_not_empty(message)` asserts that the value returns `false` to `empty?()`, optionally giving a message for when the assertion fails
- `is_callable(message)` asserts that the value is callable 

