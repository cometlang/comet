#ifndef comet_common_h
#define comet_common_h

#include <stdbool.h> 
#include <stddef.h>  
#include <stdint.h>  

#define DEBUG_PRINT_CODE (0)
#define DEBUG_TRACE_EXECUTION (1)
#define DEBUG_STRESS_GC (0)
#define DEBUG_LOG_GC (0)
#define DEBUG_LOG_GC_MINIMAL (0)
#define DEBUG_LOG_GC_OBJ_FREES (0)
#define MAX_VAR_COUNT (UINT8_MAX + 1)
#define MAX_HANDLER_FRAMES (UINT8_MAX)
#define DEBUG_ASSERT_ENABLED (1)
#define MAX_ARGS (UINT8_MAX)

#ifndef UNUSED
# ifdef WIN32
#  include <Windows.h>
#  define UNUSED(x) UNREFERENCED_PARAMETER(x)
# else
#  define UNUSED(x) x __attribute__ ((unused))
# endif
#endif

typedef struct {
    char *path;
    char *source;
} SourceFile;


#if DEBUG_ASSERT_ENABLED
#include <assert.h>
# define DEBUG_ASSERT(x) assert(x)
#else
# define DEBUG_ASSERT(x)
#endif

#endif 
