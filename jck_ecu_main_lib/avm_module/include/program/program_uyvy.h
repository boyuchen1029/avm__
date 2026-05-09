/**
 *******************************************************************************
 * @file : program_uyvy.h
 * @describe : Header of program_uyvy.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211227 0.1.1 Linda.
 *******************************************************************************
*/

#ifndef _PROGRAM_UYVY_H_
#define _PROGRAM_UYVY_H_
#include "avm/GLHeader.h"

extern void init_avm_program(void);

extern void open_gl_init_avm(void);

extern int open_gl_init_avm_fbo(void);

extern void draw_avm(const GLuint FramebufDefault);

#endif //_PROGRAM_UYVY_H_