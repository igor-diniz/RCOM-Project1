// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

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
    setUp(serialPort, role, baudRate, nTries, timeout);

    llclose(1);
}
