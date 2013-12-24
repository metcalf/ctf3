#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <sys/time.h>

#define MSG_LENGTH 292
#define COMMIT_LENGTH MSG_LENGTH+11
#define BLOCK_LENGTH 64
#define TIMING_SLOTS 10

typedef struct timing_info_struct {
    struct timeval start;
    struct timeval totals[10];
    int counts[10];
    int slot;
} timing_info;

typedef struct hash_args_struct {
    char *msg;
    char *stop;
    char found;
    unsigned char difficulty;
} hash_args;

void reset_timing(timing_info *info);
void start_timing(timing_info *info);
void time_point(timing_info *info);
void print_timing(timing_info *info);

#endif
