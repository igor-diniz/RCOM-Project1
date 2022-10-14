// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "utils.h"

#define BUF_SIZE 256

static LinkLayer parameters;

int setUp(const char *serialPort, const char *role, int baudRate,
           int nTries, int timeout)
{
    strcpy(parameters.serialPort, serialPort);
    if (strcmp(role, "tx") == 0) parameters.role = LlTx;
    else if (strcmp(role, "rx") == 0) parameters.role = LlRx;
    else exit (-1);
    parameters.baudRate = baudRate;
    parameters.nRetransmissions = nTries;
    parameters.timeout = timeout;

    return llopen(parameters);
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    if (setUp(serialPort, role, baudRate, nTries, timeout) == -1) {
        printf("Couldn't establish connection.\n");
    }


    unsigned char buf[BUFFER_SIZE + 1] = {0};
    for (int i = 0; i < 10; i++) {
        buf[i] = 'a' + i;
    }

    if (parameters.role == LlTx) {
        llwrite(buf, 10);
    }
    else if (parameters.role == LlRx) {
        llread(buf);
        printf("bytes read: \n %s\n", buf);
    }

    llclose(1);
}
