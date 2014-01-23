#include <ctype.h>

#define FNS 11

void hash(char *string, unsigned int* output){
    unsigned long state[4] = {5381, 261, 0, 5381};
    int c, i;

    while((c = tolower(*string++))){
        state[0] = ((state[0] << 5) + state[0]) ^ c;
        state[1] = ((state[1] << 5) + state[1]) ^ c;
        state[2] = c + (state[2] << 6) + (state[2] << 16) - state[2];
        state[3] = c + (state[3] << 6) + (state[3] << 16) - state[3];
    }

    for(i = 0; i < FNS; i++){
        output[i] = 0x3FFFFF & (state[(i * 22) / 64] >> ((i % 3) * 22));
    }
    output[2] |= 0x03 & (state[3] << 44);
    output[5] |= 0x03 & (state[3] << 46);
    output[8] |= 0x03 & (state[3] << 48);
}
