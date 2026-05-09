/**
 *******************************************************************************
 * @file     : init_UART.c
 * @describe : init uart dev and serial port setup.
 * 
 * @author : Sean.
 * @verstion : 0.1.0.
 * @date 20210809 0.1.0 Sean.
 *******************************************************************************
*/
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "../../include/user_control/init_UART.h"

static int uart_serial_port_setup( uart_pattern_t * uartPattern)
{
    printf("uartDevice = %s uartFd = %d nSpeed = %d nBits = %d nEvent = %s nStop = %d\n", 
                uartPattern->uartDevice, uartPattern->uartFd, uartPattern->nSpeed, uartPattern->nBits, (char*)uartPattern->nEvent, uartPattern->nStop);

    struct termios device_opt;
    memset(&device_opt, '\0', sizeof(device_opt));

    device_opt.c_cflag |= CLOCAL | CREAD;
    device_opt.c_cflag &= ~CSIZE;
    switch (uartPattern->nBits)
    {
        case 7:
            device_opt.c_cflag |= CS7;
        break;

        case 8:
            device_opt.c_cflag |= CS8;
        break;
    }

    switch (uartPattern->nEvent[0])
    {
        case 'O':
            device_opt.c_cflag |= PARENB;
            device_opt.c_cflag |= PARODD;
            device_opt.c_iflag |= (INPCK | ISTRIP);
        break;

        case 'E':
            device_opt.c_iflag |= (INPCK | ISTRIP);
            device_opt.c_cflag |= PARENB;
            device_opt.c_cflag &= ~PARODD;
        break;

        case 'N':
            device_opt.c_cflag &= ~PARENB;
        break;
    }

    switch (uartPattern->nSpeed)
    {
        case 2400:
            cfsetispeed(&device_opt, B2400);
            cfsetospeed(&device_opt, B2400);
        break;

        case 4800:
            cfsetispeed(&device_opt, B4800);
            cfsetospeed(&device_opt, B4800);
        break;

        case 9600:
            cfsetispeed(&device_opt, B9600);
            cfsetospeed(&device_opt, B9600);
        break;

        case 19200:
            cfsetispeed(&device_opt, B19200);
            cfsetospeed(&device_opt, B19200);
        break;

        case 115200:
            cfsetispeed(&device_opt, B115200);
            cfsetospeed(&device_opt, B115200);
        break;

        case 921600:
            cfsetispeed(&device_opt, B921600);
            cfsetospeed(&device_opt, B921600);
        break;

        default:
            cfsetispeed(&device_opt, B115200);
            cfsetospeed(&device_opt, B115200);
        break;
    }

    if (uartPattern->nStop == 1)
    {
        device_opt.c_cflag &= ~CSTOPB;
    }
    else if (uartPattern->nStop == 2)
    {
        device_opt.c_cflag |= CSTOPB;
    }

    device_opt.c_cc[VTIME] = 0;
    device_opt.c_cc[VMIN] = 10;
    tcflush(uartPattern->uartFd, TCIOFLUSH);
    if ((tcsetattr(uartPattern->uartFd, TCSANOW, &device_opt)) != 0)
    {
        puts("com set error\n");
        return -1;
    }
    else
    {
        puts("set done!\n");
        return 0;
    }
}

int init_uart_dev(uart_pattern_t * uartPattern)
{
    // int fdUART;

    if ((uartPattern->uartFd = open(uartPattern->uartDevice, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
    {
        puts("open USBRS232 fail.");
    }
    else
    {
        puts("open USBRS232 done.");
    }
    //printf("uartDevice = %s uartFd = %d nSpeed = %d nBits = %d nEvent = %s nStop = %d\n", uartPattern->uartDevice, uartPattern->uartFd, uartPattern->nSpeed, uartPattern->nBits, uartPattern->nEvent, uartPattern->nStop);

    if (uart_serial_port_setup(uartPattern) < 0)
    {
        puts("USB RS232 set_opt error");
    }
    return uartPattern->uartFd;
}