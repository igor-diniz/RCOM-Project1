// Application layer protocol header.
// NOTE: This file must not be changed.

#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

/**
 * @brief writes the data package
 * 
 * @param buf 
 * @param pkgIndex 
 * @param size 
 */
void configureDataPackage(unsigned char* buf, int pkgIndex, int size);

/**
 * @brief writes the control package
 * 
 * @param fileSize 
 * @param fileName 
 * @param start 
 * @return int 
 */
int writeCtrl(int fileSize, const char* fileName, int start);

// Application layer main function.
// Arguments:
//   serialPort: Serial port name (e.g., /dev/ttyS0).
//   role: Application role {"tx", "rx"}.
//   baudrate: Baudrate of the serial port.
//   nTries: Maximum number of frame retries.
//   timeout: Frame timeout.
//   filename: Name of the file to send / receive.

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename);

#endif // _APPLICATION_LAYER_H_
