/**
 *******************************************************************************
 * @file     : canbus_control.h
 * @describe : Header for canbus_control.c file
 *
 * @author : Linda.
 * @verstion : 0.1.0.
 * @date 20220104 0.1.0 Linda.
 *******************************************************************************
*/

//
// Created by AI611 on 2021/11/4.
//

#ifndef _CANBUS_CONTROL_H_
#define _CANBUS_CONTROL_H_

#include "user_control/ui.h"
#include "canbus.h"


extern void avm_control(avm_ui_info_t * uiInfo, can_bus_info_t * can, user_command_mode_t *user_command, camera_para_t *camerapara);
extern void avm_panel_move(avm_ui_info_t *UiInfo,camera_para_t *camerapara,int mode, float theta,float phi);
#endif //_AVM_SUPPORT_FUNCS_H_
