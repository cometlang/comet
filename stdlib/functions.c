#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "cometlib.h"
#include "comet.h"
#include "comet_stdlib.h"

VALUE std_streams;
#define STD_STREAM_OUT 0
#define STD_STREAM_ERR 1

static Value clockNative(VM *vm, int UNUSED(argCount), Value UNUSED(*args))
{
    return create_number(vm, (double)clock() / CLOCKS_PER_SEC);
}

static VALUE printNative(VM *vm, int arg_count, VALUE *args)
{
    for (int i = 0; i < arg_count; i++)
    {
        call_function(vm, args[i], common_strings[STRING_TO_STRING], 0, NULL);
        printf("%s", string_get_cstr(peek(vm, 0)));
        pop(vm);
    }
    printf("\n");
    fflush(stdout);
    return NIL_VAL;
}

static VALUE print_to(VM *vm, int arg_count, VALUE *args)
{
    uint64_t stream_val = enumvalue_get_value(args[0]);
    FILE *stream = stdout;
    if (stream_val == STD_STREAM_ERR)
        stream = stderr;
    for (int i = 1; i < arg_count; i++)
    {
        call_function(vm, args[i], common_strings[STRING_TO_STRING], 0, NULL);
        fprintf(stream, "%s", string_get_cstr(peek(vm, 0)));
        pop(vm);
    }
    fprintf(stream, "\n");
    fflush(stream);
    return NIL_VAL;
}

static VALUE assertNative(VM *vm, int UNUSED(arg_count), VALUE *args)
{
    if (bool_is_falsey(args[0]))
    {
        const char *message = "assert failed";
        if (arg_count == 2)
            message = string_get_cstr(args[1]);
        throw_exception_native(vm, "AssertionException", message);
    }
    return NIL_VAL;
}

VALUE callable_p(VM UNUSED(*vm), int UNUSED(arg_count), VALUE *args)
{
    VALUE val = args[0];
    if (IS_BOUND_METHOD(val) || IS_CLASS(val) || IS_FUNCTION(val) || IS_CLOSURE(val) ||
        IS_NATIVE(val) || IS_NATIVE_METHOD(val) || IS_NATIVE_CLASS(val))
        return TRUE_VAL;

    return FALSE_VAL;
}

VALUE fn_sleep(VM UNUSED(*vm), int UNUSED(arg_count), VALUE *args)
{
    double input_time = number_get_value(args[0]);
#ifdef WIN32
    Sleep(input_time * MILLI_SECONDS_PER_SECOND);
#else
    struct timespec sleep_time, remainder;
    sleep_time.tv_sec = floor(input_time);
    sleep_time.tv_nsec = (input_time - sleep_time.tv_sec) * NANO_SECONDS_PER_SECOND;
    nanosleep(&sleep_time, &remainder);
#endif
    return NIL_VAL;
}

VALUE fn_exit(VM UNUSED(*vm), int UNUSED(arg_count), VALUE *args)
{
    // I should probably make more of an attempt to clean up
    // resources (threads?) but for now, we'll leave that to the OS
    int exit_status = (int) number_get_value(args[0]);
    exit(exit_status);
}

// https://stackoverflow.com/a/1455007/4780928
static void set_stdin_echo(bool enable)
{
#ifdef WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    if( !enable )
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode );

#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

#define MAX_INPUT_LINE_SIZE 512
char input_line[MAX_INPUT_LINE_SIZE];

VALUE get_password(VM *vm, int arg_count, VALUE *args)
{
    set_stdin_echo(false);
    if (arg_count == 1)
    {
        call_function(vm, args[0], common_strings[STRING_TO_STRING], 0, NULL);
        printf("%s", string_get_cstr(peek(vm, 0)));
        pop(vm);
    }
    fgets(input_line, MAX_INPUT_LINE_SIZE, stdin);
    set_stdin_echo(true);
    return copyString(vm, input_line, strlen(input_line));
}

VALUE input(VM *vm, int arg_count, VALUE *args)
{
    if (arg_count == 1)
    {
        call_function(vm, args[0], common_strings[STRING_TO_STRING], 0, NULL);
        printf("%s", string_get_cstr(peek(vm, 0)));
        pop(vm);
    }
    fgets(input_line, MAX_INPUT_LINE_SIZE, stdin);
    return copyString(vm, input_line, strlen(input_line));
}

#if DEBUG_TRACE_EXECUTION
VALUE fn_print_stack(VM UNUSED(*vm), int UNUSED(arg_count), VALUE UNUSED(*args))
{
    toggle_stack_printing();
    return NIL_VAL;
}
#endif

void init_functions(VM *vm)
{
    std_streams = enum_create(vm);
    push(vm, std_streams);
    addGlobal(copyString(vm, "STD_STREAM", 10), std_streams);
    enum_add_value(vm, std_streams, "IN", 1);
    enum_add_value(vm, std_streams, "ERR", 2);
    pop(vm);

    defineNativeFunction(vm, "clock", &clockNative);
    defineNativeFunction(vm, "print", &printNative);
    defineNativeFunction(vm, "print_to", &print_to);
    defineNativeFunction(vm, "input", &input);
    defineNativeFunction(vm, "get_password", &get_password);
    defineNativeFunction(vm, "assert", &assertNative);
    defineNativeFunction(vm, "callable?", &callable_p);
    defineNativeFunction(vm, "sleep", &fn_sleep);
    defineNativeFunction(vm, "exit", &fn_exit);
#if DEBUG_TRACE_EXECUTION
    defineNativeFunction(vm, "toggle_stack_printing", &fn_print_stack);
#endif
}
