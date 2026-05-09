/**
 *******************************************************************************
 * @file     : camera_3D_params.h
 * @describe : Header for camera_3D_params.c file
 * 
 * @author : Bennit Yang.
 * @verstion : 0.1.0.
 * @date 20211103 0.1.0 Bennit Yang.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/
//
// Created by AI611 on 2021/11/3.
//

#ifndef _CAMARA_3D_PARAMS_H_
#define _CAMARA_3D_PARAMS_H_


typedef struct
{
    int planeW;
    int planeH;
}camera_3D_params;


extern void init_camera_3D_params(void);

#endif //_CAMARA_3D_PARAMS_H_
