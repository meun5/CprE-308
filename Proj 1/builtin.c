#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>

#include "builtin.h"
#include "child.h"

static size_t builtin_list_size = 0;
static builtin_t **builtins = NULL;
static size_t builtin_alloc_size = _DEFAULT_BUILTIN_LIST_SIZE;

void register_builtin(char *name, builtin_func_t *func) {
    if (builtins == NULL) {
        builtins = malloc(sizeof(builtin_t *) * _DEFAULT_BUILTIN_LIST_SIZE);
    }

    if (builtin_list_size >= (builtin_alloc_size - 1)) {
        builtins = realloc(builtins, sizeof(builtin_t *) * (builtin_alloc_size += _DEFAULT_BUILTIN_LIST_SIZE));
    }

    builtin_t *a = malloc(sizeof(builtin_t));

    a->name = name;
    a->func = func;

    builtins[builtin_list_size++] = a;
}

int call_builtin(char *name, FILE *out, FILE *err, size_t argc, char **argv) {
    for (size_t i = 0; i < builtin_list_size; i++) {
        if (strcmp(builtins[i]->name, name) == 0) {
            return builtins[i]->func(out, err, argc, argv);
        }
    }

    return -2;
}

int cd(FILE *_, FILE *err, size_t argc, char **argv) {
    char *dir = NULL;
    if (argc >= 2) { // This implementation of cd ignores extra arguments, which does not follow the implmentation that (b)ash uses.
        dir = argv[1];
    } else {
        dir = getenv("HOME");

        if (dir == NULL) {
            dir = ".";
        }
    }

    if (chdir(dir) != 0) {
        fprintf(err, "cd: %s\n", strerror(errno));
        return 1;
    }

    return 0;
}

int exits(FILE *_, FILE *err, size_t argc, char **argv) {
    if (argc >= 2 && isdigit(argv[1][0])) {
        exit(argv[1][0] - '0');
    }

    exit(EXIT_SUCCESS);

    fprintf(err, "You should not have gotten to this point. The call to exit failed. This is very spooky.");

    return 1;
}

int pwd(FILE *out, FILE *err, size_t argc, char **argv) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        fprintf(out, "%s\n", cwd);
    } else {
        fprintf(err, "getcwd: %s\n", strerror(errno));
        return 1;
    }

    return 0;
}

int pid(FILE *out, FILE *err, size_t argc, char **argv) {
    fprintf(out, "%d\n", getpid());

    return 0;
}

int ppid(FILE *out, FILE *err, size_t argc, char **argv) {
    fprintf(out, "%d\n", getppid());

    return 0;
}

int jobs(FILE *out, FILE *err, size_t argc, char **argv) {
    print_all_children(out);

    return 0;
}
