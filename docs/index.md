# The Comet Programming Language

# [Syntax](syntax/index.md)
# [Stdlib](stdlib/index.md)

Comet was created with the help of [Crafting Interpreters](https://www.craftinginterpreters.com) by Bob Nystrom

Comet is in the C family of languages and is object-oriented.  It liberally borrows from other languages, such as Python, Ruby, and JavaScript to name a few.  I've tried to keep the semantics unsurprising and the novelty-factor low.  So why would you want to use this instead of say, Python, Ruby or JavaScript?

1. It's small. At less than 2MB (including the standard libary in a single executable) it's ideal for an embedded environment
2. It's a self-contained language.  Many scripting languages require you to understand the low-level C programming
techniques in order to use them (like the file modes for fopen, or the socket dance).  I've attempted to abstract
those APIs into a more naturally comet style.
3. It's learned from (some of) the mistakes of some of its inspirational languages
  - Ruby's heavy use of optional syntax often makes it difficult to understand what's happening
  - Python attempts to search large portions of the filesystem when it imports modules
  - JavaScript's type-coercion makes for error-prone code
  - C's (lack of) memory management is one of the major sources of security bugs in the world
  - Python's dogmatic approach to whitespace means that many useful constructs just aren't reasonable to implement
  - C#'s empty `throw;` statement is often forgotten
  - JavaScript's many exceptions to the rule make programming it very error-prone
  - Java's verbose syntax make it very unfriendly to programmers and keyboards alike
  - C++'s compilation rules and templating system make reasoning about programs very challenging

## Drawbacks
- Currently, the math operations are _very_ slow.  Because of how operator overloading is implemented, all math operations incur the cost of an object function call with virtual method resolution.
- The garbage collection is pretty basic, naive and slow.  For every garbage collection, we iterate through every object in the system, which is _much_ more than necessary.  A relatively straight forward improvement would be to implement a generational system.  Also, all memory operations on all threads come to a halt when the GC is running (which essentially means all execution stops).
