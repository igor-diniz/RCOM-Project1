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

        int i = 0;

        while ((nbytes = read(fd, buf, MAX_CHUNK_SIZE)) != 0){ // zero indicates end of file
            if(nbytes == -1){
                printf("An error occurred in the reading of the %s", filename);
            }

            i++;
        
            llwrite(buf, nbytes);
        }
    }

    // Receiver
    else if (parameters.role == LlRx) {
        int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXO);
        //int fileSize = 0;
        //char* rcvFilename[BUFFER_SIZE] = {0};
        if (fd < 0) {
            printf("%s cannot be opened \n", filename);
            exit(-1);
        }

        int frameSize = llread(buf);

        /*if (buf[0] == 2) { // start
            int i = 1;
            for (; i < frameSize; i++) {
                if (buf[i] == 0) { // size
                    int l = buf[i + 1];
                    i += 2;
                    for (int k = l; k >= 0; k++) {
                        fileSize += (buf[i] << (8 * k));
                        i++;
                    }
                }
                if (buf[i] == 1) { // file name
                    int l = buf[i + 1];
                    i += 2;
                    for (int k = 0; k < l; k++) {
                        rcvFilename[k] = buf[i];
                        rcvFilename[k + 1] = '\0';
                        i++;
                    }
                }
        
                }
            }
        }*/
    

        int i = 0;
        while (i < 86) {
            llread(buf);
            i++;
        }
    }

    llclose(1);
}
