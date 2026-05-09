/**
 *******************************************************************************
 * @file : program_TSAA.h
 * @describe : Header of program_TSAA.c.
 *
 * @author : Alfred
 * @verstion : 0.1.0.
 * @date 20231123 0.1.1 Alfred.
 *******************************************************************************
*/

//#123

#ifndef _PROGRAM_TSAA_H_
#define _PROGRAM_TSAA_H_

#include "../../../setup/AVM_STRUCT.h"

extern void init_FXAA_program(void);
extern void init_FXAA_opengl();
extern void draw_FXAA(float *glViewport, bool isEnableFxaa);
extern void trans_FXAA_texture(GLuint raw_texture);
extern int  check_FXAA_frame();
extern int get_FXAA_ready_to_draw();

#endif //_PROGRAM_TSAA_H_
