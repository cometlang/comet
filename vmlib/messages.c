#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


// Taken directly from the strtol manpage:
char *make_message(const char *fmt, va_list ap)
{
    int n = 0;
    size_t size = 0;
    char *p = NULL;

    /* Determine required size */
    n = vsnprintf(p, size, fmt, ap);

    if (n < 0)
        return NULL;

    /* One extra byte for '\0' */

    size = (size_t) n + 1;
    p = malloc(size);
    if (p == NULL)
        return NULL;

    n = vsnprintf(p, size, fmt, ap);

    if (n < 0) {
        free(p);
        return NULL;
    }

    return p;
}
