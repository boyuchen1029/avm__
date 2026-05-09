/**
 *******************************************************************************
 * @file     : cantest.h
 * @describe : Header for cantest.c file
 * 
 * @author : Linda.
 * @verstion : 0.1.0.
 * @date 20211220 0.1.0 Linda.
 *******************************************************************************
*/

#ifndef _CANTEST_H_
#define _CANTEST_H_

typedef struct
{
    float steer;
    float speed;
    char gear;
    char turningSignal;
    char keymode;
    int keypad;

    float meterage;
    int Mod;

    // unsigned char flagReceived;
}can_bus_info_t;

extern void write_can(int opc, int len, int * data);

extern void can_bus_thread_start(can_bus_info_t *canBus);

#endif //_CANTEST_H_