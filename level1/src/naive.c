#include <stdio.h>
#include <openssl/sha.h>
#include <string.h>

#include "naive.h"

void* force_hash(hash_args *args){
    unsigned char hash[SHA_DIGEST_LENGTH];
    unsigned long long *modifier = &args->msg[COMMIT_LENGTH-BLOCK_LENGTH];
    int i;
    SHA_CTX msg_ctx;
    SHA_CTX tmp_ctx;

    SHA1_Init(&msg_ctx);
    SHA1_Update(&msg_ctx, args->msg,
                COMMIT_LENGTH-64);

    while(!*(args->stop)){
        (*modifier)++;

        memcpy(&tmp_ctx, &msg_ctx, sizeof(SHA_CTX));
        SHA1_Update(&tmp_ctx, &args->msg[COMMIT_LENGTH-BLOCK_LENGTH], BLOCK_LENGTH);
        SHA1_Final(hash, &tmp_ctx);

        for(i=0; i < args->difficulty; i++){
            if(hash[i] != 0){
                break;
            }
        }

        if(i == args->difficulty){
            args->found = 1;
            break;
        }
    }

    pthread_exit(NULL);
}
