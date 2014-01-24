#ifndef CO_SHA1_H
#define CO_SHA1_H

#include <stdint.h>

#include "common.h"

#define SM_COUNT 2
#define BLOCKS_PER_SM 8
#define THREADS_PER_BLOCK 2048 / BLOCKS_PER_SM
#define BLOCKS_PER_GRID SM_COUNT * BLOCKS_PER_SM

typedef struct hash_digest
{
    uint32_t h0;
    uint32_t h1;
    uint32_t h2;
    uint32_t h3;
    uint32_t h4;
} hash_digest_t;

#ifdef __cplusplus
extern "C" {
#endif
void force_kernel(uint32_t *d_result, const uint32_t idx);

cudaError_t copy_constants(uint32_t *h_block,
                               uint8_t *h_difficulty,
                               hash_digest_t *h_ctx);

#endif
#ifdef __cplusplus
}
#endif
