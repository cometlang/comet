#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <libgen.h>
#endif

#include "import.h"
#include "common.h"
#include "compiler.h"
#include "comet.h"

#define EXTENSION_MAX_STRLEN 4

#ifdef WIN32
static char* dirname(char* filename)
{
    return filename;
}
#endif

ObjFunction *import_from_file(VM *vm, const char *filename, Value import_path)
{
    int filename_len = strlen(filename);
    const char *path = string_get_cstr(import_path);
    int path_len = strlen(path);
    int length = path_len + filename_len + EXTENSION_MAX_STRLEN + 1;
    char *candidate = malloc(sizeof(char) * length);
    if (candidate == NULL)
    {
        runtimeError(vm, "Out of memory");
        return NULL;
    }
    char *dir = dirname((char *)filename);
    int dir_len = strlen(dir);
    memcpy(candidate, dir, dir_len);
    candidate[dir_len] = '/';
    memcpy(&candidate[dir_len + 1], &path[1], path_len - 2);
    memcpy(&candidate[dir_len + 1 + path_len - 2], ".cmt", 4);
    candidate[dir_len + 1 + path_len - 2 + 4] = '\0';
    SourceFile *source = readSourceFile(candidate);
    
    ObjFunction *function = compile(source, vm);
    if (function == NULL)
    {
        runtimeError(vm, "Failed to import %s\n", candidate);
        free(candidate);
        return NULL;
    }

    free(candidate);
    return function;
}
