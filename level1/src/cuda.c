#include <cuda_runtime.h>

#include <stdint.h>

#include "cuda.h"
#include "sha1.h"

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line)
{
   if (code != cudaSuccess)
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
   }
}

uint32_t *d_result;
uint32_t *result;
uint32_t mask;

int init_hasher(unsigned char difficulty){
    mask = 0xffffffff << (32 - difficulty * 4);

    cudaMalloc(&d_result, sizeof(uint32_t));
    cudaMallocHost(&result, sizeof(uint32_t));

    cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);

    return 0;
}

void* force_hash(hash_args *args){
    uint32_t i;
    SHA_CTX msg_ctx;
    uint32_t words[16];
    uint32_t *block_ptr = (uint32_t *)&args->msg[BUFFER_LENGTH-BLOCK_LENGTH];

    SHA1_Init(&msg_ctx);
    SHA1_Update(&msg_ctx, args->msg, BUFFER_LENGTH-BLOCK_LENGTH);

    hash_digest_t h_ctx = { msg_ctx.h0, msg_ctx.h1, msg_ctx.h2,
                            msg_ctx.h3, msg_ctx.h4 };

    for(i=0; i < 16; i++){
        words[i] = __builtin_bswap32(block_ptr[i]);
    }

    cudaMemset(d_result, 0, sizeof(uint32_t));
    gpuErrchk(copy_constants(words, &mask, &h_ctx));

    i = 0;
    while(!(*args->stop)){
        force_kernel(d_result, i);

        cudaMemcpy(result, d_result, sizeof(uint32_t),
                        cudaMemcpyDeviceToHost);
        if(*result){
            args->found = 1;
            block_ptr[12] = __builtin_bswap32((*result)-1);
            block_ptr[11] = __builtin_bswap32(i);
            break;
        }

        i++;
    }

    return NULL;
}

int free_hasher(){
    cudaFree(d_result);

    return 0;
}
