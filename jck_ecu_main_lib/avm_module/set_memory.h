/**
 *******************************************************************************
 * @file : set_memory.h
 * @describe : Header of set_memory.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20220120 0.1.0 Linda.
 *******************************************************************************
*/

#ifndef _SET_MEMORY_H_
#define _SET_MEMORY_H_

#include "user_control/ui.h"
#include "avm/ui_memory_func.h"


typedef enum
{
    ODA = 0,
    FBLR,
    FRAME,
    CAR_2DAVM,
}img_id_t;

typedef struct
{
    unsigned char * img;
    position_item_t  position;
    float imgW;
    float imgH;
    int uiswitch;
}img_position_t;

extern void set_memory_malloc(void);

extern void set_ui_memory(avm_ui_info_t * uiInfo);

extern void draw_memory(void);

extern void draw_2d_car(int texobj, int viewportW, int viewportH);

extern vertices_para_t init_texure_vertices(img_id_t imgId);

#endif //_SET_MEMORY_H_