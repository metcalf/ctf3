#include <ctype.h>

void hash(char *string, unsigned int* output){
    unsigned long state1 = 5381, state2 = 261;
    int c;

    while((c = tolower(*string++))){
        state1 = ((state1 << 5) + state1) ^ c;
        state2 = ((state2 << 5) + state2) ^ c;
    }

    output[0] = state1 & 0x1FFFFF;
    output[1] = (state1 >> 21) & 0x1FFFFF;
    output[2] = (state1 >> (21*2)) & 0x1FFFFF;
    output[3] = state2 & 0x1FFFFF;
    output[4] = (state2 >> 21) & 0x1FFFFF;
    output[5] = (state2 >> (21*2)) & 0x1FFFFF;
}
