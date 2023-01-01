#ifndef _COMET_STDLIB_PRIVATE_LIST_H_
#define _COMET_STDLIB_PRIVATE_LIST_H_
#include "cometlib.h"

VALUE list_iterator_has_next_p(VM *vm, VALUE self, int arg_count, VALUE *arguments);
VALUE list_iterator_get_next(VM *vm, VALUE self, int arg_count, VALUE *arguments);
VALUE list_iterable_iterator(VM *vm, VALUE self, int arg_count, VALUE *arguments);

#endif
