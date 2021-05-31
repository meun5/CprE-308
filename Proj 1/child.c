#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#include "child.h"

static child_t **process_registry = NULL;
static size_t process_registry_list_size = 0;
static size_t process_registry_alloc_size = _DEFAULT_CHILD_LIST_SIZE;
static FILE *signal_out = NULL;

// No crimes against child were commited in the creation of this program.
void register_child(char *name, pid_t pid) {
    if (process_registry == NULL) {
        process_registry = malloc(sizeof(child_t *) * _DEFAULT_CHILD_LIST_SIZE);
    }

    if (process_registry_list_size >= (process_registry_alloc_size - 1)) {
        process_registry = realloc(process_registry, sizeof(child_t *) * (process_registry_alloc_size += _DEFAULT_CHILD_LIST_SIZE));
    }

    child_t *a = malloc(sizeof(child_t));
    a->name = name;
    a->pid = pid;

    process_registry[process_registry_list_size++] = a;
}

void unregister_child_by_name(char * name) {
    if (process_registry_list_size <= 0) {
        return;
    }

    for (size_t i = 0; i < process_registry_list_size; i++) {
        if (strcmp(process_registry[i]->name, name) == 0) {
            free(process_registry[i]->name);
            free(process_registry[i]);

            if (process_registry_list_size > 2) {
                process_registry[i] = process_registry[process_registry_list_size - 1];
                process_registry[process_registry_list_size - 1] = NULL;
            }

            process_registry_list_size--;
        }
    }
}

void unregister_child_by_pid(pid_t pid) {
    if (process_registry_list_size <= 0) {
        return;
    }

    for (size_t i = 0; i < process_registry_list_size; i++) {
        if (process_registry[i]->pid == pid) {
            free(process_registry[i]->name);
            free(process_registry[i]);

            if (process_registry_list_size > 2) {
                process_registry[i] = process_registry[process_registry_list_size - 1];
                process_registry[process_registry_list_size - 1] = NULL;
            }

            process_registry_list_size--;
        }
    }
}

void print_all_children(FILE *out) {
    for (size_t i = 0; i < process_registry_list_size; i++) {
        fprintf(out, "[%d] %s\n", process_registry[i]->pid, process_registry[i]->name);
    }
}

pid_t make_child_exec(char *prog, FILE *out, FILE *err, size_t argc, char **argv) {
    pid_t pid = fork();

    if (pid == 0) {
        char **args = malloc(sizeof(char *) * (argc + 1));
        for (size_t i = 0; i < argc; i++) { // Copy all arguments
            args[i] = strdup(argv[i]);
        }

        args[argc] = 0;

        // Child
        execvp(prog, args);
        fprintf(err, "execvp: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        fprintf(out, "[%d] %s\n", pid, prog);
        register_child(prog, pid);
        return pid;
    }

    return 0;
}

int wait_child(pid_t pid) {
    int status = 0;
    waitpid(pid, &status, 0);

    unregister_child_by_pid(pid);
    fprintf(signal_out, "[%d] Exit Code %d\n", pid, WEXITSTATUS(status));

    return status;
}

void set_child_signal_out(FILE *out) {
    signal_out = out;
}

void child_signal_exit() {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) != -1) {
        if (pid == 0 || pid == -1) {
            return;
        }

        unregister_child_by_pid(pid);
        fprintf(signal_out, "[%d] Exit Code %d\n", pid, WEXITSTATUS(status));
    }
}
