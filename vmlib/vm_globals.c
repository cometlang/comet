#include "vm.h"
#include "mem.h"

static Table globals;
static Table strings;
static Table modules;
Value common_strings[NUM_COMMON_STRINGS];

void initGlobals(void)
{
    initTable(&globals);
    initTable(&strings);
    initTable(&modules);
}

void freeGlobals(void)
{
    freeTable(&globals);
    freeTable(&strings);
    freeTable(&modules);
    for (int i = 0; i < NUM_COMMON_STRINGS; i++)
    {
        common_strings[i] = NIL_VAL;
    }
}

void markGlobals(void)
{
    markTable(&globals);
    markTable(&strings);
    markTable(&modules);
    for (int i = 0; i < NUM_COMMON_STRINGS; i++)
    {
        markValue(common_strings[i]);
    }
    markValue(NIL_VAL);
    markValue(TRUE_VAL);
    markValue(FALSE_VAL);
}

void removeWhiteStrings(void)
{
    tableRemoveWhite(&strings);
}

Value findInternedString(const char *chars, uint32_t hash)
{
    return tableFindString(&strings, chars, hash);
}

bool internString(Value string)
{
    return tableSet(&strings, string, NIL_VAL);
}

bool findGlobal(Value name, Value *value)
{
    return tableGet(&globals, name, value);
}

bool addGlobal(Value name, Value value)
{
    return tableSet(&globals, name, value);
}

bool findModuleVariable(Value module, Value name, Value *value)
{
    ObjInstance *instance = AS_INSTANCE(module);
    return tableGet(&instance->fields, name, value);
}

bool addModuleVariable(Value module, Value name, Value value)
{
    ObjInstance *instance = AS_INSTANCE(module);
    return tableSet(&instance->fields, name, value);
}

void addModule(Value module, Value filename)
{
    tableSet(&modules, filename, module);
}

bool findModule(Value filename, Value *module)
{
    return tableGet(&modules, filename, module);
}

void getAllModules(VM *vm, Value list)
{
    tableGetValues(&modules, vm, list);
}
