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

void configureDataPackage(unsigned char* buf, int pkgIndex, int size){
    unsigned char auxBuffer[BUF_SIZE] = {0};
    auxBuffer[0] = 1; //always 1 cause its a data package   // C
    auxBuffer[1] = pkgIndex % 255; //seqN mod with 255      // N
    auxBuffer[2] = size / 256;                              // L1
    auxBuffer[3] = size % 256;                              // L2
    memcpy(&auxBuffer[4], buf, MAX_CHUNK_SIZE); //buf tem o conteudo lido do ficheiro
    memcpy(buf, auxBuffer, MAX_CHUNK_SIZE + 4); 
    // buf tem agr os 4 parametros iniciais do pacote de dados + o conteudo lido do ficheiro
    // logo, buf é o nosso pacote de dados completo
}

int writeCtrl(int fileSize, const char* fileName, int start) {
    unsigned char buf[BUFFER_SIZE] = {0};
    if (start == 1) buf[0] = 2;
    else buf[0] = 3;
    buf[1] = 0;     // fileSize -> mandatory
    int numOct = 0, s = fileSize;
    while (s > 0) {
        numOct++;
        s = s >> 8;
    }
    buf[2] =  numOct; //specifies the size of the next field

    int i = 0;
    //the for loopfills V1 with the parameter value
    for (; i < numOct; i++){
        buf[3 + i] = 0xff & (fileSize >> (8 * (numOct - i - 1)));
        //0x1234 -> buf[3]=12 e buf[4]=34
    }
    i += 3; 
    buf[i] = 1;     // fileName -> optional
    numOct = strlen(fileName);
    buf[i + 1] =  numOct; //specifies the size of the next field

    i += 2;
    int j = 0;
    //the for loopfills V2 with the parameter value
    for (j = 0; j < numOct; j++){
        buf[i + j] = fileName[j];
    }
    return llwrite(buf, i + j); 
    //returns the number of bytes written to the file
}

void applicationTx(const char* filename) {
    // Opening {filename} and verifying if any error occurred
    int nbytes = 0, seqN = 0, written = 0;
    unsigned char buf[BUF_SIZE];
    //fd é o ficheiro de onde vamos ler os dados para enviar ao recetor
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("%s cannot be opened \n", filename);
        exit(-1);
    }
    
    int fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // sinalizar o início da transferência do ficheiro
    if (writeCtrl(fileSize, filename, 1) == -1) {
        printf("Connection timed out.\n");
        return;
    }
    
    // After signaling the start of file transfer successfully..
    // .. let's start send data:
    // o while loop termina qd já não houver nada a enviar ou se ocorrer um erro no envio
    while ((nbytes = read(fd, buf, MAX_CHUNK_SIZE)) != 0){ // zero indicates end of file
        if(nbytes == -1){
            printf("An error occurred in the reading of the %s", filename);
        }
        configureDataPackage(buf, seqN, nbytes); //seqN starts at 0
        //buf é o nosso pacote de dados completo pronto a ser enviado para o ficheiro
        seqN++;
        if (llwrite(buf, nbytes + 4) == -1) {
            printf("Connection timed out.\n");
            return;
        }
        //depois de o pacote ser enviado..
        written += nbytes;
        //..damos print de todo o conteúdo já enviado
        printBar(written, fileSize);
    }

    //sinalizar o fim da transmissão (END) -> com a mesma informação do pacote de inicio de transmissão)
    if (writeCtrl(fileSize, filename, 0) == -1) {
        printf("Connection timed out.\n");
        return;
    }
    close(fd); //o ficheiro de onde lemos os dados a enviar é fechado
}

void receiveCtrl(unsigned char* buf, int llSize, int* fileSize, char* rcvFilename) {
    int i = 1;
    while (i < llSize) {
        if (buf[i] == 0) { // size (T1)
            int l = (int)buf[i + 1]; //length do campo seguinte (L1)
            i += 2;
            for (int k = l - 1; k >= 0; k--) {
                *fileSize += (buf[i] << (8 * k));
                i++;
            }
            //filesize => (V1)
            i--;
        }
        else if (buf[i] == 1) { // file name (T2)
            int l = (int)buf[i + 1]; //length do campo seguinte (L2)
            i += 2;
            for (int k = 0; k < l; k++) {
                rcvFilename[k] = buf[i];
                rcvFilename[k + 1] = '\0';
                i++;
            }
            //rcvFilename => (V2)
            i--;
        }
        else i++;
    }
}

void applicationRx(const char* filename) {
    unsigned char buf[BUF_SIZE];
    // ficheiro para onde vamos escrever os dados enviados pelo emissor
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXO);
    int fileSize = 0, written = 0, prev_idx = -1;
    char rcvFilename[BUFFER_SIZE] = {0};
    if (fd < 0) {
        printf("%s cannot be opened \n", filename);
        exit(-1);
    }

    int llSize = 0;
    while (llSize != -1) { // enquanto houver dados para ler
        llSize = llread(buf);
        if (llSize > 0) {
            if (buf[0] == 2) { // start (é um pacote de controlo)
                receiveCtrl(buf, llSize, &fileSize, rcvFilename);
            }
            else if (buf[0] == 3) { // end (é um pacote de controlo)
                char rcvFilenameEnd[BUFFER_SIZE] = {0};
                int fileSizeEnd = 0;
                receiveCtrl(buf, llSize, &fileSizeEnd, rcvFilenameEnd);
                //compara se os pacotes de controlo end e start tem a mesma informação de file size e name
                if (fileSize != fileSizeEnd || strcmp(rcvFilename, rcvFilenameEnd) != 0) {
                    printf("An error occurred.\n");
                }
            }
            else if (buf[0] == 1) { // é um pacote de dados
                int idx = buf[1]; //num de seq
                if (idx != (prev_idx + 1) % 255) { //verifica se não é perdido nenhum pacote
                    printf("An error occurred.\n");
                    break;
                }
                prev_idx++;
                int num = buf[2] * 256 + buf[3]; //num de octetos do campo de dados (que começa na pos 4)
                int nbytes = write(fd, &buf[4], num); //escreve para o file os dados do pacote
                written += nbytes;
                printBar(written, fileSize); 
            }
        }
    }
    close(fd); // fechamos o ficheiro para onde escrevemos os dados enviados pelo emissor
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
    // Receiver
    else if (parameters.role == LlRx) {
        applicationRx(filename);
    }

    llclose(1);
}
