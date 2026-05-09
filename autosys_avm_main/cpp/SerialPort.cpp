#include "SerialPort.h"

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

SerialPort::SerialPort(const std::string& portName, int baudRate) {
    serialPort = open(portName.c_str(), O_RDWR);
    if (serialPort < 0) {
        std::cerr << "Error " << errno << " opening " << portName << ": " << strerror(errno) << std::endl;
    } else {
        memset(&tty, 0, sizeof tty);
        if (tcgetattr(serialPort, &tty) != 0) {
            std::cerr << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
        } else {
            cfsetospeed(&tty, baudRate);
            cfsetispeed(&tty, baudRate);

            tty.c_cflag |= (CLOCAL | CREAD);    // ignore modem controls, enable reading
            tty.c_cflag &= ~CSIZE;
            tty.c_cflag |= CS8;         // 8-bit characters
            tty.c_cflag &= ~PARENB;     // no parity bit
            tty.c_cflag &= ~CSTOPB;     // only need 1 stop bit
            tty.c_cflag &= ~CRTSCTS;    // no hardware flow control

            tty.c_lflag &= ~ICANON;
            tty.c_lflag &= ~ECHO;       // disable echo
            tty.c_lflag &= ~ECHOE;      // disable erasure
            tty.c_lflag &= ~ECHONL;     // disable new-line echo
            tty.c_lflag &= ~ISIG;       // disable interpretation of INTR, QUIT and SUSP
            tty.c_iflag &= ~(IXON | IXOFF | IXANY); // disable XON/XOFF flow control
            tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // disable any special handling of received bytes
            tty.c_oflag &= ~OPOST;      // prevent special interpretation of output bytes (e.g. newline chars)
            tty.c_oflag &= ~ONLCR;      // prevent conversion of newline to carriage return/line feed

            // fetch bytes as they become available
            tty.c_cc[VMIN] = 1;
            tty.c_cc[VTIME] = 5;        // 0.5 seconds read timeout

            if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
                std::cerr << "Error " << errno << " from tcsetattr" << std::endl;
            }
        }
    }
}

SerialPort::~SerialPort() {
    if (serialPort >= 0) {
        close(serialPort);
    }
}

std::string SerialPort::readString() {
    if (serialPort < 0) {
        return "";
    }

    char buf[256];
    memset(buf, 0, sizeof buf);
    int n = read(serialPort, buf, sizeof(buf));
    if (n < 0) {
        std::cerr << "Error reading: " << strerror(errno) << std::endl;
        return "";
    }
    return std::string(buf, n);
}

bool SerialPort::isOpen() const {
    return serialPort >= 0;
}
