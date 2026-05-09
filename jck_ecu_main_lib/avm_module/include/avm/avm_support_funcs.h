/**
 *******************************************************************************
 * @file     : avm_support_funcs.h
 * @describe : Header for avm_support_funcs.c file
 * 
 * @author : Bennit Yang..
 * @verstion : 0.1.0.
 * @date 20211104 0.1.0 Bennit Yang..
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/

//
// Created by AI611 on 2021/11/4.
//

#ifndef _AVM_SUPPORT_FUNCS_H_
#define _AVM_SUPPORT_FUNCS_H_
#include <sys/time.h>
#include "../system.h"
#include "user_control/ui.h"
#include "canbus.h"

extern float tire_rotate_angle(float speed, float fps);

extern void cal_tire_roate_angle(float * angle, float * speed, float fps, int gear);

extern void cal_tire_rudder_rotation_matrix(float * matrix, float rudder, int tireNum);

extern void car_rotate_360(avm_ui_info_t * uiInfo, float * theta, float fps);

extern void init_avmUiInfo(avm_ui_info_t * uiInfo);

extern void set_status_bar_para(avm_ui_info_t * uiInfo, can_bus_info_t * canInfo);

extern float calculate_time(struct timeval start, struct timeval end);


#endif //_AVM_SUPPORT_FUNCS_H_
