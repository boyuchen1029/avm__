/**
 *******************************************************************************
 * @file	 : ui.h
 * @describe : This file contains the user control command defines of the application.
 * 
 * @author : Woody.
 * @verstion : 0.1.0.
 * @date 20210803 0.1.0 Woody.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/
#ifndef _UI_H_
#define _UI_H_

#include "../../system.h"
#include "ui_struct.h"

typedef enum
{
	CAM_VIEW_FISHEYE,
	CAM_VIEW_CORRECT,
	CAM_VIEW_FISHEYE_TRANS,
	CAM_VIEW_ADAS,
	CAM_VIEW_FISHEYE_CYLINDRICAL
}cam_view_t;

typedef struct
{
    float x;
    float y;
    float w;
    float h;
}position_item_t;

typedef enum
{
	CAM_NUM_FRONT = 0,
	CAM_NUM_BACK,
	CAM_NUM_LEFT,
	CAM_NUM_RIGHT,
	CAM_NUM_LEFT_BSD,
	CAM_NUM_RIGHT_BSD,
}cam_num_t;

typedef enum
{
	oda_left = 0,
	oda_right,
	bsd_left,
	bsd_right,
	ldw_left,
	ldw_right,
};

//AVM UI ICON information
typedef struct
{
	int circleStart;
	int seeThrough;
	int adasMode;
	int mod;
	int avm2dFBO;

	avm_view_t view;
	avm_view_t viewold;

	unsigned char flag2davm;

	ui_struct_t ui_state;

}avm_ui_info_t;

typedef enum
{
	GL_COLOR_RED = 0,
	GL_COLOR_YELLOW,
	GL_COLOR_GREEN,
	GL_COLOR_ORAHGE,
	GL_COLOR_GRAY,
}reverse_line_color;

typedef enum
{
	VAO_3D_AVM = 0,
	VAO_2D_AVM
}VAO_3D_2D_AVM_t;

typedef struct
{
	int front;
	int back;
	int left;
	int right;
	int leftBSD;
	int rightBSD;
}trans_camera_t;

#endif