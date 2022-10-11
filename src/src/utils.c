#include "utils.h"

void stuff(unsigned char *buf, int bufSize) {
    unsigned char tmp[BUFFER_SIZE + 1] = {0};
    int tmp_i = 0;
    for (int i = 0; i < bufSize; i++) {
        if (buf[i] == 0x7e) {
            tmp[tmp_i] = 0x7d;
            tmp_i++;
            tmp[tmp_i] = 0x5e;
        }
        else if (buf[i] == 0x7d) {
            tmp[tmp_i] = 0x7d;
            tmp_i++;
            tmp[tmp_i] = 0x5d;
        }
        else {
            tmp[tmp_i] = buf[i];
        }
        tmp_i++;
    }
    for (int i = 0; i < tmp_i; i++) {
        buf[i] = tmp[i];
    }
}


void deStuff(unsigned char *buf, int bufSize) {
    unsigned char tmp[BUFFER_SIZE + 1] = {0};
    int tmp_i = 0;
    for (int i = 0; i < bufSize; i++) {
        if (buf[i] == 0x7d) {
            i++;
            if (buf[i] == 0x5e) {
                tmp[tmp_i] = 0x7e;
            }
            else if (buf[i] == 0x5d) {
                tmp[tmp_i] = 0x7d;
            }
        }
        else {
            tmp[tmp_i] = buf[i];
        }
        tmp_i++;
    }
    for (int i = 0; i < tmp_i; i++) {
        buf[i] = tmp[i];
    }
}