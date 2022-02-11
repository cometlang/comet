#ifdef WIN32
#include <Windows.h>
#else
#define _GNU_SOURCE
#include <pthread.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "mem.h"
#include "compiler.h"
#include "comet.h"

#include <stdio.h>
#if DEBUG_LOG_GC
#include "debug.h"
#endif

// 128kB
#define MINIMUM_GC_MARK 131072

size_t _bytes_allocated = 0;
size_t _next_GC = MINIMUM_GC_MARK;
static bool collecting_garbage;

static void collectGarbage(void);

static VM **threads;
static int num_threads = 0;
static int thread_capacity = 0;

static Obj **grey_stack;
static int grey_capacity = 0;
static int grey_count = 0;

static uint32_t gc_count;
static Obj *generation_0;
static Obj *generation_1;
static Obj *generation_2;


#ifdef WIN32
static HANDLE gc_lock;
#define MUTEX_LOCK(mut) WaitForSingleObject(mut, INFINITE)
#define MUTEX_UNLOCK(mut) ReleaseMutex(mut)
#define MUTEX_DESTROY(mut) CloseHandle(mut)
#else
static pthread_mutex_t gc_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#define MUTEX_LOCK(mut) pthread_mutex_lock(&mut)
#define MUTEX_UNLOCK(mut) pthread_mutex_unlock(&mut)
#define MUTEX_DESTROY(mut) pthread_mutex_destroy(&mut)
#endif

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
    MUTEX_LOCK(gc_lock);
    _bytes_allocated += newSize - oldSize;
#if DEBUG_STRESS_GC
    if (newSize > oldSize && newSize > MINIMUM_GC_MARK)
    {
        collectGarbage();
    }
#else
    if ((_bytes_allocated > _next_GC) && !collecting_garbage)
    {
        collectGarbage();
    }
#endif
    if (newSize == 0)
    {
        free(previous);
        MUTEX_UNLOCK(gc_lock);
        return NULL;
    }
    void *result = realloc(previous, newSize);
    MUTEX_UNLOCK(gc_lock);
    return result;
}

Obj *allocateObject(size_t size, ObjType type)
{
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = generation_0;
    generation_0 = object;

#if DEBUG_LOG_GC
    printf("%p allocate %ld for %s\n", (void *)object, size, objTypeName(type));
#endif

    return object;
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

    if (object->type == OBJ_NATIVE_INSTANCE || object->type == OBJ_INSTANCE)
    {
        ObjInstance *instance = (ObjInstance *)object;
        switch (instance->klass->classType)
        {
            case CLS_HASH:
                hash_mark_contents(OBJ_VAL(object));
                break;
            case CLS_LIST:
                list_mark_contents(OBJ_VAL(object));
                break;
            case CLS_SET:
                set_mark_contents(OBJ_VAL(object));
                break;
            case CLS_ENUM:
                enum_mark_contents(OBJ_VAL(object));
                break;
            case CLS_THREAD:
                thread_mark_contents(OBJ_VAL(object));
                break;
            default:
                break;
        }
    }
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
        markValue(function->module);
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
    if (IS_NATIVE_INSTANCE(OBJ_VAL(object)) &&
        AS_INSTANCE(OBJ_VAL(object))->klass->classType == CLS_STRING)
    {
        printf("%p free String: \"%s\"\n", (void *)object, string_get_cstr(OBJ_VAL(object)));
    }
    else if (IS_NATIVE_INSTANCE(OBJ_VAL(object)) &&
             AS_INSTANCE(OBJ_VAL(object))->klass->classType == CLS_NUMBER)
    {
        printf("%p free Number: \"%.17g\"\n", (void *)object, number_get_value(OBJ_VAL(object)));
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
            klass->destructor(instance);
        }
        freeTable(&instance->instance.fields);
        FREE_NATIVE_INSTANCE(object);
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
    for (Value *slot = vm->stack; slot <= vm->stackTop; slot++)
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

static Obj *sweep_object_list(Obj *object, Obj **base)
{
    Obj *previous = NULL;
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
                *base = object;
            }

            freeObject(unreached);
        }
    }
    if (previous != NULL)
    {
        return previous->next != NULL ? previous->next : previous;
    }
    return *base;
}

static void sweep()
{
    if (gc_count != 0)
    {
        if (gc_count % 11 == 0)
        {
            sweep_object_list(generation_2, &generation_2);
        }
        if (gc_count % 7 == 0)
        {
            Obj *end = sweep_object_list(generation_1, &generation_1);
            if (end != NULL)
            {
                end->next = generation_2;
                generation_2 = generation_1;
                generation_1 = NULL;
            }
        }
    }

    Obj *end = sweep_object_list(generation_0, &generation_0);
    if (end != NULL)
    {
        end->next = generation_1;
        generation_1 = generation_0;
        generation_0 = NULL;
    }

    gc_count++;
}

static void collectGarbage()
{
#if DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = _bytes_allocated;
#endif
    collecting_garbage = true;

    for (int i = 0; i < num_threads; i++)
    {
        markRoots(threads[i]);
    }
    markGlobals();
    traceReferences();
    removeWhiteStrings();
    sweep();

    _next_GC = _bytes_allocated * 2;
    collecting_garbage = false;
#if DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %ld bytes (from %ld to %ld) next at %ld\n",
           before - _bytes_allocated, before, _bytes_allocated,
           _next_GC);
#endif
}

void initializeGarbageCollection()
{
#ifdef WIN32
    gc_lock = CreateMutex(NULL, false, NULL);
#endif
    collecting_garbage = false;
    gc_count = 0;
    generation_0 = NULL;
    generation_1 = NULL;
    generation_2 = NULL;
}

static void free_object_list(Obj *object)
{
    while (object != NULL)
    {
        Obj *next = object->next;
        freeObject(object);
        object = next;
    }
}

void freeObjects()
{
    free_object_list(generation_0);
    free_object_list(generation_1);
    free_object_list(generation_2);
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

    MUTEX_DESTROY(gc_lock);
}