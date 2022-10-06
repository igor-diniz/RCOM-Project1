// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "frame.h"

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

int setConnection() {
    unsigned char buf[BUF_SIZE] = {0};
    if (parameters.role == LlTx) {
        writeFrame(SET, ADDR_T);
        printf("Sent SET frame.\n");
        sleep(1);
        if (readFrame(buf, UA, ADDR_T) == 0)
            printf("Read UA frame.\n");
    }
    else if (parameters.role == LlRx) {
        if (readFrame(buf, SET, ADDR_T) == 0) {
            writeFrame(UA, ADDR_T);
            printf("Sent UA frame.\n");
        }
    }
    return 0;
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    setUp(serialPort, role, baudRate, nTries, timeout);

    setConnection();

    llclose(1);
}
