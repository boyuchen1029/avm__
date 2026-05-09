/**
 *******************************************************************************
 * @file : camara_3D_params.c
 * @describe : 
 *
 * @author : Bennit Yang
 * @verstion : 0.1.0.
 * @date 20211103 0.1.0 Bennit Yang.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/
//
// Created by AI611 on 2021/11/3.
//
#include "avm/camara_3D_params.h"

// #define PUBLISH_MTK_H774
// #ifdef PUBLISH_MTK_H774
//     #include "../../include/avm/display_location_h774.h"
// #else
//     #include "../../include/display_location_h870.h"
// #endif

// static camera_3D_params view3DCamParams[VIEW_END] = {0};

// static int viewPlaneW[VIEW_END] = {
//     0, 0, 0, 0,                         0,                      // 0 ~ 4
//     0, 0, 0, 0,                         0,                      // 5 ~ 9
//     0, 0, 0, 0,                         0,
//     0, 0, 0, SV_MIDDLE_SPLIT_SMALL_W,   SV_MIDDLE_SPLIT_SMALL_W,
//     0, 0, 0, 0,                         0,                      // 20 ~ 24
//     0, 0, 0, SV_3D_BOTTOM_full_W,       SV_3D_BOTTOM_full_W,    // 25 ~ 29
//     0, 0                                                 //  30 ~ 31
// };

// static int viewPlaneH[VIEW_END] = {
//     0, 0, 0, 0,                         0,
//     0, 0, 0, 0,                         0,
//     0, 0, 0, 0,                         0,
//     0, 0, 0, SV_MIDDLE_SPLIT_SMALL_H,   SV_MIDDLE_SPLIT_SMALL_H,
//     0, 0, 0, 0,                         0,                      // 20 ~ 24
//     0, 0, 0, SV_3D_BOTTOM_full_H,       SV_3D_BOTTOM_full_H,    // 25 ~ 29
//     0 ,0                                              // 30 ~ 31
// };

// void init_camera_3D_params(void)
// {
//     int ix;
//     for(ix = 0; ix < VIEW_END; ix++)
//     {
//         view3DCamParams[ix].planeW = viewPlaneW[ix];
//         view3DCamParams[ix].planeH = viewPlaneH[ix];
//     }
// }