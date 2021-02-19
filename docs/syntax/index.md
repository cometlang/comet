# The language itself

## Scope
The scope works like C - lexical scope.  Inner scope can access the scopes out of it, but not vice-versa.  This means that variables are not "hoisted" like they are in Python or (var) JavaScript.  It does mean that functions and methods _can_ access and modify global variables without any special syntax.

Variables are resolved from the innermost scope outwards, which means that it is entirely possible to "shadow" outer variables or functions, including built-ins.

## Comments
Comments use the `#` character and go to the end of the line.  There are no block comments.

## Keywords

`as`, `catch`, `class`, `else`, `enum`, `false`, `finally` `for`, `foreach`, `function`, `if`, `import`, `in`, `instanceof`, `next`, `nil`, `operator`, `rethrow`, `return`, `self`, `super`, `throw`, `true`, `try`, `var`, `while`

## Literals

### Strings
Literal strings can be defined using either single `'` or double `"` quotes.  There isn't an interpolated string, yet. All strings are UTF-8 encoded.  Strings in Comet are immutable.

### Numbers
Currently all numbers are 64bit signed floating point values.  Literal numbers can contain underscores for your reading convenience, e.g. 1_000_000

Hexadecimal constants are supported, prefixed by 0x (not 0X). e.g. 0xdeadbeef15bad

### Lists
literal lists can be declared with `[]` for an empty list or `[val, val2, ...]` for static initialization.

### Hashes
Literal hashes can be declare with `{}` for an empty hash or `{key1: val1, key2: val2, ...}` for static initialization.

### nil
`nil` is a singleton instance of the [Nil](../stdlib/nil.md) class.  It is not possible to instantiate more instances of [Nil](../stdlib/nil.md).  `nil` is the only thing that responds `true` to the `nil?` method.  `nil` is iterable, but will never have any contents, so it's totally reasonable to iterate over `nil` using a `foreach` loop and the loop will silently be skipped.

### true and false
`true` and `false` are instances of the [Boolean](../stdlib/boolean.md) class, but unlike `nil`, they aren't singletons as it's possible to instantiate other instances of [Boolean](../stdlib/boolean.md).  This means they are compared by value, not using the object's address, making comparison incur the cost of a method call.

## Operators
### Mathematical
- `*` multiplication
- `+` addition
- `-` subtraction
- `/` division
- `%` modulo    
(the += operators are not yet implemented, though they should be accepted by the lexer)

### Comparison
- `>` greater than
- `<` less than
- `>=` greater than or equal to
- `<=` less than or equal to
- `instanceof` compares an instance to see if it is an instance of the given class, including its inheritance hierarchy

### Logical
- `&&` logical and
- `||` logical or
- `!` logical not
- `==` logical equivalency

### Bitwise (not yet implemented)
- `|` bitwise or
- `&` bitwise and
- `^` bitwise exclusive or

## Variables
Variables must be declared before use.  Local and global variables are declared using the `var` keyword.  They may be initialized at the point of declaration. All variables in `Comet` are reference-types, so there is only pass-by-reference, and never pass-by-value.  All variables in `Comet` are objects and therefore inherit from [Object](../stdlib/object.md).


```
var my_variable = 'This super cool string'
```

## Statements
There may only be one statement per line.  The semi-colon `;` is not valid to end a statement and may only be used to delineate for loop section declarations.

## Loops
The `next` statement will mean that the next iteration of the loop will be executed.
There is currently no `break` statement in Comet for loop control.  It is planned and coming.

### for
Exactly like a tradional C for loop.  `for ([declaration]; [condition]; [post action] )`

```
for (var i = 0; i < 10; i = i + 1)
{
    print(i)
}
```

### while
Exactly like a tradional C while loop. `while (condition)`
```
while (true)
{
    print('Comet is the best language')
}
```

### foreach
The foreach loops are special - they are syntactic sugar for iterating over an iterable object.  So anything that implements the [Iterable](../stdlib/iterable.md) methods can be used in a foreach loop.

```
foreach(var thing in things)
{
    ...
}
```

Is the same as writing:

```
var iterator = things.iterator()
while (iterator.has_next?())
{
    ...
    var thing = iterator.get_next()
}
```

## If statements
```
if (my_var == true)
{
    ...
}
else if (their_var == true)
{
    ...
}
else
{
    ...
}
```

## Functions
Functions (i.e. those routines not within a class) are declared via the `function` keyword and are followed by a "block".  They may have up to 255 parameters and 255 locally declared variables.  Functions always `return` a value, even if that is an implicit `nil`.


```
function my_function(parameter, other_parameter)
{
    var local_var = nil
    {
        var abitrary_scope = local_var
        print(arbitrary_scope)
    }
    return "This is a literal string"
}
```

## Classes
Classes are declared using the `class` keyword and must be uniquely named - i.e. Comet does not support open classes.  They may optionally provide a constructor, methods and operator overloads.  If no parent class is given, they will implicitly inherit from `Object`.

Method declarations do not use a keyword, but start with the name of the method itself.  Methods, operators and the constructor implicitly have access to the `self` variable referring to the current instance.  Those same things also have access to the `super` variable referring to the parent or base class.

Objects are instantiated by calling the class name with any parameters - `var instance = MySuperCoolClass("this is a string")`


```
class MySuperCoolClass
{
    my_method(parameter)
    {
        self.parameter = parameter
    }
}
```

### Constructor
This is a special method which is automatically called by the interpreter.  It must be called `init` (because it is really an initializer, because the object already exists by the time it's called).

```
class MySuperCoolClass
{
    init(arg)
    {
        self.arg = arg
        self.other = nil
    }
}
```

### Methods
Methods are declared with the name of the method, followed by an argument list, and thereafter the body. Methods are implicitly virtual, so all methods will override a parent class's method with the same name.  Methods can access the parent class method by using the `super` keyword.

```
class MySuperCoolClass
{
    my_method(arg1, arg2, arg3)
    {
        self.arg1 = arg1
        print(arg2)
        if (arg3 == true)
        {
            return false
        }
    }
}
```

### Static methods
Static methods are declared using the `static` keyword in front of the method name.  A static method does not have access to fields (there is no such thing as a static field) and neither do they have access to `super` or `self`.  They may call functions or other static methods and can still access global variables.

```
class MySuperCoolClass
{
    static my_static_method()
    {
        ...
    }
}
```

### Fields
Fields are variables assigned to instances.  Fields can have the same name as methods and will hide the methods if they do.

### Operators
Operators can be overloaded and/or overridden in classes.  This is done via the `operator` keyword and the display character of the operator.  There are several different operators that are possible to overload.  The operator functions have the same rules as methods - they can access instance fields, methods, `self`, and `super`.

The operators able to be overloaded are as follows:
- `*` (multiplication)
- `+` (addition)
- `-` (subtraction)
- `/` (division)
- `>` (greater than)
- `<` (less than)
- `>=` (greater than or equal to)
- `<=` (less than or equal to)
- `==` (comparision)
- `[]` (get by index)
- `[]=` (assign to index) - takes two arguments, the index and the value to assign

```
class MySuperCoolClass
{
    init()
    {
        self.list = List(10)
    }

    operator [] (index)
    {
        if (index > self.list.length())
        {
            throw Exception('Index out of bounds')
        }
        return self.list[index]
    }
}
```

### Inheritance
Classes can inherit from exactly one other class.  If no super class is defined, then they implicitly inherit from [Object](../stdlib/object.md).  Inheritance is marked with the `:` character.

A class will inherit all methods, static methods, fields, and operator overloads from the parent hierarchy.

```
class MySuperCoolClass : SomeOtherClass
{
    init()
    {
        super.init()
    }
}
```


## Lambda Functions
Lambda functions are anonymous functions that can access the current lexical scope.  They may be assigned to variables and/or passed to other functions/methods.  Their syntax is slightly different to named functions, but are called in exactly the same manner.


```
var my_string = 'This is a cool string'
var my_lambda = |arg1, arg2[, ...]| {
    print(arg1, arg2)
    printf(my_string)
}

my_lambda('this is ', 'a string')
```

## Exception Handling
An [Exception](../stdlib/exception.md) may be thrown (well, any type may be thrown, but traditionally, it is exceptions that are) via the `throw` keyword.  When an exception is thrown, the interpreter will set a field on it called `stacktrace` which contains the string representation of the stacktrace (separated by newline characters).  If the exception being thrown already contains a field called `stacktrace`, it will be overwritten.

Exceptions may be caught if they are thrown from within a `try` block.  Exception handlers may also have an optional `finally` block to be executed irrespective of whether an exception is thrown or handled.

```
function my_function()
{
    throw Exception('My exception message')
}

try
{
    my_function()
}
catch (Exception as exception)
{
    print(exception.message())
    print(exception.stacktrace)
}
finally
{
    print('inside the finally block')
}
```

It is only possible to catch a single type of exception and it's only possible to have a single catch block at this stage.

If you have caught an exception and wish to re-throw it, but maintain the previous stacktrace stored, then you should use the `rethrow` keyword.  It is possible to use this outside of an exception handler, but it doesn't make a lot of sense anywhere else.  Rethrowing an exception that hasn't previously been thrown will print a warning instead of a stacktrace.

## Imports

Currently, imports are cheap-and-cheerful, and are planned to be changed in the future.  It is possible to import a file through the `import` keyword, followed by a string containing the path to a file, but without its extension. This can be relative or absolute.  e.g. `import '../path/to/file'`
I say that imports are cheap and cheerful, because all files share a single global scope, meaning that globals are truly global and name collisions are a real possibility.

## Native Enhancements

It's totally possible to implement classes as native libraries (see the stdlib).  It's not possible to do exception handling in native code, however.  You can even make calls into the non-native code, so long as you have access to it.
