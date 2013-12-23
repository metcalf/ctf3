#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#define MSG_LENGTH 292
#define COMMIT_LENGTH MSG_LENGTH+11
#define BLOCK_LENGTH 64

typedef struct hash_args_struct {
    char *msg;
    char *stop;
    char found;
    unsigned char difficulty;
} hash_args;

#endif
