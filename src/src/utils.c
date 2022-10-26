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


int deStuff(unsigned char *dest, int bufSize) {
    unsigned char src[BUFFER_SIZE + 1] = {0};
    for (int i = 0; i < bufSize; i++) {
        src[i] = dest[i];
    }
    int dest_i = 0;
    for (int i = 0; i < bufSize; i++) {
        if (src[i] == 0x7d && i + 1 < bufSize) {
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

void printBar(int cur, int total) {
    if (total > 0) {
        int pt = (cur * 100) /  total;
        printf("[");
        for (int i = 0; i < 200; i += 10) {
            if (i < pt * 2) printf("#");
            else printf(" ");
        }
        printf("] (%d%%)\n", pt);
    }
}