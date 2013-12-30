#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "common.h"

void reset_timing(timing_info *info){
    int i;

    info->slot = 0;

    for(i=0; i < TIMING_SLOTS; i++){
        info->totals[i].tv_sec = 0;
        info->totals[i].tv_usec = 0;

        info->counts[i] = 0;
    }
}

void start_timing(timing_info *info){
    info->slot = 0;
    gettimeofday(&(info->start), NULL);
}

void time_point(timing_info *info){
    struct timeval curr;
    struct timeval *slot_time = &(info->totals[info->slot]);

    gettimeofday(&curr, NULL);

    timersub(&curr, &(info->start), &curr);
    timeradd(slot_time, &curr, slot_time);

    info->counts[info->slot]++;
    info->slot++;
}

void skip_point(timing_info *info){
    info->slot++;
}

void print_timing(timing_info *info){
    int i = 0;
    long times[TIMING_SLOTS];
    long diff;
    struct timeval *curr;

    for(i=0; info->counts[i]; i++){
        curr = &(info->totals[i]);
        times[i] = (curr->tv_sec * 1000 + curr->tv_usec / 1000) /
            info->counts[i];
    }

    for(i=0; info->counts[i]; i++){
        diff = times[i];
        if(i){
            diff -= times[i-1];
        }

        printf("Slot %d: %ld\t", i, diff);
    }

    printf("\n");
}

int pad_message(char *msg, unsigned int length, unsigned int buffer_length){
    int bit_length = length * 8;
    if(buffer_length < length + 9){
        puts("Buffer is too short!");
        return -1;
    }

    msg[length] = '\x80';

    memset(&msg[length+1], 0, 6);

    msg[length+7] = (bit_length >> 8) & 0xff;
    msg[length+8] =(char)bit_length & 0xff;

    return 0;
}
