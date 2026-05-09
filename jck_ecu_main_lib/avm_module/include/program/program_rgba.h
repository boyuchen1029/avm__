/**
 *******************************************************************************
 * @file : program_rgba.h
 * @describe : Header of program_rgba.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211116 0.1.0 Linda.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/

#ifndef _PROGRAM_RGBA_H_
#define _PROGRAM_RGBA_H_

#include "user_control/ui.h"
#include "program/set_framebuffer.h"
#include "avm/ui_memory_func.h"

extern void init_2davm_program(void);

extern void rgba_mem_malloc(void);

extern void open_gl_init_avm_2d(void);

extern void open_gl_init_avm(void);

extern void open_gl_init_rgba(void);

extern FBO_t* open_gl_init_2davm_fbo(int texWidth, int texHeight);

extern void draw_2DAVM_texture(void);

extern void draw_texture(GLuint texture, int vao);

extern void draw_rgba(ui_draw_info_t avmDraw, int stadyn, int subdata, int texsub);

extern void draw_tex(unsigned char * arUi);

#endif //_PROGRAM_RGBA_H_