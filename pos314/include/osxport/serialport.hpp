/********************************************************************
          FILE:         SerialPort.h
   DESCRIPTION:         POS printer function definition(s)
 ********************************************************************/
#ifndef __SERIAL_PORT_HPP__
#define __SERIAL_PORT_HPP__
#include <unistd.h>
////////////////////////////////////////////////////////////////////
//  Method(s)
bool serialPortIsOpen();
ssize_t flushSerialData();
ssize_t closeSerialPort(void);
int getSerialFileDescriptor(void);
ssize_t readSerialData(char* bytes, size_t length);
ssize_t writeSerialData(const char* bytes, size_t length);
int openAndConfigureSerialPort(const char* portPath, int baudRate);
#endif //__SERIAL_PORT_HPP__
