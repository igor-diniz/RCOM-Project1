#include "link_layer.h"

#define FLAG 0x7E
#define ADDR_T 0X03
#define ADDR_R 0X01
#define SET 0X03
#define UA 0X07

#define BUF_SIZE 256

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} State;

int readFrame(unsigned char* frame, unsigned char expected, unsigned char addr);

int writeFrame(unsigned char ctrl, unsigned char addr);
