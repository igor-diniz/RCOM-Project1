// Link layer protocol implementation

#include "link_layer.h"
#include "frame.h"
#include "utils.h"
#include <signal.h>

static int fd;
static struct termios oldtio;
static struct termios newtio;

static LinkLayer parameters;

static int numTries;
static int alarmTriggered;

static int frameNumber = 0;

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

#define BUF_SIZE 256

void alarmHandler(int signum) {
    alarmTriggered = 0;
    numTries++;
}

int openPort(LinkLayer connectionParameters) {
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }


    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 1 char received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    return 1;
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    unsigned char buffer[BUF_SIZE + 1] = {0};
    parameters = connectionParameters;
    openPort(connectionParameters);
    signal(SIGALRM, alarmHandler);
    numTries = 0;
    alarmTriggered = 0;

    if (connectionParameters.role == LlTx) {
        while ((numTries < connectionParameters.nRetransmissions) && getState() != STOP) {
            if (!alarmTriggered) {
                writeCtrlFrame(fd, SET, ADDR_T);
                setState(START);
                alarm(connectionParameters.timeout);
                alarmTriggered = 1;
                printf("Sent SET frame.\n");
            }
            read(fd, buffer, 1);
            if (stateStep(buffer[0], UA, ADDR_T)) {
                printf("Received UA frame.\n");
                return 0;
            }
        }
    }
    else if (connectionParameters.role == LlRx) {
        while (getState() != STOP) {
            read(fd, buffer, 1);
            if (stateStep(buffer[0], SET, ADDR_T)) {
                printf("Read SET frame.\n");
                writeCtrlFrame(fd, UA, ADDR_T);
                printf("Sent UA frame.\n");
                return 0;
            };
        }
    }
    return -1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    unsigned char tmp[BUF_SIZE + 1] = {0};
    unsigned char buffer[BUF_SIZE + 1] = {0};
    unsigned char copy[BUF_SIZE + 1] = {0};
    alarmTriggered = 0;
    int nbytes = 0;
    tmp[0] = FLAG;
    tmp[1] = ADDR_T;
    tmp[2] = frameNumber;
    tmp[3] = ADDR_T ^ frameNumber;
    bufSize = stuff(buf, copy, bufSize);
    int i = 0;
    for (; i < bufSize; i++) {
        tmp[i + 4] = copy[i];
    }
    tmp[i] = 0;  //TODO bcc
    tmp[i + 1] = FLAG;
    tmp[i + 2] = '\0';

    while ((numTries < parameters.nRetransmissions) && getState() != STOP) {
        if (!alarmTriggered) {
            nbytes = write(fd, tmp, i);
            setState(START);
            alarm(parameters.timeout);
            alarmTriggered = 1;
            printf("%d bytes written.\n", nbytes);
        }
        read(fd, buffer, 1);
        //TODO check for REJ
        if (stateStep(buffer[0], RR, ADDR_T)) {
            printf("Received RR frame.\n");
            return 0;
        }
    }
    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    sleep(2);
    writeCtrlFrame(fd, RR, ADDR_T);
    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}
