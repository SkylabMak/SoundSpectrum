#include "SerialComm.h"
#include <atomic>
#include <thread>
#include <mutex>

HANDLE hSerial; // Global handle for serial port
// std::atomic<std::string> storeData("");  // Shared buffer for latest data (atomic)
std::string storeData = "";          // Shared buffer for latest data
std::mutex dataMutex;                // Mutex to protect storeData
std::atomic<bool> keepRunning(true); // Flag to stop thread
std::thread writeThread;             // Background writing thread

void serialWriteLoop()
{
    std::cout << "run thread " << std::endl;
    while (keepRunning)
    {
        // std::string currentData = storeData.load();  // Get latest data safely (atomic)
        std::string currentData;

        { // Lock access to storeData
            std::lock_guard<std::mutex> lock(dataMutex);
            currentData = storeData;
        } // auto unlock()
        // std::cout << "data " << currentData << std::endl;
        if (!currentData.empty())
        {
            DWORD bytesWritten;
            BOOL status = WriteFile(hSerial, currentData.c_str(), currentData.length(), &bytesWritten, NULL);
            if (!status)
            {
                std::cout << " Error: Cannot send data!" << std::endl;
            }
            else
            {
                //  Wait for ACK
                char ackBuffer[10] = {0};
                DWORD bytesRead;
                bool ackReceived = false;
                auto startTime = std::chrono::steady_clock::now();

                //  Wait until ACK in 1 s
                while (!ackReceived)
                {
                    BOOL ackStatus = ReadFile(hSerial, ackBuffer, sizeof(ackBuffer) - 1, &bytesRead, NULL);

                    if (ackStatus && bytesRead > 0)
                    {                                // receive data
                        ackBuffer[bytesRead] = '\0'; // Ensure not null
                        std::string ackResponse(ackBuffer);

                        if (ackResponse.find("ACK") != std::string::npos)
                        {
                            // std::cout << "Received ACK from Arduino, sending next data..." << std::endl;
                            ackReceived = true;
                        }
                        else
                        {
                            // std::cout << "Unexpected response from Arduino: " << ackResponse << std::endl;
                        }
                    }

                    // check timeout in 1 sec
                    auto now = std::chrono::steady_clock::now();
                    double elapsedTime = std::chrono::duration<double>(now - startTime).count();
                    if (elapsedTime > 1.0)
                    {
                        std::cout << " No ACK received within 1 sec, retrying..." << std::endl;
                        break; // Exit waiting loop and retry sending
                    }

                    // std::this_thread::sleep_for(std::chrono::milliseconds(5));  // Avoid CPU overuse
                }
            }
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(500));  // Wait x ms before next write
    }
}

bool openSerialPort(const char *portName)
{
    hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    // hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 1, NULL, OPEN_EXISTING, 0, NULL); for share

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        std::cout << " Error: Cannot open serial port " << portName << "!" << std::endl;
        return false;
    }

    // test configure serial port
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        std::cout << " Error: Cannot get serial state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        std::cout << " Error: Cannot set serial state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    // Configure timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        std::cout << " Error: Cannot set timeouts!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    std::cout << " Serial port " << portName << " opened successfully!" << std::endl;
    //  Start background thread for sending serial data every 10ms
    keepRunning = true;
    writeThread = std::thread(serialWriteLoop);
    return true;
}

void writeToSerial(const std::string &data)
{
    // storeData.store(data);  // Update shared buffer
    std::lock_guard<std::mutex> lock(dataMutex); // Protect access to storeData
    storeData = data;
} // auto unlock()

void closeSerialPort()
{
    CloseHandle(hSerial);
    std::cout << " Serial port closed!" << std::endl;
}
