#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include <windows.h>
#include <iostream>
#include <string>

bool openSerialPort(const char *portName);
void writeToSerial(const std::string &data);
std::string readFromSerial();
void closeSerialPort();

#endif // SERIALCOMM_H
