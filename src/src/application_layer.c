// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#define BUF_SIZE 256

static LinkLayer parameters;

int setUp(const char *serialPort, const char *role, int baudRate,
           int nTries, int timeout)
{
    strcpy(parameters.serialPort, serialPort);
    if (strcmp(role, "tx")) parameters.role = LlTx;
    else if (strcmp(role, "rx")) parameters.role = LlRx;
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
    unsigned char buf[BUF_SIZE] = {0};
    unsigned char hello[BUF_SIZE] = {0};

    for (int i = 0; i < 6; i++) {
        hello[i] = 97 + i;
    }
    
    hello[5] = '\n';
    if (parameters.role == LlTx) { 
        llwrite(hello, strlen((char*)hello));
        printf("Written to port \n");
    }
    else {
        llread(buf);
        printf("%s\n", buf);
    }

    llclose(1);
}
