#include <stdio.h>

#ifndef _DEFAULT_BUILTIN_LIST_SIZE
#define _DEFAULT_BUILTIN_LIST_SIZE 16
#endif

#ifndef _HAVE_BUILTIN_HANDLER_DEFS
#define _HAVE_BUILTIN_HANDLER_DEFS 0

typedef int (builtin_func_t) (FILE *, FILE *, size_t, char **);

typedef struct builtin_t {
    char *name;
    builtin_func_t *func;
} builtin_t;

void register_builtin(char *, builtin_func_t *);
int call_builtin(char *, FILE *, FILE *, size_t, char **);
#endif

#ifndef _HAVE_BUILTIN_COMMAND_DEFS
#define _HAVE_BUILTIN_COMMAND_DEFS 0

int cd(FILE *, FILE *, size_t, char **);
int exits(FILE *, FILE *, size_t, char **);
int pwd(FILE *, FILE *, size_t, char **);
int pid(FILE *, FILE *, size_t, char **);
int ppid(FILE *, FILE *, size_t, char **);
int jobs(FILE *, FILE *, size_t, char **);
#endif
