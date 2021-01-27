#include <stdio.h>
#include <stdlib.h>

#include "import.h"
#include "common.h"

#define EXTENSION_MAX_STRLEN 4

void import_path(const char *path, int path_len)
{
    int length = path_len + EXTENSION_MAX_STRLEN + 1;
    char candidate[length];
    snprintf(candidate, length, "%s.cmt", path);
}