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
    char hex_hash[SHA_DIGEST_LENGTH*2];
    char hex_difficulty[SHA_DIGEST_LENGTH*2] = "00000008ffffffffffffffffffffffffffffffff";
    unsigned char difficulty;

    struct timeval start, end, diff;
    unsigned long total = 0, curr;

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
        memcpy(hex_difficulty, argv[2], SHA_DIGEST_LENGTH*2);
    }

    difficulty = parse_difficulty(hex_difficulty);
    printf("Starting benchmark with difficulty %02x\n", difficulty);

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

        for(j=0; j < 20; j++){
            sprintf(&hex_hash[j*2], "%02x", hash[j] & 0xff);
        }

        if(memcmp(hex_hash, hex_difficulty, SHA_DIGEST_LENGTH*2) > 0){
            printf("Msg:");
            for(j = 0; j < BUFFER_LENGTH; j++){
                printf("%02x", msg[j] & 0xff);
            }

            printf("\nBad hash: %.40s\n", hex_hash);
            exit(1);
        } else {
            printf("Successful run in %ld ms: %.40s\n", curr, hex_hash);
        }

        if(!args.found){
            puts("Failed to find a hash!");
            exit(1);
        }
        printf("\n");
    }

    printf("\n%ld ms per iteration (%d iters, %.40s difficulty)\n",
           total / iters,
           iters, hex_difficulty);

    free_hasher();

    exit(0);
}
