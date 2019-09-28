#include <stdlib.h>
#include "memory.h"
#include "util.h"

void *reallocate(void *previous, size_t UNUSED(oldSize), size_t newSize)
{
    if (newSize == 0)
    {
        free(previous);
        return NULL;
    }

    return realloc(previous, newSize);
}
