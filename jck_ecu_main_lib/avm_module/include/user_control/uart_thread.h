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
#ifndef _UART_THREAD_H_
#define _UART_THREAD_H_
#include "../../8368-P/Demo/canbus.h"
//int uart_serial_port_setup( uart_pattern_t * uartPattern);
extern void stm32_read_trans_thread_start(can_bus_info_t *canBus);

#endif