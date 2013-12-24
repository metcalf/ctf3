#include <errno.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "naive.h"

SHA_CTX msg_ctx;
char worker_stop;
char *all_stop;
int difficulty;
char *solution;

pthread_mutex_t solution_mutex;
pthread_cond_t solved;

#define THREAD_COUNT 8

void* force_worker(void* thread_id){
    unsigned char hash[SHA_DIGEST_LENGTH];
    int i;
    SHA_CTX tmp_ctx;
    char current[64];

    // Set a byte so threads work separately
    current[32] = (char) thread_id;
    unsigned long long *modifier = current;

    while(!(*all_stop || worker_stop)){
        (*modifier)++;

        memcpy(&tmp_ctx, &msg_ctx, sizeof(SHA_CTX));

        SHA1_Update(&tmp_ctx, current, BLOCK_LENGTH);
        SHA1_Final(hash, &tmp_ctx);

        for(i=0; i < difficulty; i++){
            if(hash[i] != 0){
                break;
            }
        }

        if(i == difficulty){
            pthread_mutex_lock(&solution_mutex);

            memcpy(solution, current, BLOCK_LENGTH);

            pthread_cond_signal(&solved);
            pthread_mutex_unlock(&solution_mutex);

            break;
        }
    }

    pthread_exit(NULL);
}

void* force_hash(hash_args *args){
    int i;
    pthread_t threads[THREAD_COUNT];
    struct timeval now;
    struct timespec abstime;

    SHA1_Init(&msg_ctx);
    SHA1_Update(&msg_ctx, args->msg, COMMIT_LENGTH-BLOCK_LENGTH);

    worker_stop = 0;
    all_stop = args->stop;
    difficulty = args->difficulty;
    solution = &args->msg[COMMIT_LENGTH-BLOCK_LENGTH];

    pthread_mutex_init(&solution_mutex, NULL);
    pthread_cond_init (&solved, NULL);

    printf("Starting %d subthreads\n", THREAD_COUNT);
    for(i=0; i < THREAD_COUNT; i++){
        pthread_create(&threads[i], NULL, force_worker, (void *)i);
    }

    printf("Waiting for a solution\n");

    while(!(*all_stop)){

        gettimeofday(&now, NULL);
        abstime = (struct timespec) { now.tv_sec + 1, now.tv_usec*1000 };

        i = pthread_cond_timedwait(&solved, &solution_mutex, &abstime);
        if (i != ETIMEDOUT){
            worker_stop = 1;
            args->found = 1;
            break;
        }
    }

    for (i=0; i<THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_exit(NULL);
}
