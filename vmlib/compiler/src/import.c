#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "import.h"
#include "common.h"
#include "compiler.h"

#define EXTENSION_MAX_STRLEN 4

void import_path(VM *vm, const char *path, int path_len)
{
    int length = path_len + EXTENSION_MAX_STRLEN + 1;
    char candidate[length];
    memcpy(candidate, &path[1], path_len - 2);
    memcpy(&candidate[path_len - 2], ".cmt", 4);
    candidate[length - 1] = '\0';
    SourceFile *source = readSourceFile(candidate);
    if (interpret(vm, source) != INTERPRET_OK)
        runtimeError(vm, "Failed to import %s\n", candidate);
}