#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <sstream>

#ifdef WIN32
#define popen _popen
#define pclose _pclose
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "comet.h"
#include "cometlib.h"

typedef struct {
    ObjInstance obj;
} process_data_t;

typedef struct {
    ObjInstance obj;
} process_run_result_t;

static VALUE result_klass;


VALUE process_static_run(VM *vm, VALUE UNUSED(self), int UNUSED(arg_count), VALUE *arguments)
{
    const char *cmd = string_get_cstr(arguments[0]);
    FILE *cmd_file = popen(cmd, "r");
    if (cmd_file == nullptr)
    {
        throw_exception_native(vm, "Exception", "Could not run a process: %s", strerror(errno));
    }
    VALUE proc_result = OBJ_VAL(newInstance(vm, AS_CLASS(result_klass)));
    push(vm, proc_result);
    std::stringstream stream;
    char buffer[256];
    while (!feof(cmd_file)) {
        int numChars = fread(buffer, 1, 255, cmd_file);
        buffer[numChars] = 0;
        stream << buffer;
    }
    int result = pclose(cmd_file);
    setNativeProperty(vm, proc_result, "status_code", create_number(vm, result));
    std::string output = stream.str();
    setNativeProperty(vm, proc_result, "output", copyString(vm, output.c_str(), output.length()));
    setNativeProperty(vm, proc_result, "command", arguments[0]);
    return pop(vm);
}

void init_process(VM *vm)
{
    VALUE klass = defineNativeClass(vm, "Process", nullptr, nullptr, nullptr, nullptr, CLS_PROCESS, sizeof(process_data_t), true);
    defineNativeMethod(vm, klass, &process_static_run, "run", 1, true);

    result_klass = defineNativeClass(vm, "ProcessRunResult", nullptr, nullptr, nullptr, nullptr, CLS_PROCESS_RUN_RESULT, sizeof(process_run_result_t), true);
}

#ifdef __cplusplus
}
#endif