/**
 *******************************************************************************
 * @file : avm_2d_3d_para.h
 * @describe : Header for avm_2d_3d_para.c file
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20201220 0.1.0 Linda.
 *******************************************************************************
*/
#ifndef _AVM_2D_3D_PARA_H_
#define _AVM_2D_3D_PARA_H_

#include "user_control/ui.h"
#include "avm/camera_matrix.h"

extern void init_3d_avm_parameter(camera_para_t *para);

extern void reloading_3d_avm_fov(camera_para_t *para, int AVM_3D_WIDTH, int AVM_3D_HEIGHT);

extern void init_2d_avm_parameter(camera_para_t *para, int carSizeW, int carSizeH);

extern void init_seethrough_avm_parameter(camera_para_t *paraSee, camera_para_t *para2D);

extern void init_car_parameter(camera_para_t *para);

extern void set_3d_avm_parameter(avm_view_t view, avm_view_t viewOld, camera_para_t *para);

#endif //_AVM_2D_3D_PARA_H_
