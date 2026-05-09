#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <string>
#include <termios.h>

class SerialPort {
public:
    SerialPort(const std::string& portName, int baudRate);
    ~SerialPort();
    std::string readString();
    bool isOpen() const;

private:
    int serialPort;
    struct termios tty;
};

#endif // SERIALPORT_H
