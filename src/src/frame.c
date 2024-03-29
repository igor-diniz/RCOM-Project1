#include "frame.h"
#include "utils.h"

static int state = START;
static int is_disc = 0;
static unsigned char data[BUF_SIZE + 1] = {0};
static int data_idx = 0;

Step stateStep(unsigned char buf, unsigned char expected, unsigned char addr)
{
    unsigned char bcc;
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
        is_disc = 0;
        if (buf == FLAG) {
            state = FLAG_RCV;
        }
        else if (buf == DISC) {
            is_disc = 1;
            state = C_RCV;
        }
        else if (buf == (REJ | (expected & 0x80))) {
            state = START;
            return REJECTED;
        }
        else {
            int test = testCtrl(buf, expected);
            if (test == 0 || test == -1) { // not valid
                state = START;
            }
            else if (test == 1) { // Valid
                state = C_RCV;
            }
            else { // duplicate
                state = START;
                return DUPLICATE;
            }
        }
        break;

    case C_RCV:
        if (buf == (addr ^ expected)) {
            state = BCC_OK;
        }
        else if (is_disc && (addr ^ DISC)) {
            state = BCC_OK;
        }
        else if (buf == FLAG)
            state = FLAG_RCV;
        else {
            state = START;
        }
        break;

    case BCC_OK:
        if (buf == FLAG){
            if (is_disc) return DISCONNECT;
            return COMPLETE;
        }
        else {
            data_idx = 0;
            data[data_idx] = buf;
            data_idx++;
            state = DATA;
        }
        break;

    case DATA:
        if (buf == FLAG) { // finished
            bcc = 0x00;
            data_idx = deStuff(data , data_idx);
            int bcc_rcv = data[data_idx - 1];
            for (int i = 0; i < data_idx - 1; i++) {
                bcc ^= data[i];
            }

            if (bcc_rcv == bcc) {
                state = START;
                return COMPLETE;
            }
            else {
                state = START;
                return REJECTED;
            }
            break;
        }
        else {
            data[data_idx] = buf;
            data_idx++;
        }
        break;

    case STOP:
        break;
    }
    return CONTINUE;
}

int writeCtrlFrame(int fd, unsigned char ctrl, unsigned char addr)
{
    unsigned char buf[BUF_SIZE + 1] = {0};

    buf[0] = FLAG;         // flag
    buf[1] = addr;         // campo de endereço: 0x03 ou 0x01
    buf[2] = ctrl;         // campo de controlo: SET,DISC,UA,RR,REJ
    buf[3] = addr ^ ctrl;  // campo de proteção de cabeçalho
    buf[4] = FLAG;         // flag
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

int getData(unsigned char* dest) {
    for (int i = 0; i < data_idx; i++) {
        dest[i] = data[i];
    }
    return data_idx;
}

int testCtrl(unsigned char buf, unsigned char expected) {
    if (expected != 0 && expected != (1 << I_CTRL_SHIFT)) 
        return (buf == expected);
    if (buf == expected) return 1;
    if (expected == 0 && buf == (1 << I_CTRL_SHIFT)) return 2;
    if (expected == (1 << I_CTRL_SHIFT) && buf == 0) return 2;
    return -1;
}
