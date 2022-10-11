#include "link_layer.h"

#define FLAG 0x7E
#define ADDR_T 0X03
#define ADDR_R 0X01

#define SET 0X03
#define UA 0X07
#define DISC 0x0B
#define RR 0x05
#define REJ 0x01

#define BUF_SIZE 256

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    DATA,
    BCC2_OK,
    STOP
} State;


/**
 * Handle state changes for data read.
 * @param buf Last byte read.
 * @param expected Expected control character.
 * @param addr Expected address field.
 * @return 1 if the read is the frame is complete, zero otherwise.
*/
int stateStep(unsigned char buf, unsigned char expected, unsigned char addr);

/**
 * Write a frame to the port.
 * @param fd Serial port's file descriptor.
 * @param ctrl Control char to be written.
 * @param addr Address field to be written.
 * @return Number of bytes written.
*/
int writeCtrlFrame(int fd, unsigned char ctrl, unsigned char addr);

/**
 * Change the current state.
 * @param new_state New state.
*/
void setState(State new_state);

/**
 * Get the current state.
 * @return Current state.
*/
State getState();

/**
 * Get the data read from the last frame.
 * @return Data read from last frame.
*/
unsigned char* getData();
