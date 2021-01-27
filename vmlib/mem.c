#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "mem.h"
#include "compiler.h"
#include "comet.h"

#if DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2
#define MINIMUM_GC_MARK 8192

size_t _bytes_allocated = 0;
size_t _next_GC = 1024 * 1024;

static void collectGarbage(void);

static VM **threads;
static int num_threads = 0;
static int thread_capacity = 0;

static Obj **grey_stack;
static int grey_capacity = 0;
static int grey_count = 0;

void register_thread(VM *vm)
{
    if (thread_capacity <= num_threads)
    {
        int new_capacity = GROW_CAPACITY(thread_capacity);
        threads = GROW_ARRAY(threads, VM*, thread_capacity, new_capacity);
        thread_capacity = new_capacity;
    }
    threads[num_threads++] = vm;
}

void deregister_thread(VM *vm)
{
    int index = 0;
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
        collectGarbage();
    }
#if DEBUG_STRESS_GC
    if (_bytes_allocated > _next_GC)
    {
        collectGarbage();
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
    printObject(OBJ_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    if (grey_capacity < grey_count + 1)
    {
        grey_capacity = GROW_CAPACITY(grey_capacity);
        grey_stack = realloc(grey_stack, sizeof(Obj *) * grey_capacity);
    }

    grey_stack[grey_count++] = object;
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
    printObject(OBJ_VAL(object));
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
    case OBJ_NATIVE_CLASS:
    {
        ObjClass *klass = (ObjClass *)object;
        markTable(&klass->methods);
        markTable(&klass->staticMethods);
        for (int i = 0; i < NUM_OPERATORS; i++)
        {
            markValue(klass->operators[i]);
        }
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
        markValue(function->name);
        markArray(&function->chunk.constants);
        break;
    }
    case OBJ_INSTANCE:
    case OBJ_NATIVE_INSTANCE:
    {
        ObjInstance *instance = (ObjInstance *)object;
        markObject((Obj *)instance->klass);
        markTable(&instance->fields);
        break;
    }
    case OBJ_UPVALUE:
        markValue(((ObjUpvalue *)object)->closed);
        break;
    case OBJ_NATIVE_METHOD:
    case OBJ_NATIVE:
        break;
    }
}

static void freeObject(Obj *object)
{
#if DEBUG_LOG_GC
    if (IS_NATIVE_INSTANCE(OBJ_VAL(object)) && strcmp(((ObjInstance *)object)->klass->name, "String") == 0)
    {
        printf("%p free String: \"%s\"\n", (void *)object, string_get_cstr(OBJ_VAL(object)));
    }
    else if (IS_INSTANCE(OBJ_VAL(object)) || IS_NATIVE_INSTANCE(OBJ_VAL(object)))
    {
        printf("%p free %s (%s)\n", (void *)object, objTypeName(object->type), ((ObjInstance *)object)->klass->name);
    }
    else
    {
        printf("%p free %s\n", (void *)object, objTypeName(object->type));
    }
#endif
    switch (object->type)
    {
    case OBJ_BOUND_METHOD:
        FREE(ObjBoundMethod, object);
        break;
    case OBJ_CLASS:
    {
        ObjClass *klass = (ObjClass *)object;
        FREE_ARRAY(char, klass->name, strlen(klass->name));
        freeTable(&klass->methods);
        freeTable(&klass->staticMethods);
        FREE(ObjClass, object);
        break;
    }
    case OBJ_NATIVE_CLASS:
    {
        ObjNativeClass *klass = (ObjNativeClass *)object;
        FREE_ARRAY(char, klass->klass.name, strlen(klass->klass.name));
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
}

static void traceReferences()
{
    while (grey_count > 0)
    {
        Obj *object = grey_stack[--grey_count];
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

static void collectGarbage()
{
#if DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = _bytes_allocated;
#endif

    for (int i = 0; i < num_threads; i++)
    {
        markRoots(threads[i]);
    }
    markGlobals();
    markCompilerRoots();
    traceReferences();
    removeWhiteStrings();
    for (int i = 0; i < num_threads; i++)
    {
        sweep(threads[i]);
    }

    _next_GC = _bytes_allocated * GC_HEAP_GROW_FACTOR;
#if DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %ld bytes (from %ld to %ld) next at %ld\n",
           before - _bytes_allocated, before, _bytes_allocated,
           _next_GC);
#endif
}

void incorporateObjects(VM *vm)
{
    VM *main_thread = threads[0];
    Obj *object = vm->objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        object->next = main_thread->objects;
        main_thread->objects = object;
        object = next;
    }
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
}

void finalizeGarbageCollection(void)
{
    FREE_ARRAY(Obj*, grey_stack, grey_capacity);
    grey_stack = NULL;
    grey_count = 0;
    grey_capacity = 0;

    FREE_ARRAY(VM *, threads, thread_capacity);
    threads = NULL;
    thread_capacity = 0;
    num_threads = 0;
}