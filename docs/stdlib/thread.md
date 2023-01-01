[up](index.md)

WARNING:
Threads are known to not work particularly well.  They cause problems with the Garbage Collection, so
random segfaults that don't occur with non-threaded code, is likely because of threads.  One day I might
re-write the GC to be reference-counted, so this issue will go away.

## Thread
inherits [Object](object.md)
final

This is a real Thread with all the problems and performance that come with that.  See also [Thread Synchronisation](thread_synchronisation.md)

### methods
- `start(callable, arg)` starts a new OS thread by calling the callable with the argument
- `join()` waits until the thread finishes and collects the thread
