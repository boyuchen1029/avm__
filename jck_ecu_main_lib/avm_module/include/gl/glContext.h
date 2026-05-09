/**
 *******************************************************************************
 * @file : glShaderUtils.h
 * @describe : Header of glShaderUtils.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211220 0.1.0 Linda.
 *******************************************************************************
*/
#ifndef _GLCONTEXT_H_
#define _GLCONTEXT_H_

typedef enum
{
    SEETHROUGH,
    CARMODEL,
    LINE,
    BLUR,
}gl_program;


typedef enum
{
    FBO3_TEXTURE,
    FBO_SEETHROUGH_TEXTURE,
}SEETHROUGH_TEXTURE;

typedef enum
{
    PGL_COLORBAR_STYLE0,
    PGL_COLORBAR_STYLE1,
    PGL_COLORBAR_STYLE2,
    PGL_COLORBAR_STYLE3,
    PGL_COLORBAR_STYLE4,
}PGL_TEXTURE;

typedef enum
{
    FISHEYE_FRONT_BLUR_TEXTURE,
    FISHEYE_REAR_BLUR_TEXTURE,
    FISHEYE_LEFT_BLUR_TEXTURE,
    FISHEYE_RIGHT_BLUR_TEXTURE,
}BLUR_TEXTURE;


typedef enum
{
    FBO_4TO1,
    FBO_FXAA,
    FBO_BLUR_TMP,
    FBO_BLUR_1,
    FBO_BLUR_2,
    FBO_BLUR_3,
    FBO_BLUR_4,
    FBO_STATIC_CAR_MODEL_0,
    FBO_STATIC_CAR_MODEL_1,
    FBO_STATIC_CAR_MODEL_2,
}FBO_ID;


#endif //_GLCONTEXT_H_