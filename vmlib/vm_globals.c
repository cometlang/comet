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

bool findModuleVariable(ObjModule *module, Value name, Value *value)
{
    return tableGet(&module->variables, name, value);
}

bool addModuleVariable(ObjModule *module, Value name, Value value)
{
    return tableSet(&module->variables, name, value);
}

void addModule(ObjModule *module, Value filename)
{
    tableSet(&modules, filename, OBJ_VAL(module));
}

bool findModule(Value filename, ObjModule **module)
{
    Value result;
    if (tableGet(&modules, filename, &result))
    {
        *module = AS_MODULE(result);
        return true;
    }
    return false;
}
