#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "common.h"

#ifdef IMPL_THREAD
#include "naive.h"
#elif IMPL_CUDA
#include "cuda.h"
#else
#include "gpgpu.h"
#endif

int main(int argc, char **argv){
    int i, j;
    char unused_stop = 0;
    int iters = 10;
    hash_args args;
    char msg[BUFFER_LENGTH];
    unsigned char hash[SHA_DIGEST_LENGTH];
    unsigned char difficulty = 6;

    struct timeval start, end, diff;
    unsigned long total = 0, curr;
    int tmp;

    // Fill in some deterministic data
    for(i = 0; i < BUFFER_LENGTH; i++){
        msg[i] = i % 128;
    }

    pad_message(msg, COMMIT_LENGTH, BUFFER_LENGTH);

    args.stop = &unused_stop;
    args.msg = (char*)msg;

    if(argc > 1){
        iters = atoi(argv[1]);
    }

    if(argc > 2){
        difficulty = atoi(argv[2]);
    }

    init_hasher(difficulty);

    for(i = 0; i < iters; i++){
        memset(msg, 0, COMMIT_LENGTH);
        *((int*)(&(msg[0]))) = i;
        args.found = 0;

        cudaProfilerStart();
        gettimeofday(&start, NULL);
        force_hash(&args);
        gettimeofday(&end, NULL);
        cudaProfilerStop();

        timersub(&end, &start, &diff);
        curr = diff.tv_sec * 1000 + diff.tv_usec / 1000;
        total += curr;

        SHA1(msg, COMMIT_LENGTH, hash);
        tmp = __builtin_bswap32 (*((int*)hash));

        if(tmp >> (32 - difficulty * 4)){
            printf("Msg:");
            for(j = 0; j < BUFFER_LENGTH; j++){
                printf("%02x", msg[j] & 0xff);
            }

            printf("\nBad hash! ");
            for(j=0; j < 20; j++){
                printf("%02x", hash[j]);
            }
            printf("\n");
            exit(1);
        } else {
            printf("Successful run in %ld ms\n", curr);
        }

        if(!args.found){
            puts("Failed to find a hash!");
            exit(1);
        }
        printf("\n");
    }

    printf("\n%ld ms per iteration (%d iters, %d difficulty)\n",
           total / iters,
           iters, difficulty);

    free_hasher();

    exit(0);
}
