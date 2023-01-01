#include "comet_stdlib.h"
#include <filesystem>

#include <iostream>

extern "C" {

typedef struct dirData
{
    ObjInstance obj;
} DirectoryData;


VALUE dir_list(VM *vm, VALUE UNUSED(klass), int arg_count, VALUE *arguments)
{
    bool include_dots = false;
    if (arg_count == 2)
    {
        include_dots = arguments[1] == TRUE_VAL;
    }

    const char *path = string_get_cstr(arguments[0]);
    VALUE result = list_create(vm);
    push(vm, result);
    for (const auto & entry : std::filesystem::directory_iterator(path))
    {
        std::string path_str = entry.path().generic_string();
        VALUE path_obj = copyString(vm, path_str.c_str(), path_str.length());
        push(vm, path_obj);
        list_add(vm, result, 1, &path_obj);
        pop(vm);
    }

    pop(vm);
    return result;
}

void init_directory(VM* vm)
{
    VALUE klass = defineNativeClass(vm, "Directory", NULL, NULL, NULL, "Object", CLS_DIRECTORY, sizeof(DirectoryData), false);
    defineNativeMethod(vm, klass, &dir_list, "list", 1, true);
}

}
