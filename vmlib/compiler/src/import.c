#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "import.h"
#include "common.h"
#include "compiler.h"

#define EXTENSION_MAX_STRLEN 4

void import_from_file(VM *vm, const char *filename, const char *path, int path_len)
{
    int filename_len = strlen(filename);
    int length = path_len + filename_len + EXTENSION_MAX_STRLEN + 1;
    char candidate[length];
    char *dir = dirname((char *)filename);
    int dir_len = strlen(dir);
    memcpy(candidate, dir, dir_len);
    candidate[dir_len] = '/';
    memcpy(&candidate[dir_len + 1], &path[1], path_len - 2);
    memcpy(&candidate[dir_len + 1 + path_len - 2], ".cmt", 4);
    candidate[dir_len + 1 + path_len - 2 + 4] = '\0';
    SourceFile *source = readSourceFile(candidate);
    if (interpret(vm, source) != INTERPRET_OK)
        runtimeError(vm, "Failed to import %s\n", candidate);
}