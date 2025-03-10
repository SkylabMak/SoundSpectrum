#include "SerialComm.h"

HANDLE hSerial;  // Global handle for serial port

bool openSerialPort(const char* portName) {
    hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cout << "❌ Error: Cannot open serial port " << portName << "!" << std::endl;
        return false;
    }

    // Configure serial port
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cout << "❌ Error: Cannot get serial state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cout << "❌ Error: Cannot set serial state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    // Configure timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cout << "❌ Error: Cannot set timeouts!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    std::cout << "✅ Serial port " << portName << " opened successfully!" << std::endl;
    return true;
}

void writeToSerial(const std::string& data) {
    DWORD bytesWritten;
    BOOL status = WriteFile(hSerial, data.c_str(), data.length(), &bytesWritten, NULL);
    if (status) {
        // std::cout << " Sent: " << data << std::endl;
    } else {
        std::cout << " Error: Cannot send data!" << std::endl;
    }
}

std::string readFromSerial() {
    char buffer[256] = { 0 };
    DWORD bytesRead;

    BOOL status = ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    if (!status || bytesRead == 0) {
        return "❌ No data received!";
    }

    buffer[bytesRead] = '\0';  // Ensure null termination
    return std::string(buffer);
}

void closeSerialPort() {
    CloseHandle(hSerial);
    std::cout << "✅ Serial port closed!" << std::endl;
}
