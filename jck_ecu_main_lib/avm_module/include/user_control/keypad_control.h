/**
 *******************************************************************************
 * @file     : keypad.h
 * @describe : Header for keypad.c file
 * 
 * @author : Woody.
 * @verstion : 0.1.0.
 * @date 20211105 0.1.0 Woody.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/
//
// Created by AI611 on 2021/11/5.
//

#ifndef _KEYPAD_CONTROL_H_
#define _KEYPAD_CONTROL_H_

#include "user_command.h"
#include "avm/camera_matrix.h"
#include "ui.h"

#include "canbus.h"


extern void control_camara_parameter(user_command_mode_t user_command, camera_para_t * para);

extern void control_record(user_command_mode_t *user_command, can_bus_info_t canBus, can_bus_info_t canBusOld, avm_view_t view);

#endif //_KEYPAD_CONTROL_H_
