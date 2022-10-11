#include "frame.h"

static int state = START;

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
        if (buf == expected)
        {
            state = C_RCV;
        }
        else if (buf == FLAG)
            state = FLAG_RCV;
        else
            state = START;
        break;

    case C_RCV:
        if (buf == (addr ^ expected))
            state = BCC_OK;
        else if (buf == FLAG)
            state = FLAG_RCV;
        else
            state = START;
        break;

    case BCC_OK:
        if (buf == FLAG)
            return 1;
        else
            state = START;
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
