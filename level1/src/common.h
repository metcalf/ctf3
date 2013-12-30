#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <sys/time.h>

#define BUFFER_LENGTH 256 // divisible mod 512 bits plus a null terminator
#define PADDING_LENGTH 9
#define PREAMBLE_LENGTH 11
#define BLOCK_LENGTH 64

#define COMMIT_LENGTH BUFFER_LENGTH - PADDING_LENGTH
#define MSG_LENGTH COMMIT_LENGTH - PREAMBLE_LENGTH

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
void skip_point(timing_info *info);

int pad_message(char *msg, unsigned int length, unsigned int buffer_length);

#endif
