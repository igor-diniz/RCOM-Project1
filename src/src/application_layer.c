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


/*
int configureDataPackage(unsigned char* buf, int pkgIndex){
    unsigned char auxBuffer[BUF_SIZE] = {0};
    auxBuffer[0] = 1;                        // C
    auxBuffer[1] = pkgIndex % 255;           // N
    auxBuffer[2] = MAX_CHUNK_SIZE / 256;     // L1
    auxBuffer[3] = MAX_CHUNK_SIZE % 256;     // L2
    memcpy(&auxBuffer[4], buf, MAX_CHUNK_SIZE);
    memcpy(buf, auxBuffer[1], MAX_CHUNK_SIZE + 4);
}
*/

/*
int configureCtrlPackage(int fd, unsigned char* buf, int fileSize, const char *filename){
    buf[0] = 2;
    buf[1] = 0;     // fileSize
    int numOct = (fileSize / 255) + fileSize % 255;
    buf[2] =  numOct;

    for(int i = 0; i < numOct; i++){
        if (i != numOct)
            buf[3 + i] = 255;
        else if (fileSize % 255){ // if == numOct
            buf[3 + i] = fileSize % 255; 
        }
    }
}
*/

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
       
       /*
       int fileSize;
        fileSize = lseek(fd, 0, SEEK_END);

        unsigned char ctrlPkg[BUF_SIZE] = {0};
        configureCtrlPackage(fd, ctrlPkg, fileSize, filename);
        */

        int i = 0;

        while ((nbytes = read(fd, buf, MAX_CHUNK_SIZE)) != 0){ // zero indicates end of file
            if(nbytes == -1){
                printf("An error occurred in the reading of the %s", filename);
            }

            i++;
        
            llwrite(buf, nbytes);
        }
        close(fd);
    }
    else if (parameters.role == LlRx) {// Receiver
        int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXO);
        //int fileSize = 0;
        //char* rcvFilename[BUFFER_SIZE] = {0};
        if (fd < 0) {
            printf("%s cannot be opened \n", filename);
            exit(-1);
        }

        int n = 0, llSize = 0;
        while (llSize != -1) {
            if (n == 50 && 0) {
                printf("-------------------NOISE NOW-------------------\n");
                sleep(2);
            }
            llSize = llread(buf);
            if (llSize > 0) {
                nbytes = write(fd, buf, llSize);
                printf("-- to file -> %d bytes\n", nbytes);
            }
            n++;

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
        }
        close(fd);
    }

    llclose(1);
}
