#ifndef _COMET_LIB_H_
#define _COMET_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "comet.h"

#define MILLI_SECONDS_PER_SECOND 1000
#define NANO_SECONDS_PER_SECOND 1000000000

    void init_object(VM* vm, VALUE klass);
    void init_string(VM* vm, VALUE obj_klass);
    void init_exception(VM* vm);
    void init_file(VM* vm);
    void init_datetime(VM* vm);
    void init_duration(VM *vm);
    void init_functions(VM* vm);
    void init_hash(VM* vm);
    void init_image(VM* vm);
    void init_list(VM* vm);
    void init_module(VM* vm);
    void init_nil(VM* vm);
    void init_boolean(VM* vm);
    void init_socket(VM* vm);
    void init_thread(VM* vm);
    void init_thread_sync(VM* vm);
    void init_enum(VM* vm);
    void init_set(VM* vm);
    void init_env(VM* vm);
    void init_directory(VM* vm);
    void bootstrap_number(VM* vm);
    void complete_number(VM* vm);
    void bootstrap_iterable(VM* vm);
    void complete_iterable(VM* vm);
    void init_string_builder(VM *vm);
    void init_function(VM *vm);

    VALUE callable_p(VM* vm, int arg_count, VALUE* args);

#ifdef WIN32
    void socket_cleanup(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
