// Link layer protocol implementation

#include "link_layer.h"
#include "frame.h"
#include <signal.h>

static int fd;
static struct termios oldtio;
static struct termios newtio;

static LinkLayer parameters;

static int numTries;
static int alarmTriggered;

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
                writeFrame(fd, SET, ADDR_T);
                setState(START);
                alarm(3);
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
                writeFrame(fd, UA, ADDR_T);
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
    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
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
