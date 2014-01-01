#include <cuda_runtime.h>

#include <stdint.h>

#include "cuda.h"
#include "sha1.h"

hash_digest_t *d_sha_ctx;
uint32_t *d_block;
uint32_t *d_result;
uint32_t *d_mask;

int init_hasher(unsigned char difficulty){
    uint32_t mask = 0xffffffff << (32 - difficulty * 4);

    cudaMalloc(&d_sha_ctx, sizeof(hash_digest_t));
    cudaMalloc(&d_block, sizeof(uint32_t) * 16);
    cudaMalloc(&d_result, sizeof(uint32_t));
    cudaMalloc(&d_mask, sizeof(uint32_t));

    cudaMemcpy(d_mask, &mask, sizeof(uint32_t), cudaMemcpyHostToDevice);

    return 0;
}

void* force_hash(hash_args *args){
    uint32_t i;
    SHA_CTX msg_ctx;
    uint32_t words[16], result;
    uint32_t *block_ptr = (uint32_t *)&args->msg[BUFFER_LENGTH-BLOCK_LENGTH];
    uint32_t zero = 0;

    SHA1_Init(&msg_ctx);
    SHA1_Update(&msg_ctx, args->msg, BUFFER_LENGTH-BLOCK_LENGTH);

    hash_digest_t d_ctx = { msg_ctx.h0, msg_ctx.h1, msg_ctx.h2,
                            msg_ctx.h3, msg_ctx.h4 };

    for(i=0; i < 16; i++){
        words[i] = __builtin_bswap32(block_ptr[i]);
    }

    cudaMemcpy(d_sha_ctx, &d_ctx, sizeof(hash_digest_t),
               cudaMemcpyHostToDevice);
    cudaMemcpy(d_block, words, sizeof(uint32_t) * 16,
               cudaMemcpyHostToDevice);
    cudaMemcpy(d_result, &zero, sizeof(uint32_t),
               cudaMemcpyHostToDevice);

    i = 0;
    while(!(*args->stop)){
        cudaMemcpy(&d_block[11], &i, sizeof(uint32_t), cudaMemcpyHostToDevice);

        force_kernel(d_block, d_result,
                     d_sha_ctx, d_mask);

        cudaMemcpy(&result, d_result, sizeof(uint32_t), cudaMemcpyDeviceToHost);
        if(result){
            args->found = 1;
            block_ptr[12] = __builtin_bswap32(result-1);
            block_ptr[11] = __builtin_bswap32(i);
            break;
        }

        i++;
    }

    return NULL;
}

int free_hasher(){
    cudaFree(d_sha_ctx);
    cudaFree(d_block);
    cudaFree(d_result);
    cudaFree(d_mask);

    return 0;
}
