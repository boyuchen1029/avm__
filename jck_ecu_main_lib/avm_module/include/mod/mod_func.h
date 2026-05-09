#pragma once
#ifndef _MOD_FUNC_H_
#define _MOD_FUNC_H_

#include "mod_para.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef unsigned char uint8;
typedef double float64;

#define SOURCE_ROOT "E:\\MOD\\recordings\\s3\\examples\\"
#define SOURCE_FOLDER "car_wash1\\"
#define SOURCE_MOD_DIR "2d_avm\\2d_dbg.yuv"

#define SIMULATION_FOLDER "simulation_20fps_mod2_v004_tttest\\"
#define SIMULATION_FPS 20.0f

typedef struct
{
	int x1, x2;
	int y1, y2;
	int id;
}gl_rect_t;

typedef struct
{
	short x[1];
	short y[1];
	int area;		// �`�ƶq
	int flag;		// �ثe�j�M���X��
	int num;		// �s�սs��
	int center[2];
	int lumaMean;
	int lumaMeanLast;
}mod_group_t;

typedef struct
{
	int up;
	int down;
	int left;
	int right;
}extreme_point_t;

typedef struct mod_t
{
	int frameW;
	int frameH;
	int carWidth;
	int carHeight;

	double blurMask[5];

	uint8* input;
	float64* imgAvg_float;
	uint8* imgAvg;
	uint8* imgGray;
	uint8* imgBlur;
	uint8* imgDiff;
	uint8* imgThresh;
	uint8* imgGroup;
	uint8* imgEven;
	uint8* imgOdd;

	extreme_point_t extremePoint[GROUP_NUM_THRESH];
	int exPNum;

	mod_group_t * group;
	short* groupX;
	short* groupY;
	uint8 flagImgSaving;
	uint8 flagTarget;
	
	int detectPoly[4][4][2];
	int modDetectSW[4];

	int groupNumMax;
	int motionThresh;
	int groupAreaThresh;

	gl_rect_t * glRect;
	int glRectNum;
}mod_t;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
extern void mod_func_init(mod_t * mod_, int carSizeW, int carSizeH);
extern void mod_func(mod_t * mod_);
extern void mod_release(mod_t * mod_);
extern void open_log(char* folder);
#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _MOD_FUNC_H_