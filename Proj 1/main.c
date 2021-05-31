#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "strutil.h"
#include "builtin.h"
#include "child.h"

#ifndef _DEFAULT_ROOT_PROMPT_SUFFIX
#define _DEFAULT_ROOT_PROMPT_SUFFIX '#'
#endif

#ifndef _DEFAULT_USER_PROMPT_SUFFIX
#define _DEFAULT_USER_PROMPT_SUFFIX '>'
#endif

#ifndef _DEFAULT_SHELL_PROMPT
#define _DEFAULT_SHELL_PROMPT "308sh"
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef _HELP_DESC
#define _HELP_DESC "Usage: %s [OPTION]...\n\
Example: %s -p \"PROMPT\"\n\
\n\
Options:\n\
  -p    Set the prompt to this text\n\
  -h    Print this help message\n"
#endif

#ifndef _DEFAULT_ARG_ALLOC_SIZE
#define _DEFAULT_ARG_ALLOC_SIZE 24
#endif

char *self_name = "shell.exe";

void print_help(FILE *out, const char *pretext) {
    if (pretext != NULL) {
        fprintf(out, "%s\n", pretext);
    }

    fprintf(out, _HELP_DESC, self_name, self_name);
    fflush(out);
}

void print_prompt(FILE *out, char *prompt, bool ignore_euid) {
    if (ignore_euid) {
        fprintf(out, "%s ", prompt);
    } else {
        char suffix = _DEFAULT_USER_PROMPT_SUFFIX;
        if (geteuid() == 0) {
            suffix = _DEFAULT_ROOT_PROMPT_SUFFIX;
        }

        fprintf(out, "%s%c ", prompt, suffix);
    }

    // fflush(out);
}

typedef enum multi_args_t {
    PIPE,
    BACKGROUND,
    MULTI
} multi_args_t;

// Welcome to Shell in a Day. Start Datetime: 03/08/2021 ‏‎02:52:58 (the day it is due).
// This is the main entry function.
int main(int argc, char *argv[]) {
    int opt = -1;
    char *prompt = _DEFAULT_SHELL_PROMPT;

    if (argc > 0) {
        self_name = argv[0];
    }

    while ((opt = getopt(argc, argv, "hp:")) != -1) {
        switch (opt) {
            case 'p':
                if (optarg == NULL) {
                    print_help(stderr, "Prompt flag was provided, but no prompt text was provided\n");
                    exit(EXIT_FAILURE);
                }

                prompt = optarg;

                break;

            case 'h':
            default:
                print_help(stderr, NULL);
                exit(EXIT_SUCCESS);
                break;
        }
    }

    set_child_signal_out(stdout);
    signal(SIGCHLD, child_signal_exit);

    register_builtin("cd", cd);
    register_builtin("exit", exits);
    register_builtin("pwd", pwd);
    register_builtin("pid", pid);
    register_builtin("ppid", ppid);
    register_builtin("jobs", jobs);

    while (true) {
        print_prompt(stdout, prompt, false);
        char *input = NULL;
        size_t len = 0;
        bool background = false;
        int c = getline(&input, &len, stdin); // Get a line of input from the user.

        if (c == 0) {
            free(input);
            continue; // Fail Early.
        }

        size_t arg_alloc_size = _DEFAULT_ARG_ALLOC_SIZE;
        size_t num_args = 0;
        char **args = (char **) malloc(sizeof(char *) * _DEFAULT_ARG_ALLOC_SIZE); // Make a place to store the tokenized output.

        char *tok;
        for (num_args = 0; *input; num_args++) {
            tok = qtok(input, &input);

            if (num_args >= (arg_alloc_size - 1)) { // Increase our args array size to allow for more args.
                args = realloc(args, sizeof(char *) * (arg_alloc_size += _DEFAULT_ARG_ALLOC_SIZE));
            }

            tok[strcspn(tok, "\n")] = 0;

            args[num_args] = tok;
        }

        int last_arg_length = strlen(args[num_args - 1]);
        if (args[num_args - 1][last_arg_length - 1] == '&') {
            background = true;

            if (last_arg_length == 1) {
                args[num_args - 1] = NULL;
                num_args--;
            } else {
                args[num_args - 1][last_arg_length - 1] = '\0';
            }
        }

        int status = call_builtin(args[0], stdout, stderr, num_args, args); // Builtins always take precedence.
        if (status == -2) {
            // Not a builtin command. Exec It.
            pid_t pid = make_child_exec(args[0], stdout, stderr, num_args, args);

            if (!background) {
                wait_child(pid);
            }
        }

        free(args);
    }

    exit(EXIT_SUCCESS);
}
