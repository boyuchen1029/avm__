/**
 *******************************************************************************
 * @file : system.h
 * @describe : 
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211112 0.1.0 Linda.
 *******************************************************************************
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <stdio.h>

//
#define IMAX8 0
#define MTK2712 1
#define SPHE8368_P 2
#define SPHE6702 3
#define NT98690 4

//set input image format
#define YUYV 0
#define UYVY 1
#define VIV 2

#define DEBUG
#define DEBUG_FPS
// #define READ_FILE
// #define MSAA
#define FPS_OPTIMISE_V0        1

#define PLAFORM SPHE8368_P
#define READ_OFFLINE_SHADER 1
#define __CRC_SYSTEM_WRITE__
 	#ifdef __CRC_SYSTEM_WRITE__
		#define binary_crc_start_key 33
		#define binary_crc_end_key 67
 	#endif

extern int g_PANEL_WIDTH;
extern int g_PANEL_HEIGHT;

extern float yReverse;//for GL yReverse; 1.0 = positiv, -1.0 = negative;
extern int touch_write_flag;
extern int GO_to_QT_flag;
extern int g_enable_360system;
extern int g_reset_360rotateflag;
extern  int flag_360_finsih;
extern int g_onlyonetime_360rotateflag;
extern int g_3D_carModel_alpha_value;
extern float current_carAngle;
extern int g_reset_540sysyem;
extern int g_CarModel_ever_do_rotate_ones[50];
extern int g_avm_BB_PID_Latency;
extern int g_avm_camera_flag;


/*Eight Touch*/
typedef struct 
{
    int busy;
    int cur_id;
    int net_id;
	int director;
	int moving;
	int block;
	float processTime;
	int interrupt;
	/* level2 */
	int level;
	float cur_angle;
	float total_angle;
	int signal;
    /* data */
}stEightTouch;
extern stEightTouch eight;




#define UNION_TEST

#ifdef UNION_TEST
	#define dynamic_line_union_test 0
	#define seethrough_demo_withoutUART 0
	#define save_current_screen_framebuffer 1
	#define save_FXAA_framebuffer 0

	#ifdef save_current_screen_framebuffer
		#define scrFileName "newconfig.rgba"
	#endif
#endif


#ifndef DEBUG
	#define AVM_LOGI(...)
	#define AVM_LOGE(...)
	#define AVM_LOGV(...)
	#define AVM_LOGD(...)
	#define AVM_LOGW(...)
#else
	#define AVM_LOGI(...) printf("\033[1m\033[33m[AutoSys] \33[0m" __VA_ARGS__)
	#define AVM_LOGE(...) perror("\033[1m\033[31m[AutoSys][ERROR] \33[0m"__VA_ARGS__)
#endif

#define FRAGSHADER  "../fsh/default_frag.c"
#define VERTSHADER "../vsh/default_vertex.c"

#define AVM_VALUE(...) #__VA_ARGS__
#define LOG_RED(str) "\033[31m"str"\033[0m"

#define PING AVM_LOGI("---- PING LINE %d ----\n", __LINE__)

#define LUXGEN_URX 0
#define CROSS 1
#define LUXGEN_U6 2
#define HUANSU_S6 3

#include "define.h"

#if(CAR_MODEL == CROSS) //s3
	#define CAR_WIDTH 178.3 //178.0		//mm
	#define CAR_LONG_HALF (455.1/2) //(456/2)		//mm
	#define CAR_WHEELBASE 262.0 //272.0
	#define CAR_WHEELTOREAR 96.0// 272.0  

	#define CAR_BODY "car_model/cross/Car_body_cross.h"
    #define TIRE_LB "car_model/cross/tire_LB_cross.h"
    #define TIRE_RB "car_model/cross/tire_RB_cross.h"
    #define TIRE_LF "car_model/cross/tire_LF_cross.h"
    #define TIRE_RF "car_model/cross/tire_RF_cross.h"
    #define VERTICE_NUM "car_model/cross/vertice_num_cross.h"
#elif(CAR_MODEL == LUXGEN_URX)
	#define CAR_WIDTH 182.6 //178.0		//mm
	#define CAR_LONG_HALF (472.5/2) //(456/2)		//mm
	#define CAR_WHEELBASE 272.0 //272.0
	#define CAR_WHEELTOREAR 100.25// 272.0
	#define CAR_REAR_TRACK 157.8

	// #define CAR_BODY "car_model/luxgen_urx/car_body_luxgen_urx.h"
    // #define TIRE_LF "car_model/luxgen_urx/tire_LF_luxgen_urx.h"
    // #define TIRE_LB "car_model/luxgen_urx/tire_LB_luxgen_urx.h"
    // #define TIRE_RF "car_model/luxgen_urx/tire_RF_luxgen_urx.h"
    // #define TIRE_RB "car_model/luxgen_urx/tire_RB_luxgen_urx.h"
    // #define VERTICE_NUM "car_model/luxgen_urx/vertice_num_urx.h"
	// #define Glass_h "car_model/luxgen_urx/glass.h"

#elif(CAR_MODEL == LUXGEN_U6)
	#define CAR_WIDTH 182.5 //178.0		//mm
	#define CAR_LONG_HALF (463.0/2) //(456/2)		//mm
	#define CAR_WHEELBASE 272.0 //272.0
	#define CAR_WHEELTOREAR 100.25// 272.0

	#define CAR_BODY "car_model/luxgen_urx/car_body_luxgen_urx.h"
    #define TIRE_LF "car_model/luxgen_urx/tire_LF_luxgen_urx.h"
    #define TIRE_LB "car_model/luxgen_urx/tire_LB_luxgen_urx.h"
    #define TIRE_RF "car_model/luxgen_urx/tire_RF_luxgen_urx.h"
    #define TIRE_RB "car_model/luxgen_urx/tire_RB_luxgen_urx.h"
    #define VERTICE_NUM "car_model/luxgen_urx/vertice_num_urx.h"
#elif(CAR_MODEL == HUANSU_S6)
	#define CAR_WIDTH 182.6 //178.0		//mm
	#define CAR_LONG_HALF (472.5/2) //(456/2)		//mm
	#define CAR_WHEELBASE 272.0 //272.0
	#define CAR_WHEELTOREAR 100.25// 272.0
	#define CAR_REAR_TRACK 157.8
	
    #define CAR_BODY "car_model/huansu_s6/car_body_huansu_s6.h"
    #define TIRE_LF "car_model/huansu_s6/tire_LF_huansu_s6.h"
    #define TIRE_LB "car_model/huansu_s6/tire_LB_huansu_s6.h"
    #define TIRE_RF "car_model/huansu_s6/tire_RF_huansu_s6.h"
    #define TIRE_RB "car_model/huansu_s6/tire_RB_huansu_s6.h"
    #define VERTICE_NUM "car_model/huansu_s6/vertice_num_s6.h"
#endif

extern void mod_exit();

// extern int camera_mode;
extern int mod_recall_flag;
// extern int mod_alert[4];
// extern int bsd_alert[2];
// extern int oda_alert[2];
// extern int ldw_alert[2];
extern char * EVSTable_path[4];

#endif //_SYSTEM_H_
