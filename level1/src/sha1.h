#ifndef CO_SHA1_H
#define CO_SHA1_H

#include <stdint.h>

#include "common.h"

#define BLOCKS_PER_GRID 16
#define THREADS_PER_BLOCK 256

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
void force_kernel(uint32_t *d_block, uint32_t *d_result,
                  hash_digest_t *d_sha_ctx, uint32_t *d_mask);

#endif
#ifdef __cplusplus
}
#endif
