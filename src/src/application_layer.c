// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "utils.h"
#include "frame.h"

#define BUF_SIZE 256
#define MAX_CHUNK_SIZE 128

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

    int nbytes;
    unsigned char buf[BUF_SIZE];

    // Transmitter
    if (parameters.role == LlTx) {
        // Openning {filename} and verifying if any error occurred
        int fd = open(filename, O_RDONLY);
        if (fd < 0) {
            printf("%s cannot be opened \n", filename);
            exit(-1);
        }

        while ((nbytes = read(fd, buf, MAX_CHUNK_SIZE)) != 0){ // zero indicates end of file
            if(nbytes == -1){
                printf("An error occurred in the reading of the %s", filename);
            }
        
            llwrite(buf, nbytes);
        }
    }

    // Receiver
    else if (parameters.role == LlRx) {
        int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXO);
        if (fd < 0) {
            printf("%s cannot be opened \n", filename);
            exit(-1);
        }

        int i = 0;
        while (i < 20) {
            llread(buf);
        }
    }

    llclose(1);
}
