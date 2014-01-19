#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "hash.c"

#define MAX_LENGTH 64
#define READ_BUFFER 2048
#define OUT_BUFFER 3078

extern unsigned char bloom_data[] asm("_binary_src_filter_bin_start");
extern unsigned char bloom_size[] asm("_binary_src_filter_bin_size");

size_t size = (size_t)((void *)bloom_size);

#define match(offset) bloom_data[offset / 8] & (1 << offset % 8)

int write_word(char* word, int len, char* output){
    unsigned int hashOut[6];

    if(len){
        word[len] = 0;
        hash(word, hashOut);

        if(match(hashOut[0]) &&
           match(hashOut[1]) &&
           match(hashOut[2]) &&
           match(hashOut[3]) &&
           match(hashOut[4]) &&
           match(hashOut[5])){
            memcpy(output, word, len);
        } else {
            output[0] = '<';
            memcpy(&output[1], word, len);
            output[len+1] = '>';
            return len+2;
        }
    }
    return len;
}

int main () {
    char word[MAX_LENGTH];
    char buffer[READ_BUFFER];
    char output[OUT_BUFFER];
    char curr;
    int word_pos = 0, buff_pos, out_pos = 0, cnt;

    //setvbuf(stdout, NULL, _IOFBF, 0);
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);

    while((cnt = read(0, buffer, READ_BUFFER)) > 0){
        for(buff_pos=0; buff_pos < cnt; buff_pos++){
            curr = buffer[buff_pos];
            if(curr == '\n' || curr == ' '){
                out_pos += write_word(word, word_pos, &output[out_pos]);
                output[out_pos] = curr;
                out_pos++;
                word_pos = 0;
            } else {
                word[word_pos] = curr;
                word_pos++;
            }
        }

        write(1, output, out_pos);
        out_pos = 0;
    }

    return 0;
}
