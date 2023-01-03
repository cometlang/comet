#include <filesystem>
#include <string>
#include "comet_stdlib.h"
#include "directory.hpp"

extern "C" {

typedef struct dirData
{
    ObjInstance obj;
    std::filesystem::path path;
} DirectoryData;

static VALUE klass;

VALUE directory_create(VM *vm, const std::filesystem::path& path)
{
    VALUE dir = OBJ_VAL(newInstance(vm, AS_CLASS(klass)));
    push(vm, dir);
    DirectoryData *data = GET_NATIVE_INSTANCE_DATA(DirectoryData, dir);
    data->path = path;

    return pop(vm);
}

static VALUE dir_init(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE *arguments)
{
    DirectoryData *data = GET_NATIVE_INSTANCE_DATA(DirectoryData, self);
    const char *path = string_get_cstr(arguments[0]);
    data->path = std::filesystem::path(path);
    return NIL_VAL;
}

static VALUE dir_parent(VM *vm, VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    DirectoryData *data = GET_NATIVE_INSTANCE_DATA(DirectoryData, self);
    auto parent = data->path.parent_path();
    return directory_create(vm, parent);
}

static VALUE dir_to_string(VM UNUSED(*vm), VALUE self, int UNUSED(arg_count), VALUE UNUSED(*arguments))
{
    DirectoryData *data = GET_NATIVE_INSTANCE_DATA(DirectoryData, self);
    std::string path = data->path.string();
    return copyString(vm, path.c_str(), path.length());
}

VALUE dir_list(VM *vm, VALUE UNUSED(klass), int arg_count, VALUE *arguments)
{
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

VALUE dir_static_directory_q(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    const char *path = string_get_cstr(arguments[0]);
    if (std::filesystem::is_directory(path))
        return TRUE_VAL;
    return FALSE_VAL;
}

VALUE dir_static_remove(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    const char *path = string_get_cstr(arguments[0]);
    VALUE recursively = FALSE_VAL;
    if (arg_count == 2)
    {
        recursively = arguments[1];
    }
    if (recursively == TRUE_VAL)
    {
        std::filesystem::remove_all(path);
    }
    else
    {
        std::filesystem::remove(path);
    }
    return NIL_VAL;
}

VALUE dir_static_create(VM UNUSED(*vm), VALUE UNUSED(klass), int UNUSED(arg_count), VALUE *arguments)
{
    const char *path = string_get_cstr(arguments[0]);
    std::filesystem::create_directories(path);
    return NIL_VAL;
}

void init_directory(VM* vm)
{
    klass = defineNativeClass(vm, "Directory", NULL, NULL, NULL, "Object", CLS_DIRECTORY, sizeof(DirectoryData), true);
    defineNativeMethod(vm, klass, &dir_init, "init", 1, false);
    defineNativeMethod(vm, klass, &dir_parent, "parent", 0, false);
    defineNativeMethod(vm, klass, &dir_list, "list", 1, true);
    defineNativeMethod(vm, klass, &dir_to_string, "to_string", 0, true);
    defineNativeMethod(vm, klass, &dir_static_directory_q, "directory?", 1, true);
    defineNativeMethod(vm, klass, &dir_static_remove, "remove", 1, true);
    defineNativeMethod(vm, klass, &dir_static_remove, "delete", 1, true);
    defineNativeMethod(vm, klass, &dir_static_create, "create", 1, true);
}

}
