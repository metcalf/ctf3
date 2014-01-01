#ifndef CUDA_H
#define CUDA_H

#include "common.h"

int init_hasher(unsigned char difficulty);
void *force_hash(hash_args *args);
int free_hasher();

#endif
