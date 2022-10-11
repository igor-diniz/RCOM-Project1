#include "string.h"

#define BUFFER_SIZE 256

/**
 * Stuff a sequence of bytes.
 * @param buf byte array to be stuffed.
 * @param bufSize size of the buffer.
 * @return size of the new buffer.
*/
int stuff(const unsigned char *src, unsigned char* dest, int bufSize);

/**
 * De-Stuff a sequence of bytes.
 * @param buf byte array to be de-stuffed.
 * @param bufSize size of the buffer.
 * @return size of the new buffer.
*/
int deStuff(unsigned char *dest, int bufSize);