#include <stdio.h>
#include <sys/types.h>

#ifndef _DEFAULT_CHILD_LIST_SIZE
#define _DEFAULT_CHILD_LIST_SIZE 16
#endif

#ifndef _HAVE_CHILD_HANDLER_DEFS
#define _HAVE_CHILD_HANDLER_DEFS 0

typedef struct child_t {
    char *name;
    pid_t pid;
} child_t;

void register_child(char *, pid_t);
void unregister_child_by_name(char *);
void unregister_child_by_pid(pid_t);
void print_all_children(FILE *);
#endif

#ifndef _HAVE_CHILD_DEFS
#define _HAVE_CHILD_DEFS 0

pid_t make_child_exec(char*, FILE *, FILE *, size_t, char **);
int wait_child(pid_t);
void set_child_signal_out(FILE *);
void child_signal_exit();
#endif
