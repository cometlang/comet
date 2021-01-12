#ifndef _COMET_LIB_H_
#define _COMET_LIB_H_

#include "comet.h"

void init_object(VALUE klass);
void init_string(VALUE obj_klass);
void init_file(void);
void init_iterable(void);
void init_hash(void);
void init_list(void);
void init_nil(void);
void init_socket(void);
void init_thread(void);
void init_enum(void);
void init_exception(void);
void init_functions(void);

#endif
