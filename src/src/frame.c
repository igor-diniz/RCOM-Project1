#include "frame.h"
#include "utils.h"

static int state = START;
static unsigned char data[BUF_SIZE + 1] = {0};
static int data_idx = 0;
static unsigned char last_ctrl;

int stateStep(unsigned char buf, unsigned char expected, unsigned char addr)
{
    switch (state)
    {
    case START:
        data_idx = 0;
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
        else
            state = START;
        break;

    case C_RCV:
        if (buf == (addr ^ expected)) {
            state = BCC_OK;
            last_ctrl = buf;
        }
        else if (buf == FLAG)
            state = FLAG_RCV;
        else
            state = START;
        break;

    case BCC_OK:
        if (buf == FLAG)
            return 1;
        else {
            if (last_ctrl == expected) {
                state = DATA;
            }
            else if (expected == RR && last_ctrl == REJ) {
                return 2;
            }
            else if (last_ctrl < expected) { // duplicate
                return 1;
            }
            else return 0;
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
        for (int i = 0; i < data_idx - 1; i++) {
            bcc ^= data[i];
        }
        if (data[data_idx - 1] == bcc) {
            return 1;
        }
        else {
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
