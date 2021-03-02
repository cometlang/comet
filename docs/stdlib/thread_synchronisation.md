[up](index.md)

## Mutex
inherits [Object](object.md)

A mutual exclusion thread synchronisation primitive.  If locked, then only the thread holding the lock may enter the critical section, safeguarding against multiple threads mutating the state simultaneously.

### methods
 - `lock()` block until the lock can be obtained
 - `timed_lock(timeout)` block for (timeout) seconds (may be fractional) and either the lock is obtained or a `TimeoutException` is thrown.
 - `unlock()` release the lock, allowing the next waiting thread to gain the lock.  The thread owning the lock must call unlock.
 - `try_lock()`  Attempt to get the lock or immediately return.  A [Boolean](boolean.md) value will be returned indicating whether the lock was gained successfully.


## ConditionVariable
inherits [Object](object.md)

### methods
 - `wait()` Waits until the condition is signalled (blocking) 
 - `signal_one()` signals a single waiting thread that the condition is met
 - `signal_all()` signals all waiting threads that the condition is met.  The kernel scheduler will determine which thread gains the lock.
 - `timed_wait(timeout)` Wait for a limited amount of fraction seconds.  A `TimeoutException` is thrown if the interval elapses before the condition is signalled.

