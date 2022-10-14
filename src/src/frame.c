#include "frame.h"
#include "utils.h"

static int state = START;
static unsigned char data[BUF_SIZE + 1] = {0};
static int data_idx = 0;

int stateStep(unsigned char buf, unsigned char expected, unsigned char addr)
{
    switch (state)
    {
    case START:
        if (buf == FLAG)
        {
            state = FLAG_RCV;
        }
        break;

    case FLAG_RCV:
        if (buf == addr)
        {
            state = A_RCV;
        }
        else if (buf == FLAG)
            state = FLAG_RCV;
        else
            state = START;
        break;

    case A_RCV:
        if (buf == expected) {
            state = C_RCV;
        }
        else if (buf == FLAG)
            state = FLAG_RCV;
        else if (isDuplicate(buf, expected)) {
            state = START;
            return 1; // discard the data
        }
        else
            state = START;
        break;

    case C_RCV:
        if (buf == (addr ^ expected)) {
            state = BCC_OK;
        }
        else if (buf == FLAG)
            state = FLAG_RCV;
        else
            state = START;
        break;

    case BCC_OK:
        if (buf == FLAG){
            return 1;

        }
        else {
            data_idx = 0;
            state = DATA;
        }
        break;

    case DATA:
        if (buf == FLAG) {
            state = BCC2_OK;
            data_idx--;
        }
        else {
            data[data_idx] = buf;
            data_idx++;
        }
        break;

    case BCC2_OK:
        unsigned char bcc = 0x00;
        deStuff(data , data_idx);
        for (int i = 0; i < data_idx; i++) {
            bcc ^= data[i];
        }

        if (data[data_idx - 1] == bcc) {
            state = START;
            return 1;
        }
        else {
            state = START;
            return 2;
        }
        break;

    case STOP:
        break;
    }
    return 0;
}

int writeCtrlFrame(int fd, unsigned char ctrl, unsigned char addr)
{
    unsigned char buf[BUF_SIZE + 1] = {0};
    buf[0] = FLAG;
    buf[1] = addr;
    buf[2] = ctrl;
    buf[3] = addr ^ ctrl;
    buf[4] = FLAG;
    buf[5] = '\0';
    return write(fd, buf, 5);
}

void setState(State new_state)
{
    state = new_state;
}

State getState()
{
    return state;
}

unsigned char* getData() {
    return data;
}

int isDuplicate(unsigned char buf, unsigned char expected) {
    if (expected != 0 && expected != (1 << 6)) 
        return 0;
    if (buf == expected) return 0;
    if (expected == 0 && buf == (1 << 6)) return 1;
    if (expected == (1 << 6) && buf == 0) return 1;
    return 0;
}
