/**
 *******************************************************************************
 * @file     : ui_memory_func.h
 * @describe : Header for ui_memory_func.c file
 * 
 * @author : Linda.
 * @verstion : 0.1.0.
 * @date 20220504 0.1.0 Linda.
 *******************************************************************************
*/


#ifndef _UI_MEMORY_FUNC_H_
#define _UI_MEMORY_FUNC_H_


#include "user_control/ui.h"


typedef struct
{
    position_item_t  out;
    position_item_t  tex;
    float outZ;
    int StaDyn;
}vertices_para_t;

typedef struct
{
    float vertex[18 * 10];
    float texCoods[12 * 10];
    int rectangles;
}ui_draw_info_t;

typedef enum
{
    LEFT = 0,
    CENTER,
    RIGHT,
    TOP,
    BOTTOM
}align_t;

typedef enum
{
    YELLOW = 0,
    RED,
    BLACK,
    DARK,
    ORANGE,
    GRAY,
    NO_TRANS,
    DR_HALF_TRANS,
    HALF_TRANS,
    RADER_RED,
    RADER_YEL,
    
}high_light_color_t;

typedef struct
{
    unsigned char * data;
    int width;
    int height;
    int channel;
}icon_info_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void malloc_read_w_h(icon_info_t * icon, int w, int h, char const * dir);

extern void cal_and_store_coords(vertices_para_t vertices_para, ui_draw_info_t * avmDrawInfo, int w, int h);

extern void copy_memory(unsigned char * mainMem, int mainMemW, unsigned char * source, position_item_t position);

extern void outputRgbaMem_color(unsigned char * mainMem, int mainMemW, int x, int y, int color[]);

extern void highlight(unsigned char * mainMem, int mainMemW, position_item_t position, high_light_color_t color);

extern void set_half_tran(unsigned char * mainMem, int mainMemW, position_item_t position, high_light_color_t color);

extern position_item_t set_center(position_item_t page, float imageW, float imageH, align_t alignW, align_t alignH);


#ifdef __cplusplus
}
#endif



#endif //_UI_MEMORY_FUNC_H_
