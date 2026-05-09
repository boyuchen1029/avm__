/**
 *******************************************************************************
 * @file     : display_location.h
 * @describe : 
 * 
 * @author : Linda.
 * @verstion : 0.1.0.
 * @date 20211229 0.1.0 Linda.
 *******************************************************************************
*/
//
// Created by AI611 on 2021/10/27.
//


#ifndef _DISPLAY_LOCATION_H_
#define _DISPLAY_LOCATION_H_

#include "../system.h"
extern float scope_W, scope_H;

#define ORIGIN_W 0
#define ORIGIN_H 0

// #define AVM_VIEW_S_W (SCREEN_W * 640.0 / 1920.0) //(0.36 * SCREEN_W) //691
// #define AVM_VIEW_L_W (SCREEN_W * 1280.0 / 1920.0)//(0.64 * SCREEN_W) //1228

#define AVM_VIEW_S_W (SCREEN_W * 0.33)
#define AVM_VIEW_L_W (SCREEN_W * 0.67)

// #define MIDDLE_3D_START_W (0.18 * SCREEN_W) //384
// #define MIDDLE_2D_START_W (0.32 * SCREEN_W) //576

#define MIDDLE_3D_START_W (SCREEN_W * 0.67) //384
#define MIDDLE_2D_START_W (SCREEN_W * 0.33) //576

#define HALF_W (0.50 * SCREEN_W) //640
#define HALF_H (0.50 * SCREEN_H) //360

#define AVM_STOP_GO_FBO_W 320.0
#define AVM_STOP_GO_FBO_H 320.0

#define AVM_BSD_FBO_W 320.0
#define AVM_BSD_FBO_H 320.0


// #define AVM_VIEW_S_H 274//223//(0.3 * SCREEN_H) //288
// #define AVM_VIEW_L_H 500//551//(0.7 * SCREEN_H) //432

// #define AVM_ODA_L_H 274//(0.3 * SCREEN_H) //288
// #define AVM_ODA_S_H 500//(0.7 * SCREEN_H) //432

// #define MIDDLE_W_START 186//(0.25 * SCREEN_W) //320
// #define MIDDLE_W 396//436//(0.50 * SCREEN_W) //640

// #define HALF_GAP_UP_H (0.55 * SCREEN_H) //396
// #define HALF_GAP_H (0.45 * SCREEN_H) //324

// #define FULL_ICON_S_W (0.1 * SCREEN_W) //76
// #define FULL_ICON_L_W (0.9 * SCREEN_W) //691

// #define APA_VIEW_S_W 316.0//316.0//(0.4 * SCREEN_W) //308
// #define APA_VIEW_L_W 454.0//(0.6 * SCREEN_W) //460

// #define APA_VIEW_S_H 270.0//(0.35 * SCREEN_H) //270
// #define APA_VIEW_L_H 504.0//(0.65 * SCREEN_H) //504

#define HALF_W (0.50 * SCREEN_W) //640
#define HALF_H (0.50 * SCREEN_H) //360

//set 2D and 3D AVM width and height
#define AVM_2D_W AVM_VIEW_S_W
#define AVM_2D_H SCREEN_H

//#define AVM_2D_FBO_W (AVM_2D_W * 1)
#define AVM_2D_FBO_USER autosys.system_info.seethrough_resolution_level

#define AVM_2D_FBO_W (508 + AVM_2D_FBO_USER * 508)
#define AVM_2D_FBO_H (AVM_2D_H * 0.8 + (AVM_2D_FBO_USER * 0.8 * AVM_2D_H))



#define AVM_3D_W AVM_VIEW_L_W
#define AVM_3D_H SCREEN_H

#define AVM_2D_SCOPE_W scope_W
#define AVM_2D_SCOPE_H scope_H

#define LINEWIDTH 4.0
#define LINEWIDTH_NOR (LINEWIDTH*2/400.0)
#define LINEWIDTH_NOR_2D (LINEWIDTH*2*4/400.0)
#define REVERSE_STA_STRIP (30.0/400.0)
#define REVERSE_STA_STRIP_AVM (100.0/400.0)


#endif //_DISPLAY_LOCATION_H_