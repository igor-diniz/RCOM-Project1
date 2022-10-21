#include "link_layer.h"
#include <stdio.h>

#define FLAG 0x7E
#define ADDR_T 0X03
#define ADDR_R 0X01

#define SET 0X03
#define UA 0X07
#define DISC 0x0B
#define RR 0x05
#define REJ 0x01

#define I_CTRL_SHIFT 6
#define R_CTRL_SHIFT 7

#define BUF_SIZE 256

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    DATA,
    STOP
} State;


/**
 * Handle state changes for data read.
 * @param buf Last byte read.
 * @param expected Expected control character.
 * @param addr Expected address field.
 * @return 1 if the reading of the frame is complete, zero otherwise. Returns 2 for Rejected frames and 3 for duplicate frames.
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

/**
 * Test the control packet for its validity.
 * @param buf The receiving buffer.
 * @param expected Expected control byte.
 * @return returns 1 if the control is valid, 0 otherwise. Return 2 if the frame is a duplicate and -1 for error.
*/
int testCtrl(unsigned char buf, unsigned char expected);
