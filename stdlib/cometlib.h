#ifndef _COMET_LIB_H_
#define _COMET_LIB_H_

#include "comet.h"

void init_object(VALUE klass);
void init_string(VALUE obj_klass);
void init_exception(VM *vm);
void init_file(VM *vm);
void init_functions(VM *vm);
void init_iterable(VM *vm);
void init_hash(VM *vm);
void init_list(VM *vm);
void init_nil(VM *vm);
void init_string(VM *vm);
void init_socket(VM *vm);
void init_thread(VM *vm);

#endif
