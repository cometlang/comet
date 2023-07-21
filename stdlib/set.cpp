#include <string>
#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif

#include "comet.h"
#include "comet_stdlib.h"

static VALUE set_class;
static VALUE set_iterator_class;

typedef struct _set_entry
{
    Value key;
    struct _set_entry *next;
} SetEntry;

typedef struct {
    ObjInstance obj;
    int count;
    int capacity;
    SetEntry **entries;
} SetData;

typedef struct {
    ObjInstance obj;
    VALUE set;
    int index;
    int returned_count;
    SetEntry *current;
} SetIterator;

#define SET_MAX_LOAD_FACTOR 0.75

void set_constructor(void *instanceData)
{
    SetData *data = (SetData *)instanceData;
    data->capacity = 0;
    data->count = 0;
    data->entries = NULL;
}

void set_destructor(void *data)
{
    SetData *set_data = (SetData *)data;
    for (int i = 0; i < set_data->capacity; i++)
    {
        SetEntry *next = NULL;
        if (set_data->entries[i] != NULL)
        {
            next = set_data->entries[i]->next;
            FREE(SetEntry *, set_data->entries[i]);
        }

        while (next != NULL)
        {
            SetEntry *current = next;
            next = current->next;
            FREE(SetEntry *, current);
        }
    }
    if (set_data->entries != NULL)
    {
        FREE_ARRAY(SetEntry, set_data->entries, set_data->capacity);
    }
    set_data->capacity = 0;
    set_data->count = 0;
    set_data->entries = NULL;
}

VALUE set_create(VM *vm)
{
    VALUE set = OBJ_VAL(newInstance(vm, AS_CLASS(set_class)));
    push(vm, set);
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, set);
    data->capacity = 0;
    data->count = 0;
    data->entries = NULL;

    return pop(vm);
}

static uint32_t getIndex(VM *vm, Value value, int capacity)
{
    call_function(vm, value, common_strings[STRING_HASH], 0, NULL);
    uint32_t result = ((uint32_t) number_get_value(peek(vm, 0))) % capacity;
    pop(vm);
    return result;
}

static bool insert(VM *vm, SetEntry **entries, int capacity, SetEntry *entry)
{
    uint32_t index = getIndex(vm, entry->key, capacity);
    if (entries[index] == NULL)
    {
        entries[index] = entry;
        return true;
    }
    else
    {
        SetEntry *current = entries[index];
        SetEntry *previous = NULL;
        while (current != NULL)
        {
            if (compare_objects(vm, current->key, entry->key))
                return false;
            previous = current;
            current = current->next;
        }
        previous->next = entry;
    }
    return true;
}

static void adjust_capacity(VM *vm, SetData *data)
{
    int new_capacity = GROW_CAPACITY(data->capacity);
    SetEntry **new_entries = ALLOCATE(SetEntry *, new_capacity);
    memset(new_entries, 0, new_capacity * sizeof(SetEntry *));
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *current = data->entries[i];
        while (current != NULL)
        {
            insert(vm, new_entries, new_capacity, current);
            current = current->next;
        }
    }
    if (data->entries != NULL)
    {
        FREE_ARRAY(SetEntry *, data->entries, data->capacity);
    }
    data->capacity = new_capacity;
    data->entries = new_entries;
}

VALUE set_add(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    if ((data->count + 1) > (data->capacity * SET_MAX_LOAD_FACTOR))
    {
        adjust_capacity(vm, data);
    }
    SetEntry *new_entry = ALLOCATE(SetEntry, 1);
    new_entry->next = NULL;
    new_entry->key = arguments[0];
    if (insert(vm, data->entries, data->capacity, new_entry))
    {
        data->count++;
        return TRUE_VAL;
    }
    else
    {
        FREE(SetEntry, new_entry);
    }
    return FALSE_VAL;
}

VALUE set_remove(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *current = data->entries[i];
        SetEntry *previous = NULL;
        while (current != NULL)
        {
            if (compare_objects(vm, current->key, arguments[0]))
            {
                VALUE result = current->key;
                push(vm, result);
                if (previous == NULL)
                {
                    data->entries[i] = current->next;
                }
                else
                {
                    previous->next = current->next;
                }
                FREE(SetEntry, current);
                data->count--;
                return pop(vm);
            }
            previous = current;
            current = current->next;
        }
    }
    return NIL_VAL;
}

VALUE set_iterable_contains_q(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    uint32_t index = getIndex(vm, arguments[0], data->capacity);
    SetEntry *current = data->entries[index];
    while (current != NULL)
    {
        if (compare_objects(vm, current->key, arguments[0]))
            return TRUE_VAL;
        current = current->next;
    }
    return FALSE_VAL;
}

VALUE set_union(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    VALUE result = set_create(vm);
    push(vm, result);
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *entry = data->entries[i];
        while (entry != NULL)
        {
            set_add(vm, result, 1, &entry->key);
            entry = entry->next;
        }
    }
    SetData *other = GET_NATIVE_INSTANCE_DATA(SetData, arguments[0]);
    for (int i = 0; i < other->capacity; i++)
    {
        SetEntry *entry = other->entries[i];
        while (entry != NULL)
        {
            set_add(vm, result, 1, &entry->key);
            entry = entry->next;
        }
    }
    return pop(vm);
}

VALUE set_intersect(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    VALUE result = set_create(vm);
    push(vm, result);
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, arguments[0]);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *entry = data->entries[i];
        while (entry != NULL)
        {
            if (set_iterable_contains_q(vm, self, 1, &entry->key) == TRUE_VAL)
                set_add(vm, result, 1, &entry->key);
            entry = entry->next;
        }
    }
    return pop(vm);
}

VALUE set_difference(VM *vm, VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    VALUE result = set_create(vm);
    push(vm, result);
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *entry = data->entries[i];
        while (entry != NULL)
        {
            if (set_iterable_contains_q(vm, arguments[0], 1, &entry->key) == FALSE_VAL)
                set_add(vm, result, 1, &entry->key);
            entry = entry->next;
        }
    }
    return pop(vm);
}

VALUE set_to_list(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    VALUE list = list_create(vm);
    push(vm, list);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *entry = data->entries[i];
        while (entry != NULL)
        {
            list_add(vm, list, 1, &entry->key);
            entry = entry->next;
        }
    }
    return pop(vm);
}

VALUE set_obj_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    std::stringstream stream;
    stream << "{";
    int found = 0;
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *entry = data->entries[i];
        while (entry != NULL)
        {
            call_function(vm, data->entries[i]->key, common_strings[STRING_TO_STRING], 0, NULL);
            stream << string_get_cstr(peek(vm, 0));
            if (found != data->count - 1)
                stream << ", ";
            pop(vm);
            entry = entry->next;
            found++;
        }
    }
    stream << "}";
    std::string result = stream.str();
    return copyString(vm, result.c_str(), result.length());
}

VALUE set_iterable_empty_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    if (data->count == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE set_iterable_iterator(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    VALUE obj = OBJ_VAL(newInstance(vm, AS_CLASS(set_iterator_class)));
    SetIterator *iter = GET_NATIVE_INSTANCE_DATA(SetIterator, obj);
    iter->set = self;
    return OBJ_VAL(obj);
}

void set_mark_contents(VALUE self)
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *entry = data->entries[i];
        while (entry != NULL)
        {
            markValue(entry->key);
            entry = entry->next;
        }
    }
}

VALUE set_iterable_count(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    return create_number(vm, data->count);
}

void set_iterator_constructor(void *instanceData)
{
    SetIterator *iter = (SetIterator *)instanceData;
    iter->current = NULL;
    iter->index = -1;
    iter->set = NIL_VAL;
    iter->returned_count = 0;
}

VALUE set_iterator_get_next(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetIterator *iter = GET_NATIVE_INSTANCE_DATA(SetIterator, self);
    SetData *set_data = GET_NATIVE_INSTANCE_DATA(SetData, iter->set);
    while (iter->current == NULL)
    {
        iter->index++;
        iter->current = set_data->entries[iter->index];
    }
    VALUE result = iter->current->key;
    iter->current = iter->current->next;
    iter->returned_count++;
    return result;
}

VALUE set_iterator_has_next_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetIterator *iter = GET_NATIVE_INSTANCE_DATA(SetIterator, self);
    SetData *set_data = GET_NATIVE_INSTANCE_DATA(SetData, iter->set);
    if (iter->returned_count < set_data->count)
        return TRUE_VAL;
    return FALSE_VAL;
}

void mark_set_iterator(VALUE self)
{
    SetIterator *iter = GET_NATIVE_INSTANCE_DATA(SetIterator, self);
    markValue(iter->set);
}

void init_set(VM *vm)
{
    set_class = defineNativeClass(
        vm,
        "Set",
        &set_constructor,
        &set_destructor,
        &set_mark_contents,
        "Iterable",
        CLS_SET,
        sizeof(SetData),
        true);
    defineNativeMethod(vm, set_class, &set_add, "add", 1, false);
    defineNativeMethod(vm, set_class, &set_remove, "remove", 1, false);
    defineNativeMethod(vm, set_class, &set_union, "union", 1, false);
    defineNativeMethod(vm, set_class, &set_intersect, "intersect", 1, false);
    defineNativeMethod(vm, set_class, &set_difference, "difference", 1, false);
    defineNativeMethod(vm, set_class, &set_to_list, "to_list", 0, false);
    defineNativeMethod(vm, set_class, &set_obj_to_string, "to_string", 0, false);
    defineNativeMethod(vm, set_class, &set_iterable_empty_p, "empty?", 0, false);
    defineNativeMethod(vm, set_class, &set_iterable_count, "count", 0, false);

    defineNativeOperator(vm, set_class, &set_union, 1, OPERATOR_BITWISE_OR);
    defineNativeOperator(vm, set_class, &set_intersect, 1, OPERATOR_BITWISE_AND);
    defineNativeOperator(vm, set_class, &set_difference, 1, OPERATOR_MINUS);

    set_iterator_class = defineNativeClass(
        vm,
        "SetIterator",
        &set_iterator_constructor,
        NULL,
        &mark_set_iterator,
        "Iterator",
        CLS_ITERATOR,
        sizeof(SetIterator),
        true);
    defineNativeMethod(vm, set_iterator_class, &set_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, set_iterator_class, &set_iterator_get_next, "get_next", 0, false);
}

#ifdef __cplusplus
}
#endif
