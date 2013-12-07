#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include "./libbloom/bloom.h"

#define MAX_ENTRIES 262144
#define MAX_LENGTH 48

char entries[MAX_ENTRIES][MAX_LENGTH];

int main (int arc, char **argv) {
    FILE *fp;
    char buffer[MAX_LENGTH];

    unsigned long totalEntries = 0;
    char word[MAX_LENGTH];
    char delim;
    int i;

    struct bloom bloom;
    bloom_init(&bloom, MAX_ENTRIES, 0.001);

    fp = fopen("/usr/share/dict/words", "r");

    if (fp == NULL)
        exit(EXIT_FAILURE);

    while(fscanf(fp, "%48[^\n]%*c", buffer) > 0){
        // Swallow anything beginning with an upper-case
        if(islower(buffer[0])){
            bloom_add(&bloom, buffer, strlen(buffer));
        }
    }
    fclose (fp);

    while(scanf(" %64[^ \n]%c", word, &delim) > 0) {
        strncpy(buffer, word, MAX_LENGTH);
        for(i=0; buffer[i]; i++){
            buffer[i] = tolower(buffer[i]);
        }

        if(bloom_check(&bloom, buffer, strlen(buffer))){
            printf("%s%c", word, delim);
        } else {
            printf("<%s>%c", word, delim);
        }
    }

    return 0;
}
