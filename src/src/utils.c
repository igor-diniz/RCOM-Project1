#include "utils.h"

int stuff(const unsigned char *src, unsigned char* dest, int bufSize) {
    int dest_i = 0;
    for (int i = 0; i < bufSize; i++) {
        if (src[i] == 0x7e) {
            dest[dest_i] = 0x7d;
            dest_i++;
            dest[dest_i] = 0x5e;
        }
        else if (src[i] == 0x7d) {
            dest[dest_i] = 0x7d;
            dest_i++;
            dest[dest_i] = 0x5d;
        }
        else {
            dest[dest_i] = src[i];
        }
        dest_i++;
    }
    return dest_i;
}


int deStuff(const unsigned char *src, unsigned char* dest, int bufSize) {
    int dest_i = 0;
    for (int i = 0; i < bufSize; i++) {
        if (src[i] == 0x7d) {
            i++;
            if (src[i] == 0x5e) {
                dest[dest_i] = 0x7e;
            }
            else if (src[i] == 0x5d) {
                dest[dest_i] = 0x7d;
            }
        }
        else {
            dest[dest_i] = src[i];
        }
        dest_i++;
    }
    return dest_i;
}