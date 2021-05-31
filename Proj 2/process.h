#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>
#include <sys/time.h>

typedef struct request_t {
    struct timeval starttime;
    char* request;
} request_t;

#ifndef COARSE_LOCK
bool initialize_account_mutexs(int);
void finalize_account_mutexes();
#else
void set_number_accounts(int);
#endif

void initialize_processing();
void finalize_processing();

void* process(void*);

#endif
