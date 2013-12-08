#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include <marisa.h>

#define MAX_LENGTH 64
#define READ_BUFFER 2048
#define OUT_BUFFER 3078

extern uint8_t trie_data[]      asm("_binary_trie_bin_start");
extern uint8_t trie_size[] asm("_binary_trie_bin_size");

marisa::Agent agent;
marisa::Trie trie;

int write_word(char* lowered, char* word, int len, char* output){
    if(len){
        word[len] = '\0';
        agent.set_query(lowered, len);
        if(trie.lookup(agent)){
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
    char lowered[MAX_LENGTH];
    char buffer[READ_BUFFER];
    char output[OUT_BUFFER];
    char curr;
    int word_pos = 0, buff_pos, out_pos = 0, cnt;

    //setvbuf(stdout, NULL, _IOFBF, 0);
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);

    size_t size = (size_t)((void *)trie_size);

    trie.map(trie_data, size);

    while((cnt = read(0, buffer, READ_BUFFER)) > 0){
        for(buff_pos=0; buff_pos < cnt; buff_pos++){
            curr = buffer[buff_pos];
            if(curr == '\n' || curr == ' '){
                out_pos += write_word(lowered, word, word_pos, &output[out_pos]);
                output[out_pos] = curr;
                out_pos++;
                word_pos = 0;
            } else {
                word[word_pos] = curr;
                lowered[word_pos] = tolower(curr);
                word_pos++;
            }
        }

        write(1, output, out_pos);
        out_pos = 0;
    }

    return 0;
}
