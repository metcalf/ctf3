#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "naive.h"
#include "common.h"

int main(int argc, char **argv){
    int i, j;
    char unused_stop = 0;
    int iters = 10;
    hash_args args;
    char msg[BUFFER_LENGTH];
    unsigned char hash[SHA_DIGEST_LENGTH];
    struct timeval start, end, diff;
    unsigned long total = 0;
    int tmp;

    memset(msg, 0, BUFFER_LENGTH);

    pad_message(msg, COMMIT_LENGTH, BUFFER_LENGTH);

    args.difficulty = 6;
    args.stop = &unused_stop;
    args.msg = (char*)msg;

    if(argc > 1){
        iters = atoi(argv[1]);
    }

    if(argc > 2){
        args.difficulty = atoi(argv[2]);
    }

    for(i = 0; i < iters; i++){
        memset(msg, 0, COMMIT_LENGTH);
        *((int*)(&(msg[0]))) = i;
        args.found = 0;

        gettimeofday(&start, NULL);
        force_hash(&args);
        gettimeofday(&end, NULL);

        timersub(&end, &start, &diff);
        total += diff.tv_sec * 1000 + diff.tv_usec / 1000;

        SHA1(msg, COMMIT_LENGTH, hash);
        tmp = __builtin_bswap32 (*((int*)hash));

        if(tmp >> (32 - args.difficulty * 4)){
            printf("Msg:");
            for(j = 0; j < BUFFER_LENGTH; j++){
                printf("%02x", msg[j] & 0xff);
            }

            printf("\nBad hash! ", hash[0], hash[1], hash[2], hash[3]);
            for(j=0; j < 20; j++){
                printf("%02x", hash[j]);
            }
            printf("\n");
            exit(1);
        }

        if(!args.found){
            puts("Failed to find a hash!");
            exit(1);
        }
    }

    printf("\n%ld ms per iteration (%d iters, %d difficulty)\n",
           total / iters,
           iters, args.difficulty);

    exit(0);
}
