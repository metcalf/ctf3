#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "hash.c"

#define SLOTS 524288

unsigned char data[SLOTS];

int main(){
    static const char outfile[] = "src/filter.bin";

    unsigned int output[FNS];
    int i;

    FILE *fp = fopen(INFILE, "r");
    char line[64];

    if(fp == NULL){
        perror(INFILE);
        return 1;
    }

    while(fgets(line, sizeof(line), fp) != NULL){
        line[strlen(line)-1] = 0;
        if(isupper(line[0])){
            continue;
        }

        hash(line, output);
        for(i = 0; i < FNS; i++){
            data[output[i] / 8] |= 1 << (output[i] % 8);
        }
    }

    fclose(fp);

    fp = fopen(outfile, "w");
    if(fp == NULL){
        perror(outfile);
        return 1;
    }

    fwrite(data, 1, SLOTS, fp);
    fclose(fp);

    return 0;
}
