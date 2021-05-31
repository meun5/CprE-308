#ifndef _DEFAULT_BUILTIN_LIST_SIZE
#define _DEFAULT_BUILTIN_LIST_SIZE 16
#endif

#ifndef _HAVE_BUILTIN_HANDLER_DEFS
#define _HAVE_BUILTIN_HANDLER_DEFS 0

#include <stdio.h>
#include <sys/time.h>
#include <stdatomic.h>

typedef int (builtin_func_t) (FILE *, atomic_uint, struct timeval, char **, size_t);

typedef struct builtin_t {
    char *name;
    builtin_func_t *func;
} builtin_t;
#endif

#ifndef _HAVE_BUILTIN_COMMAND_DEFS
#define _HAVE_BUILTIN_COMMAND_DEFS 0
#endif
