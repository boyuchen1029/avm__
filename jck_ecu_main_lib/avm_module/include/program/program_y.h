/**
 *******************************************************************************
 * @file : program_y.h
 * @describe : Header of program_y.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20220418 0.1.1 Linda.
 *******************************************************************************
*/

#ifndef _PROGRAM_Y_H_
#define _PROGRAM_Y_H_
#include "avm/GLHeader.h"

extern void init_rgba2y_program(void);

extern void open_gl_init_rgba2y(void);

extern void draw_rgba2y(const GLuint FramebufDefault,int avm_2d_width);

#endif //_PROGRAM_UYVY_H_
