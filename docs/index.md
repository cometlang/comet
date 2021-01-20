# The Comet Programming Language

# [Syntax](syntax/index.md)
# [Stdlib](stdlib/index.md)

Comet is in the C family of languages and is object-oriented.  It liberally borrows from other languages, such as Python, Ruby, and JavaScript to name a few.  I've tried to keep the semantics unsurprising and the novelty-factor low.  So why would you want to use this instead of say, Python, Ruby or JavaScript?

1. It's small. At less than 500KB (including the standard libary in a single executable) it's ideal for an embedded environment
2. It's learned from (some of) the mistakes of some of its inspirational languages
  - Ruby's optional syntax often makes it difficult to understand what's happening
  - Python attempts to search large portions of the filesystem when it imports modules
  - JavaScript's type-coercion makes for error-prone code
