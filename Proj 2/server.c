#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

#include "Bank.h"
#include "chan.h"
#include "process.h"
#include "strutil.h"

int panic(const char* error) {
    fprintf(stderr, "ERROR: %s\n", error);
    fflush(stderr);

    return 1;
}

FILE* output;

chan_t* send_chan;
chan_t* done;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        return panic("Not enough startup arguments provided. At least 4 are needed.");
    }

    pid_t pid = getpid();

    int num_accounts = atoi(argv[2]);
    if (num_accounts == 0) {
        // There aren't any accounts to service, or it was NaN.
        return panic("The number of accounts is less than or equal to 0, or is not a number.");
    }

    initialize_accounts(num_accounts);

    #ifndef COARSE_LOCK
    initialize_account_mutexs(num_accounts);
    #else
    set_number_accounts(num_accounts);
    #endif

    initialize_processing();

    int num_workers = atoi(argv[1]);
    if (num_workers == 0) {
        // The number of workers was either specified to be zero, or was NaN.
        return panic("The number of workers is less than or equal to zero, or is not a number.");
    }

    send_chan = chan_init(1);
    done = chan_init(1);

    pthread_t* children = malloc(sizeof(pthread_t) * num_workers);
    if (children == NULL) {
        // Wasn't able to get memory.
        // Abort.
        return panic("Wasn't able to allocate enough memory to hold all the workers. Is the system out of memory?");
    }

    for (int i = 0; i < num_workers; i++) {
        int* id = malloc(sizeof(int*));
        *id = i;

        // Make all the worker threads.
        pthread_create(&children[i], NULL, process, (void *)id);
    }

    output = fopen(argv[3], "w");

    while(true) {
        char *input = NULL;
        size_t len = 0;

        int c = getline(&input, &len, stdin); // Get a line of input from the user.
        debug("main", pid, "READ %d", c);

        if (c == 0) {
            free(input);
            continue; // Fail Early.
        }

        if (strncmp(input, "END", 3) == 0) {
            break;
        }

        // Handle Input
        debug("main", pid, "RECEIVED COMMAND %s", input);
        request_t* request = malloc(sizeof(request_t));
        request->request = strdup(input);

        struct timeval starttime;
        gettimeofday(&starttime, NULL);

        request->starttime = starttime;

        if (chan_send(send_chan, request) != 0) {
            return panic("The request send channel was closed prematurely. This shouldn't happen. Was another main instance forked off?");
        }
    }

    debug("main", pid, "RECEIVED TERMINATE COMMAND. CLOSING CHANNELS AND WAITING FOR WORKERS TO TERMINATE.");

    chan_close(send_chan);
    chan_recv(done, NULL);

    debug("main", pid, "CHANNELS CLOSED, AWAITING WORKER CLOSURE.");
    for (int i = 0; i < num_workers; i++) {
        pthread_join(children[i], NULL);
    }

    chan_dispose(send_chan);
    chan_dispose(done);

    finalize_processing();

    #ifndef COARSE_LOCK
    finalize_account_mutexes(num_accounts);
    #endif

    free_accounts();
    fclose(output);

    return 0;
}
