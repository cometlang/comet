#include <stdlib.h>
#include "common.h"
#include "mem.h"
#include "compiler.h"

#if DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2
#define MINIMUM_GC_MARK 8192

size_t _bytes_allocated = 0;
size_t _next_GC = 1024 * 1024;

static void collectGarbage(VM *vm);

static VM **threads;
static size_t num_threads = 0;
static size_t thread_capacity = 0;

void register_thread(VM *vm)
{
    if (thread_capacity <= num_threads)
    {
        size_t new_capacity = GROW_CAPACITY(thread_capacity);
        threads = GROW_ARRAY(threads, VM*, thread_capacity, new_capacity);
        thread_capacity = new_capacity;
    }
    threads[num_threads++] = vm;
}

void deregister_thread(VM *vm)
{
    size_t index = 0;
    for (; index < num_threads; index++)
    {
        if (threads[index] == vm)
        {
            threads[index] = NULL;
        }
    }
    for (; index < (num_threads - 1); index++)
    {
        threads[index] = threads[index + 1];
    }
    num_threads--;
}

void *reallocate(void *previous, size_t oldSize, size_t newSize)
{
    _bytes_allocated += newSize - oldSize;
    if (newSize > oldSize && newSize > MINIMUM_GC_MARK)
    {
        for (size_t i = 0; i < num_threads; i++)
        {
            collectGarbage(threads[i]);
        }
    }
#if DEBUG_STRESS_GC
    if (_bytes_allocated > _next_GC)
    {
        for (int i = 0; i < num_threads; i++)
        {
            collectGarbage(threads[i]);
        }
    }
#endif
    if (newSize == 0)
    {
        free(previous);
        return NULL;
    }

    return realloc(previous, newSize);
}

void markObject(Obj *object)
{
    if (object == NULL)
        return;
    if (object->isMarked)
        return;

#if DEBUG_LOG_GC
    printf("%p mark ", (void *)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    if (vm.grayCapacity < vm.grayCount + 1)
    {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = realloc(vm.grayStack,
                               sizeof(Obj *) * vm.grayCapacity);
    }

    vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value)
{
    if (!IS_OBJ(value))
        return;
    markObject(AS_OBJ(value));
}

static void markArray(ValueArray *array)
{
    for (int i = 0; i < array->count; i++)
    {
        markValue(array->values[i]);
    }
}

static void blackenObject(Obj *object)
{
#if DEBUG_LOG_GC
    printf("%p blacken ", (void *)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif
    switch (object->type)
    {
    case OBJ_BOUND_METHOD:
    {
        ObjBoundMethod *bound = (ObjBoundMethod *)object;
        markValue(bound->receiver);
        markObject((Obj *)bound->method);
        break;
    }
    case OBJ_CLASS:
    {
        ObjClass *klass = (ObjClass *)object;
        markObject((Obj *)klass->name);
        markTable(&klass->methods);
        markTable(&klass->staticMethods);
        break;
    }
    case OBJ_NATIVE_CLASS:
    {
        ObjNativeClass *klass = (ObjNativeClass *)object;
        markObject((Obj *)klass->klass.name);
        markTable(&klass->klass.methods);
        markTable(&klass->klass.staticMethods);
        break;
    }
    case OBJ_NATIVE_METHOD:
    {
        ObjNativeMethod *method = (ObjNativeMethod *)object;
        markValue(method->receiver);
        break;
    }
    case OBJ_CLOSURE:
    {
        ObjClosure *closure = (ObjClosure *)object;
        markObject((Obj *)closure->function);
        for (int i = 0; i < closure->upvalueCount; i++)
        {
            markObject((Obj *)closure->upvalues[i]);
        }
        break;
    }
    case OBJ_FUNCTION:
    {
        ObjFunction *function = (ObjFunction *)object;
        markObject((Obj *)function->name);
        markArray(&function->chunk.constants);
        break;
    }
    case OBJ_INSTANCE:
    {
        ObjInstance *instance = (ObjInstance *)object;
        markObject((Obj *)instance->klass);
        markTable(&instance->fields);
        break;
    }
    case OBJ_NATIVE_INSTANCE:
    {
        ObjNativeInstance *instance = (ObjNativeInstance *)object;
        markObject((Obj *)instance->instance.klass);
        markTable(&instance->instance.fields);
        break;
    }
    case OBJ_UPVALUE:
        markValue(((ObjUpvalue *)object)->closed);
        break;
    case OBJ_NATIVE:
        break;
    }
}

static void freeObject(Obj *object)
{
#if DEBUG_LOG_GC
    printf("%p free %s\n", (void *)object, objTypeName(object->type));
#endif
    switch (object->type)
    {
    case OBJ_BOUND_METHOD:
        FREE(ObjBoundMethod, object);
        break;
    case OBJ_CLASS:
    {
        ObjClass *klass = (ObjClass *)object;
        freeTable(&klass->methods);
        freeTable(&klass->staticMethods);
        FREE(ObjClass, object);
        break;
    }
    case OBJ_NATIVE_CLASS:
    {
        ObjNativeClass *klass = (ObjNativeClass *)object;
        freeTable(&klass->klass.methods);
        freeTable(&klass->klass.staticMethods);
        FREE(ObjNativeClass, object);
        break;
    }
    case OBJ_NATIVE_METHOD:
    {
        FREE(ObjNativeMethod, object);
        break;
    }
    case OBJ_CLOSURE:
    {
        ObjClosure *closure = (ObjClosure *)object;
        FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
        FREE(ObjClosure, object);
        break;
    }
    case OBJ_FUNCTION:
    {
        ObjFunction *function = (ObjFunction *)object;
        freeChunk(&function->chunk);
        FREE(ObjFunction, object);
        break;
    }
    case OBJ_INSTANCE:
    {
        ObjInstance *instance = (ObjInstance *)object;
        freeTable(&instance->fields);
        FREE(ObjInstance, object);
        break;
    }
    case OBJ_NATIVE_INSTANCE:
    {
        ObjNativeInstance *instance = (ObjNativeInstance *)object;
        ObjNativeClass *klass = (ObjNativeClass *)instance->instance.klass;
        if (klass->destructor != NULL)
        {
            klass->destructor(instance->data);
        }
        freeTable(&instance->instance.fields);
        FREE(ObjNativeInstance, object);
        break;
    }
    case OBJ_NATIVE:
        FREE(ObjNative, object);
        break;
    case OBJ_UPVALUE:
        FREE(ObjUpvalue, object);
        break;
    }
}

static void markRoots(VM *vm)
{
    for (Value *slot = vm->stack; slot < vm->stackTop; slot++)
    {
        markValue(*slot);
    }

    for (int i = 0; i < vm->frameCount; i++)
    {
        markObject((Obj *)vm->frames[i].closure);
    }

    for (ObjUpvalue *upvalue = vm->openUpvalues;
         upvalue != NULL;
         upvalue = upvalue->next)
    {
        markObject((Obj *)upvalue);
    }

    markGlobals();
    markCompilerRoots();
}

static void traceReferences(VM *vm)
{
    while (vm->grayCount > 0)
    {
        Obj *object = vm->grayStack[--vm->grayCount];
        blackenObject(object);
    }
}

static void sweep(VM *vm)
{
    Obj *previous = NULL;
    Obj *object = vm->objects;
    while (object != NULL)
    {
        if (object->isMarked)
        {
            object->isMarked = false;
            previous = object;
            object = object->next;
        }
        else
        {
            Obj *unreached = object;

            object = object->next;
            if (previous != NULL)
            {
                previous->next = object;
            }
            else
            {
                vm->objects = object;
            }

            freeObject(unreached);
        }
    }
}

static void collectGarbage(VM *vm)
{
#if DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = _bytes_allocated;
#endif

    markRoots(vm);
    traceReferences(vm);
    removeWhiteStrings();
    sweep(vm);

    _next_GC = _bytes_allocated * GC_HEAP_GROW_FACTOR;
#if DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %ld bytes (from %ld to %ld) next at %ld\n",
           before - _bytes_allocated, before, _bytes_allocated,
           _next_GC);
#endif
}

void freeObjects(VM *vm)
{
    Obj *object = vm->objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        freeObject(object);
        object = next;
    }

    free(vm->grayStack);
}
