#ifndef clox_memory_h
#define clox_memory_h

#include "objects.h"
#include "vm.h"

#define ALLOCATE(type, count) \
    (type *)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) \
    reallocate(pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity)*2)

#define GROW_ARRAY(previous, type, oldCount, newCount)        \
    (type *)reallocate((previous), sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * oldCount, 0)

void register_thread(VM *vm);
void deregister_thread(VM *vm);
void *reallocate(void *previous, size_t oldSize, size_t newSize);
void markObject(VM *vm, Obj* object);
void markValue(VM *vm, Value value);
void freeObjects(VM *vm);
#endif
