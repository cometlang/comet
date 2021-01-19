# The language itself

## Scope
The scope works like C - lexical scope.  Inner scope can access the scopes out of it, but not vice-versa.  This means that variables are not "hoisted" like they are in Python or (var declared) JavaScript.  It does mean that functions and methods _can_ access and modify global variables without any special syntax.

Variables are resolved from the innermost scope outwards, which means that it is entirely possible to "shadow" outer variables or functions, including built-ins.

## Keywords


## Literals

### Strings
### Numbers
### Lists
### Hashes


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

### Fields

### Operators

### Inheritance


## Native Enhancements