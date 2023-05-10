#ifndef _COMET_STRING_H_
#define _COMET_STRING_H_

#include "comet.h"
#include "utf8proc.h"

typedef struct
{
    ObjInstance obj;
    size_t length;
    char *chars;
    uint32_t hash;
} StringData;

typedef struct
{
    ObjInstance obj;
    StringData *string;
    utf8proc_int32_t current_codepoint;
    utf8proc_ssize_t remaining;
    utf8proc_ssize_t offset;
} StringIterator;

bool string_iter_get_next(StringIterator *iter);
VALUE string_iterator(VM *vm, VALUE self, int arg_count, VALUE *arguments);

#endif