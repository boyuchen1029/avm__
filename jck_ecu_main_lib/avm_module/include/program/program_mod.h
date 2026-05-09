#ifndef _PROGRAM_MOD_H_
#define _PROGRAM_MOD_H_

#include "program_line.h"
typedef struct 
{
    PointF OffSet;
    VECF4 mask;
    float UV_IMGHeight;
    float UV_IMGWidth;
    float UV_Ratio_HbyW;
    vector_point2F l_mod_point_text;
    vector_point2F r_mod_point_text;
    float* vertArray;
    float* textArray;
    float* alphaArray;

    float* triangle_vertArray;
    float* triangle_textArray;
    float* triangle_alphaArray;
}MOD_POLYGON_SUPPLY;

typedef struct
{
    int program;
    int texture;
    int glAttrVertex;
    int glAttrUV;
    int vao[10];
    int vbo[10][5];
}GLContext;

extern void init_mod_program(void);
extern void open_gl_init_mod_line(void);
extern void init_mod_line(float carSizeW, float carSizeH, int w, int h);
extern void draw_mod();
extern void upload_newMODline(int* modbuffer);
#endif //_PROGRAM_MOD_H_
