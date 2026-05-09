/**
 *******************************************************************************
 * @file : program_seeThrough.h
 * @describe : Header of program_seeThrough.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20220106 0.1.0 Linda.
 *******************************************************************************
*/

#ifndef _PROGRAM_SEETHROUGH_H_
#define _PROGRAM_SEETHROUGH_H_

#include "avm/camera_matrix.h"
#include "../../../setup/AVM_STRUCT.h"
#include "canbus.h"

extern void init_seethrough_program(void);

extern void open_gl_init_seethrough(double horiFov, double vertiFov);

extern void seeThrough_rotate_callculate(camera_para_t * para, can_bus_info_t *canbus);

extern void set_seeThrough_fbo(GLuint texture, float * rotate_2davm, float * rotate_see);

extern void draw_seethrough(void);

extern void flash_seethrough_texture();

extern void seethrough_process_init(int w, int h);

extern void calculate_seethrough_prcess_value();

extern float get_seethrough_processValue();

#endif //_PROGRAM_SEETHROUGH_H_