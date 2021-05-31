#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <stdatomic.h>

#include "chan.h"
#include "strutil.h"
#include "builtin.h"
#include "process.h"
#include "Bank.h"

#ifndef _DEFAULT_ARG_ALLOC_SIZE
#define _DEFAULT_ARG_ALLOC_SIZE 24
#endif

#ifndef COARSE_LOCK
bool lock_account(int account);
bool unlock_account(int account);

static pthread_mutex_t* account_mutexs;
#else
#define _GNU_SOURCE
static pthread_mutex_t account_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

extern chan_t* send_chan;
extern chan_t* done;
extern FILE* output;

static int number_accounts = 0;

// All the "builtin" looking code was borrowed from Project 1.
static size_t builtin_list_size = 0;
static atomic_uint request_count = 0;
static builtin_t** builtins = NULL;
static size_t builtin_alloc_size = _DEFAULT_BUILTIN_LIST_SIZE;

void register_builtin(char *name, builtin_func_t *func) {
    if (builtins == NULL) {
        builtins = malloc(sizeof(builtin_t *) * _DEFAULT_BUILTIN_LIST_SIZE);
    }

    if (builtin_list_size >= (builtin_alloc_size - 1)) {
        builtins = realloc(builtins, sizeof(builtin_t *) * (builtin_alloc_size += _DEFAULT_BUILTIN_LIST_SIZE));
    }

    builtin_t* a = malloc(sizeof(builtin_t));

    a->name = name;
    a->func = func;

    builtins[builtin_list_size++] = a;
}

int call_builtin(char *name, FILE *out, struct timeval time, size_t argc, char **argv) {
    for (size_t i = 0; i < builtin_list_size; i++) {
        if (strcmp(builtins[i]->name, name) == 0) {
            return builtins[i]->func(out, ++request_count, time, argv, argc);
        }
    }

    return -2;
}

int check(FILE* out, atomic_uint id, struct timeval time, char** argv, size_t argc) {
    debug("check", id, "FILE POINTER: %p; ID: %u; STARTTIME: %ld.%06ld; NUM OF ARGS: %zu", (void *) out, id, time.tv_sec, time.tv_usec, argc);

    #ifdef DEBUG
    for (size_t i = 0; i < argc; i++) {
        debug("check", id, "ARG: %s", argv[i]);
    }
    #endif

    if (argc < 2) {
        fprintf(stderr, "THE COMMAND PROVIDED HAS FEWER THAN THE REQUIRED ARGUMENTS. FORMAT: CHECK <ACCOUNT>\n");
        return -1;
    }

    int account = atoi(argv[1]);

    debug("check", id, "NUMBER OF ACCOUNTS: %d", number_accounts);

    if (account > number_accounts || account < 0) {
        fprintf(stderr, "THE PROVIDED ACCOUNT ID IS LARGER THAN THE TOTAL AMOUNT OF ACCOUNTS OR LESS THAN 0. FORMAT: CHECK <ACCOUNT>\n");
        return -1;
    }

    #ifndef COARSE_LOCK
    if (!lock_account(account)) {
        fprintf(stderr, "THE PROVIDED ACCOUNT ID IS LARGER THAN THE TOTAL AMOUNT OF ACCOUNTS OR LESS THAN 0. FORMAT: CHECK <ACCOUNT>\n");
        return -1;
    }
    #else
    debug("check", id, "LOCKING ACCOUNT TABLE");
    pthread_mutex_lock(&account_mutex);
    #endif

    printf("ID %u\n", id);
    fflush(stdout);

    int amount = read_account(account);

    debug("check", id, "ACCOUNT %d HAS %d", account, amount);

    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    fprintf(output, "%u BAL %d TIME %ld.%06ld %ld.%06ld\n", id, amount, time.tv_sec, time.tv_usec, endtime.tv_sec, endtime.tv_usec);
    fflush(output);

    #ifndef COARSE_LOCK
    if (!unlock_account(account)) {
        fprintf(stderr, "THE PROVIDED ACCOUNT ID IS LARGER THAN THE TOTAL AMOUNT OF ACCOUNTS OR LESS THAN 0. FORMAT: CHECK <ACCOUNT>\n");
        return -1;
    }
    #else
    debug("check", id, "UNLOCKING ACCOUNT TABLE");
    pthread_mutex_unlock(&account_mutex);
    #endif

    debug("check", id, "COMMAND COMPLETE");
    return 0;
}

typedef struct transfer_t {
    int account;
    int amount;
    int current_amount;
} transfer_t;

int sorter(const void* a, const void* b) {
    const transfer_t* p1 = (transfer_t *)a;
    const transfer_t* p2 = (transfer_t *)b;

    if (p1->account < p2->account) {
        return -1;
    } else if (p1->account > p2->account) {
        return 1;
    } else {
        return 0;
    }
}

int transact(FILE* out, atomic_uint id, struct timeval time, char** argv, size_t argc) {
    debug("transact", id, "FILE POINTER: %p; ID: %u; STARTTIME: %ld.%06ld; NUM OF ARGS: %zu", (void *) out, id, time.tv_sec, time.tv_usec, argc);

    #ifdef DEBUG
    for (size_t i = 0; i < argc; i++) {
        debug("transact", id, "ARG: %s", argv[i]);
    }
    #endif

    if ((argc - 1) % 2 != 0) {
        fprintf(stderr, "THE COMMAND PROVIDED HAS AN INCORRECT NUMBER OF ARGUMENTS. FORMAT: TRANS <ACCOUNT> <AMOUNT> <ACCOUNT> <AMOUNT>\n");
        return -1;
    }

    printf("ID %u\n", id);
    fflush(stdout);

    transfer_t* transfers = calloc((argc - 1) / 2, sizeof(transfer_t));

    #ifdef COARSE_LOCK
    debug("transact", id, "LOCKING ACCOUNT TABLE");
    pthread_mutex_lock(&account_mutex);
    #endif

    int j = 0;
    for (size_t i = 1; i < argc; i += 2) {
        transfers[j].account = atoi(argv[i]);
        transfers[j].amount = atoi(argv[i + 1]);

        debug("transact", id, "PARSED AMOUNT %d FROM ACCOUNT %s", transfers[i].amount, argv[i + 1]);

        j++;
    }

    qsort(transfers, j, sizeof(transfer_t), sorter);

    bool has_bad_account = false;
    int bad_account_index = -1;
    for (int i = 0; i < j; i++) {
        #ifdef COARSE_LOCK
        debug("transact", id, "CHECKING ACCOUNT %d", transfers[i].account);
        if (transfers[i].account > number_accounts || transfers[i].account < 0) {
        #else
        debug("transact", id, "LOCKING ACCOUNT %d", transfers[i].account);
        if (!lock_account(transfers[i].account)) {
        #endif
            fprintf(stderr, "THE PROVIDED ACCOUNT ID IS LARGER THAN THE TOTAL AMOUNT OF ACCOUNTS. FORMAT: TRANS <ACCOUNT> <AMOUNT> <ACCOUNT> <AMOUNT>\n");
            has_bad_account = true;
            bad_account_index = i;
            debug("transact", id, "BAD ACCOUNT INDEX: %d", bad_account_index);
            break;
        }

        transfers[i].current_amount = read_account(transfers[i].account);
        debug("transact", id, "ACCOUNT %d HAS %d", transfers[i].account, transfers[i].current_amount);
        debug("transact", id, "ACCOUNT %d AMOUNT TO ADD %d", transfers[i].account, transfers[i].amount);

        if ((transfers[i].current_amount + transfers[i].amount) < 0) {
            struct timeval endtime;
            gettimeofday(&endtime, NULL);
            fprintf(output, "%u ISF %d TIME %ld.%06ld %ld.%06ld\n", id, transfers[i].account, time.tv_sec, time.tv_usec, endtime.tv_sec, endtime.tv_usec);
            fflush(output);

            has_bad_account = true;
            bad_account_index = i;
            debug("transact", id, "ISF ACCOUNT INDEX: %d", bad_account_index);
            break;
        }
    }

    if (!has_bad_account) {
        for (int i = 0; i < j; i++) {
            write_account(transfers[i].account, transfers[i].amount + transfers[i].current_amount);
            transfers[i].current_amount += transfers[i].amount;

            debug("transact", id, "WROTE AMOUNT %d TO ACCOUNT %d", transfers[i].current_amount, transfers[i].account);
        }

        struct timeval endtime;
        gettimeofday(&endtime, NULL);
        fprintf(output, "%u OK TIME %ld.%06ld %ld.%06ld\n", id, time.tv_sec, time.tv_usec, endtime.tv_sec, endtime.tv_usec);
        fflush(output);
    }

    #ifdef COARSE_LOCK
    debug("transact", id, "UNLOCKING ACCOUNT TABLE");
    pthread_mutex_unlock(&account_mutex);
    #else
    int end = j;

    if (has_bad_account) {
        end = bad_account_index;
    }

    for (int i = 0; i <= end; i++) {
        debug("transact", id, "UNLOCKING ACCOUNT %d", transfers[i].account);
        unlock_account(transfers[i].account);
    }
    #endif

    free(transfers);

    debug("transact", id, "COMMAND COMPLETE");
    return 0;
}

void initialize_processing() {
    register_builtin("CHECK", check);
    register_builtin("TRANS", transact);
}

void finalize_processing() {
    for (size_t i = 0; i < builtin_list_size; i++) {
        free(builtins[i]);
    }

    free(builtins);
    builtin_list_size = 0;
}

#ifndef COARSE_LOCK
bool initialize_account_mutexs(int num_accounts) {
    number_accounts = num_accounts;
    account_mutexs = malloc(sizeof(pthread_mutex_t) * num_accounts);

    for (int i = 0; i < num_accounts; i++) {
        int status = pthread_mutex_init(&account_mutexs[i], NULL);

        if (status != 0) {
            return false;
        }
    }

    return true;
}

void finalize_account_mutexes() {
    for (int i = 0; i < number_accounts; i++) {
        pthread_mutex_destroy(&account_mutexs[i]);
    }

    free(account_mutexs);
}

bool lock_account(int account_number) {
    if (account_number > number_accounts || account_number < 0) {
        return false;
    }

    pthread_mutex_lock(&account_mutexs[account_number]);
    return true;
}

bool unlock_account(int account_number) {
    if (account_number > number_accounts || account_number < 0) {
        return false;
    }

    pthread_mutex_unlock(&account_mutexs[account_number]);
    return true;
}
#else
void set_number_accounts(int accounts) {
    number_accounts = accounts;
}
#endif

void* process(void* ptr) {
    void* data;

    int id = *((int *) ptr);

    while(true) {
        if (chan_recv(send_chan, &data) != 0) {
            debug("process", id, "CHANNEL RECEIVE FAILED. IT WAS PROBABLY CLOSED.");
            break;
        }

        request_t* input = (request_t *) data;

        debug("process", id, "PROCESSING (VERBATIM): %s %ld.%06ld", input->request, input->starttime.tv_sec, input->starttime.tv_usec);

        size_t arg_alloc_size = _DEFAULT_ARG_ALLOC_SIZE;
        size_t num_args = 0;
        char** args = calloc(_DEFAULT_ARG_ALLOC_SIZE, sizeof(char *)); // Make a place to store the tokenized output.

        char* tok;
        for (num_args = 0; *input->request; num_args++) {
            tok = qtok(input->request, &input->request);

            if (num_args >= (arg_alloc_size - 1)) { // Increase our args array size to allow for more args.
                args = realloc(args, sizeof(char *) * (arg_alloc_size += _DEFAULT_ARG_ALLOC_SIZE));
            }

            tok[strcspn(tok, "\n")] = 0;

            debug("process", id, "TOK: %s", tok);

            args[num_args] = tok;
        }

        debug("process", id, "PARSED %zu ARGS; COMMAND ARG: %s; STARTTIME: %ld.%06ld", num_args, args[0], input->starttime.tv_sec, input->starttime.tv_usec);

        call_builtin(args[0], output, input->starttime, num_args, args);
        free(args);
    }

    chan_send(done, 0);

    debug("process", id, "WORKER SHUTTING DOWN");
    free(ptr);

    return NULL;
}
