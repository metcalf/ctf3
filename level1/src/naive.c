#include <errno.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "naive.h"

SHA_CTX msg_ctx;
char worker_stop;
SHA_LONG mask;
char *solution;

pthread_mutex_t solution_mutex;
pthread_cond_t solved;

#define THREAD_COUNT 8

static void* force_worker(void* thread_id){
    SHA_CTX tmp_ctx;
    char block[BLOCK_LENGTH];

    memcpy(block, solution, BLOCK_LENGTH);

    block[BLOCK_LENGTH-PADDING_LENGTH-5] = (char) thread_id;
    unsigned int *modifier = (unsigned int*)&block[BLOCK_LENGTH-PADDING_LENGTH-4];

    while(!worker_stop){
        (*modifier)++;

        memcpy(&tmp_ctx, &msg_ctx, sizeof(SHA_LONG) * 5);

        SHA1_Transform(&tmp_ctx, block);

        if(!(tmp_ctx.h0 & mask)){
            // Found!
            while(pthread_mutex_trylock(&solution_mutex)){
                usleep(1);
                if(worker_stop)
                    break;
            }

            memcpy(solution, block, BLOCK_LENGTH);
            pthread_cond_signal(&solved);

            pthread_mutex_unlock(&solution_mutex);

            break;
        }
    }

    pthread_exit(NULL);
}

void init_hash(unsigned char difficulty){
    // Backwards because we're little endian
    mask = 0xffffffff << (32 - difficulty * 4);
}

void* force_hash(hash_args *args){
    int i;
    pthread_t threads[THREAD_COUNT];
    struct timeval now;
    struct timespec abstime;

    SHA1_Init(&msg_ctx);
    SHA1_Update(&msg_ctx, args->msg, BUFFER_LENGTH-BLOCK_LENGTH);

    worker_stop = 0;
    solution = &args->msg[BUFFER_LENGTH-BLOCK_LENGTH];

    pthread_mutex_init(&solution_mutex, NULL);
    pthread_cond_init (&solved, NULL);

    printf("Starting %d subthreads\n", THREAD_COUNT);
    for(i=0; i < THREAD_COUNT; i++){
        pthread_create(&threads[i], NULL, force_worker, (void *)i);
    }

    printf("Waiting for a solution\n");

    while(!(*args->stop)){

        gettimeofday(&now, NULL);
        abstime = (struct timespec) { now.tv_sec + 1, now.tv_usec*1000 };

        i = pthread_cond_timedwait(&solved, &solution_mutex, &abstime);
        if (i != ETIMEDOUT){
            args->found = 1;
            break;
        }
    }

    worker_stop = 1;

    for (i=0; i<THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    return NULL; // May be called from main thread
}
