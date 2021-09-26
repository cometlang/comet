#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <libgen.h>
#endif

#include <string>
#include <string_view>
#include <filesystem>

#include <iostream>

extern "C" {

#include "import.h"
#include "comet.h"
#include "common.h"
#include "compiler.h"

using namespace std;

constexpr string_view file_extenstion(".cmt");

#define EXTENSION_MAX_STRLEN 4

Value import_from_file(VM *vm, const char *relative_to_filename, Value to_import)
{
    const char *to_import_path = string_get_cstr(to_import);
    string current_dir;
    if (relative_to_filename == NULL)
    {
        current_dir = filesystem::current_path().string();
    }
    else
    {
        current_dir = filesystem::path(relative_to_filename).parent_path().string();
    }
    filesystem::path candidate = filesystem::path(current_dir) / filesystem::path(string(to_import_path));
    if (!filesystem::exists(candidate))
    {
        candidate += string(file_extenstion);
    }
    string absolute_path = filesystem::canonical(candidate).string();

    const char *full_path = absolute_path.c_str();
    Value full_path_val = copyString(vm, full_path, strlen(full_path));
    push(vm, full_path_val);

    Value module;
    if (!findModule(peek(vm, 0), &module))
    {
        SourceFile *source = readSourceFile(full_path);
        module = compile(source, vm);
        if (module == NIL_VAL)
        {
            runtimeError(vm, "Failed to import %s\n", full_path);
        }
        else
        {
            addModule(module, copyString(vm, full_path, strlen(full_path)));
        }
    }
    pop(vm);

    return module;
}


}