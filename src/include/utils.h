#include "string.h"

#define BUFFER_SIZE 256

/**
 * Stuff a sequence of bytes.
 * @param buf byte array to be stuffed.
 * @param bufSize size of the buffer.
*/
void stuff(unsigned char *buf, int bufSize);

/**
 * De-Stuff a sequence of bytes.
 * @param buf byte array to be de-stuffed.
 * @param bufSize size of the buffer.
*/
void deStuff(unsigned char *buf, int bufSize);