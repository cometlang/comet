#include "comet.h"
#include "comet_stdlib.h"

static VALUE set_iterator_class;

typedef struct _set_entry
{
    Value key;
    struct _set_entry *next;
} SetEntry;

typedef struct {
    int count;
    int capacity;
    SetEntry **entries;
} SetData;

typedef struct {
    SetData *set;
    int index;
    int returned_count;
    SetEntry *current;
} SetIterator;

#define SET_MAX_LOAD_FACTOR 0.75

void *set_constructor(void)
{
    SetData *data = ALLOCATE(SetData, 1);
    data->capacity = 0;
    data->count = 0;
    data->entries = NULL;
    return data;
}

void set_destructor(void *data)
{
    SetData *set_data = (SetData *)data;
    if (set_data->entries != NULL)
    {
        FREE_ARRAY(SetEntry, set_data->entries, set_data->capacity);
    }
    FREE(SetData, set_data);
}

static bool compare_objects(VALUE lhs, VALUE rhs)
{
    // try the cheap comparison first (also covers non-instance objects, like types)
    if (lhs == rhs)
        return true;
    if ((IS_NATIVE_INSTANCE(lhs) || IS_INSTANCE(lhs)) &&
        (IS_NATIVE_INSTANCE(rhs) || IS_INSTANCE(rhs)))
    {
        ObjInstance *obj = AS_INSTANCE(lhs);
        VALUE result = call_function(lhs, obj->klass->operators[OPERATOR_EQUALS], 1, &rhs);
        if (result == TRUE_VAL)
            return true;
    }

    return false;
}

static bool insert(SetEntry **entries, int capacity, SetEntry *entry)
{
    VALUE hash = call_function(entry->key, common_strings[STRING_HASH], 0, NULL);
    uint32_t index = ((uint32_t) number_get_value(hash)) % capacity;
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
            if (compare_objects(current->key, entry->key))
                return false;
            previous = current;
            current = current->next;
        }
        previous->next = entry;
    }
    return true;
}

static void adjust_capacity(SetData *data)
{
    int new_capacity = GROW_CAPACITY(data->capacity);
    SetEntry **new_entries = ALLOCATE(SetEntry *, new_capacity);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *current = data->entries[i];
        while (current != NULL)
        {
            insert(new_entries, new_capacity, current);
            current = current->next;
        }
    }
}

VALUE set_add(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    if ((data->count + 1) > (data->capacity * SET_MAX_LOAD_FACTOR))
    {
        adjust_capacity(data);
    }
    SetEntry *new_entry = ALLOCATE(SetEntry, 1);
    new_entry->next = NULL;
    new_entry->key = arguments[0];
    if (insert(data->entries, data->capacity, new_entry))
    {
        data->count++;
        return TRUE_VAL;
    }
    FREE(SetEntry, new_entry);
    return FALSE_VAL;
}

VALUE set_remove(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *current = data->entries[i];
        SetEntry *previous = NULL;
        while (current != NULL)
        {
            if (compare_objects(current->key, arguments[0]))
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

VALUE set_union(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE set_intersect(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
}

VALUE set_difference(VM UNUSED(*vm), VALUE UNUSED(self), int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    return NIL_VAL;
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

VALUE set_iterable_empty_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    if (data->count == 0)
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE set_iterable_iterator(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    ObjNativeInstance *obj = (ObjNativeInstance *) newInstance(vm, AS_CLASS(set_iterator_class));
    SetIterator *iter = (SetIterator *)obj->data;
    iter->set = GET_NATIVE_INSTANCE_DATA(SetData, self);
    return OBJ_VAL(obj);
}

VALUE set_iterable_contains_q(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    SetData *data = GET_NATIVE_INSTANCE_DATA(SetData, self);
    for (int i = 0; i < data->capacity; i++)
    {
        SetEntry *current = data->entries[i];
        while (current != NULL)
        {
            if (compare_objects(current->key, arguments[0]))
                return TRUE_VAL;
            current = current->next;
        }
    }
    return FALSE_VAL;
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

void *set_iterator_constructor(void)
{
    SetIterator *iter = ALLOCATE(SetIterator, 1);
    iter->current = NULL;
    iter->index = -1;
    iter->set = NULL;
    iter->returned_count = 0;

    return iter;
}

void set_iterator_destructor(void *data)
{
    FREE(SetIterator, data);
}

VALUE set_iterator_get_next(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetIterator *iter = GET_NATIVE_INSTANCE_DATA(SetIterator, self);
    while (iter->current == NULL)
    {
        iter->index++;
        iter->current = iter->set->entries[iter->index];
    }
    VALUE result = iter->current->key;
    iter->current = iter->current->next;
    iter->returned_count++;
    return result;
}

VALUE set_iterator_has_next_p(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    SetIterator *iter = GET_NATIVE_INSTANCE_DATA(SetIterator, self);
    if (iter->returned_count < iter->set->count)
        return TRUE_VAL;
    return FALSE_VAL;
}

void init_set(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Set", &set_constructor, &set_destructor, "Iterable", CLS_SET);
    defineNativeMethod(vm, klass, &set_add, "add", 1, false);
    defineNativeMethod(vm, klass, &set_remove, "remove", 1, false);
    defineNativeMethod(vm, klass, &set_union, "union", 1, false);
    defineNativeMethod(vm, klass, &set_intersect, "intersect", 1, false);
    defineNativeMethod(vm, klass, &set_difference, "difference", 1, false);
    defineNativeMethod(vm, klass, &set_to_list, "to_list", 0, false);
    defineNativeMethod(vm, klass, &set_iterable_empty_p, "empty?", 0, false);
    defineNativeMethod(vm, klass, &set_iterable_count, "count", 0, false);

    set_iterator_class = defineNativeClass(
        vm, "SetIterator", &set_iterator_constructor, &set_iterator_destructor, "Iterator", CLS_ITERATOR);
    defineNativeMethod(vm, set_iterator_class, &set_iterator_has_next_p, "has_next?", 0, false);
    defineNativeMethod(vm, set_iterator_class, &set_iterator_get_next, "get_next", 0, false);
}
