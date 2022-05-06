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

static constexpr string_view file_extension(".cmt");
static const char *COMET_LIB_ENV_VARNAME = "COMET_LIB_DIR";

static filesystem::path resolve_import_path(const char *relative_to_filename, const char *to_import)
{
    std::string to_import_filename = string(to_import);
    if (!to_import_filename.ends_with(file_extension))
    {
        to_import_filename += string(file_extension);
    }
    filesystem::path import_path = filesystem::path(to_import_filename);
    filesystem::path current_dir;
    if (relative_to_filename == NULL)
    {
        current_dir = filesystem::current_path();
    }
    else
    {
        current_dir = filesystem::path(relative_to_filename).parent_path();
    }
    filesystem::path candidate = current_dir / import_path;
    if (filesystem::exists(candidate))
    {
        return candidate;
    }

    char *dir = std::getenv(COMET_LIB_ENV_VARNAME);
    if (dir != NULL)
    {
        candidate = filesystem::path(std::string(dir)) / import_path;
    }
    else
    {
#ifdef WIN32
        const char *lib_dir = "C:\\comet";
#else
        const char *lib_dir = "/usr/local/lib/comet";
#endif
        candidate = filesystem::path(std::string(lib_dir)) / import_path;
    }

    if (filesystem::exists(candidate))
    {
        return candidate;
    }

    return filesystem::path();
}


Value import_from_file(VM *vm, const char *relative_to_filename, Value to_import)
{
    const char *to_import_path = string_get_cstr(to_import);
    filesystem::path candidate = resolve_import_path(relative_to_filename, to_import_path);
    if (!filesystem::exists(candidate))
    {
        throw_exception_native(vm, "ImportError", "Could not import '%s'", to_import_path);
        return NIL_VAL;
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
            runtimeError(vm, "Compilation failed for %s\n", full_path);
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