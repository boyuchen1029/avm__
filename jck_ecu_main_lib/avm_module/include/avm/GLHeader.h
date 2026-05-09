/**
 *******************************************************************************
 * @file     : GLHeader.h
 * @describe : 
 * 
 * @author : Woody.
 * @verstion : 0.1.0.
 * @date 20211108 0.1.0 Woody.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/

#ifndef _GLHEADER_H_
#define _GLHEADER_H_

//opengl define
#include "../../system.h"

#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>

#if(PLAFORM != SPHE8368_P && PLAFORM != NT98690)
#include <GLES3/gl3ext.h>
#endif

#define CAR_TEXTURE_W  1024
#define CAR_TEXTURE_H  1024 

#define ALPHA_W 2500
#define ALPHA_H 2500

#define VAO_NUM 8//camera *2

#define FISH_CORRECTION_W 400.0
#define FISH_CORRECTION_H 274.0

#define KPH 0.2778f				 // 1000 / 60 / 60
#define PERIMETER_URX 2.017		 // M
#define CIRCLE_DEGREE 360.0

typedef enum
{
    FRONT_CAMERA = 4, //program_main
    BACK_CAMERA = 5,
    LEFT_CAMERA = 6,
    RIGHT_CAMERA = 7,
    MAIN_ALPHA = 8,
    MAIN_COLOR = 9,
    MAIN_COLOR2 = 10,

    CAR_TEXTURE = 11,   //program_car
    CAR_GLOSSISESS = 12,
    CAR_SPECULAR = 13,

    AVM_2D_TEXTURE = 14,   //2D avm fbo
    RGBA_TEXTURE = 15, //program_rgba

    AVM_FBO = 16,  //rgba to uyvy fbo
    SEE_THROUGH = 17,  //see through - fbo3 fbo
    SEE_THROUGH_RESULT = 18,  //see through - fbo_seethrough fbo
    RGBA_2_Y = 19,  //mod(rgba to y) fbo

    FORMAT_TEX = 20,
    FXAA_TEXTURE = 21,
    FBO_ADAS = 22,
    FBO_ADAS_Y = 23,

    LEFT_BSD_CAMERA = 24,
    RIGHT_BSD_CAMERA = 25,

    MSAA_FBO = 26,
    CAR_NORMAL = 26,
    CAR_SKY = 27,
    CAR_SHADOW = 28,

    ADAS_IMS = 29,

    SEE_THROUGH_CROPE = 30,

    CAR_MODEL_OIT = 31,
    UI_TEST1_enable = 32,
    UI_TEST2_enable,
    UI_TEST3_enable,
    UI_TEST4_enable,
    UI_TEST5_enable,
    UI_TEST6_enable,
    UI_TEST7_enable = 40,

    UI_TEST1_disable,
    UI_TEST2_disable,
    UI_TEST3_disable,
    UI_TEST4_disable,
    UI_TEST5_disable,
    UI_TEST6_disable = UI_TEST5_disable + 10,

    

    // FBO_FISH = 20,  //stop&go fbo
    // FBO_BSD = 21,  //bsd fbo

}texture_unit_t;



#endif //_GLHEADER_H_
