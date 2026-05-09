/**
 *******************************************************************************
 * @file     : init UART.h
 * @describe : Header for init_UART.c file.
 * 
 * @author : Sean.
 * @verstion : 0.1.0.
 * @date 20210803 0.1.0 Sean.
 *******************************************************************************
*/
#ifndef _INIT_UART_H_
#define _INIT_UART_H_

typedef struct _uart_pattern_t
{
    char        *uartDevice;
    int           uartFd; 
    int           nSpeed; 
    int           nBits; 
    char        nEvent[1]; 
    int           nStop;
}uart_pattern_t;

//int uart_serial_port_setup( uart_pattern_t * uartPattern);
extern int init_uart_dev( uart_pattern_t * uartPattern);

#endif