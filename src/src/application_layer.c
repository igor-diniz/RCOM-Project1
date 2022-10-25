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

void configureDataPackage(unsigned char* buf, int pkgIndex){
    unsigned char auxBuffer[BUF_SIZE] = {0};
    auxBuffer[0] = 1;                        // C
    auxBuffer[1] = pkgIndex % 255;           // N
    auxBuffer[2] = MAX_CHUNK_SIZE / 256;     // L1
    auxBuffer[3] = MAX_CHUNK_SIZE % 256;     // L2
    memcpy(&auxBuffer[4], buf, MAX_CHUNK_SIZE);
    memcpy(buf, auxBuffer, MAX_CHUNK_SIZE + 4);
}

int writeCtrl(int fileSize, const char* fileName, int start) {
    unsigned char buf[BUFFER_SIZE] = {0};
    if (start == 1) buf[0] = 2;
    else buf[0] = 3;
    buf[1] = 0;     // fileSize
    int numOct = 0, s = fileSize;
    while (s > 0) {
        numOct++;
        s = s >> 8;
    }
    buf[2] =  numOct;

    int i = 0;
    for (; i < numOct; i++){
        buf[3 + i] = 0xff & (fileSize >> (8 * (numOct - i - 1)));
    }
    i += 3;
    buf[i] = 2;
    buf[i + 1] = 1;     // fileSize
    numOct = strlen(fileName);
    buf[i + 2] =  numOct;

    i += 3;
    int j = 0;
    for (j = 0; j < numOct; j++){
        buf[i + j] = fileName[j];
    }
    return llwrite(buf, i + j);
}

void applicationTx(const char* filename) {
    // Opening {filename} and verifying if any error occurred
    int nbytes = 0, seqN = 0, written = 0;
    unsigned char buf[BUF_SIZE];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("%s cannot be opened \n", filename);
        exit(-1);
    }
    
    int fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if (writeCtrl(fileSize, filename, 1) == -1) {
        printf("Connection timed out.\n");
        return;
    }
    
    int i = 0;
    while ((nbytes = read(fd, buf, MAX_CHUNK_SIZE)) != 0){ // zero indicates end of file
        if(nbytes == -1){
            printf("An error occurred in the reading of the %s", filename);
        }
        i++;
        configureDataPackage(buf, seqN);
        seqN++;
        if (llwrite(buf, nbytes + 4) == -1) break;
        written += nbytes;
        printBar(written, fileSize);
    }

    if (writeCtrl(fileSize, filename, 0) == -1) {
        printf("Connection timed out.\n");
        return;
    }
    close(fd);
}

void receiveCtrl(unsigned char* buf, int llSize, int* fileSize, char* rcvFilename) {
    int i = 1;
    while (i < llSize) {
        if (buf[i] == 0) { // size
            int l = (int)buf[i + 1];
            i += 2;
            for (int k = l - 1; k >= 0; k--) {
                *fileSize += (buf[i] << (8 * k));
                i++;
            }
        }
        else if (buf[i] == 1) { // file name
            int l = (int)buf[i + 1];
            i += 2;
            for (int k = 0; k < l; k++) {
                rcvFilename[k] = buf[i];
                rcvFilename[k + 1] = '\0';
                i++;
            }
        }
        else i++;
    }
}

void applicationRx(const char* filename) {
    unsigned char buf[BUF_SIZE];
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXO);
    int fileSize = 0, written = 0, prev_idx = -1;
    char rcvFilename[BUFFER_SIZE] = {0};
    if (fd < 0) {
        printf("%s cannot be opened \n", filename);
        exit(-1);
    }

    int llSize = 0;
    while (llSize != -1) {
        llSize = llread(buf);
        if (llSize > 0) {
            if (buf[0] == 2) { // start
                receiveCtrl(buf, llSize, &fileSize, rcvFilename);
            }
            else if (buf[0] == 3) { // end
                char rcvFilenameEnd[BUFFER_SIZE] = {0};
                int fileSizeEnd = 0;
                receiveCtrl(buf, llSize, &fileSizeEnd, rcvFilenameEnd);
                if (fileSize != fileSizeEnd || strcmp(rcvFilename, rcvFilenameEnd) != 0) {
                    printf("An error occurred.\n");
                }
            }
            else if (buf[0] == 1) {
                int idx = buf[1];
                if (idx != (prev_idx + 1) % 255) {
                    printf("An error occured.\n");
                    break;
                }
                prev_idx++;
                int num = buf[2] * 256 + buf[3];
                int nbytes = write(fd, &buf[4], llSize - 4);
                written += nbytes;
                printBar(written, fileSize);
            }
        }
    }
    close(fd);
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    if (setUp(serialPort, role, baudRate, nTries, timeout) == -1) {
        printf("Couldn't establish connection.\n");
        llclose(1);
        return;
    }

    // Transmitter
    if (parameters.role == LlTx) {
        applicationTx(filename);
    }
    else if (parameters.role == LlRx) {// Receiver
        applicationRx(filename);
    }

    llclose(1);
}
