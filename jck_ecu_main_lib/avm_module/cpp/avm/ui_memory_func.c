/**
 *******************************************************************************
 * @file : ui_memory_func.c
 * @describe : 
 *
 * @author : Linda.
 * @verstion : 0.1.0.
 * @date 20220504 0.1.0 Linda.
 *******************************************************************************
*/

#include "../../system.h"
#include "user_control/ui.h"
#include "avm/fp_source.h"
#include "avm/ui_memory_func.h"

#include <stdlib.h>
#include <string.h>

int YELLOW_COLOR[4]     =   {255,197,60,-1};
int RED_COLOR[4]        =   {255,50,0,-1};
int BLACK_COLOR[4]      =   {0,0,0,-1};
int DARK_COLOR[4]      =   {0,0,0,0};
int ORANGE_COLOR[4]     =   {244,159,5,-1};
int GRAY_COLOR[4]       =   {150,150,150,-1};
int NO_TRANSPARENT[4]   =   {-1, -1, -1,255};
int DR_HALF_TRANSPARENT[4]   =   {-1, -1, -1,150};
int HALF_TRANSPARENT[4]   =   {-1, -1, -1,220};
int RADER_YELLOW_COLOR[4]   =   {255, 255, 0,-1};
int RADER_RED_COLOR[4]        =   {255,50,0,-1};

void malloc_read_w_h(icon_info_t * icon, int w, int h, char const * dir)
{
    AVM_LOGI("malloc w %d h %d\n", w, h);
    icon->data = (unsigned char *)malloc(w*h*4*sizeof(unsigned char));
    icon->width = w;
    icon->height = h;
    icon->channel = 4;
    FILE * fp;
    fp = fp_source_rb(dir);
    if (fp == NULL)
    {
        AVM_LOGI("fopen fail: %s\n", dir);
        AVM_LOGE("Function malloc_read_w_h");
    }
    size_t retFread = 0;
    retFread = fread(icon->data, 1, w * h * 4, fp);
    if (retFread < 1)
    {
        AVM_LOGE("fread fail");
    }
    fclose(fp);


    AVM_LOGI("malloc icon->width %d icon->height %d\n", icon->width, icon->height);
}

void cal_and_store_coords(vertices_para_t vertices_para, ui_draw_info_t * avmDrawInfo, int w, int h)
{
    if(vertices_para.out.x == 0 && vertices_para.out.y == 0)
    {
        return;
    }

    // if(nowView == nowViewold)
    // {
    //     avmDrawInfo->rectangles++;
    //     return;
    // }
    // int w = SCREEN_W;
    // int h = SCREEN_H;
    float vertices_LX;
    float vertices_RX;
    float vertices_UY;
    float vertices_DY;
    float vertices_Z;

    float texCoords_LX;
    float texCoords_RX;
    float texCoords_UY;
    float texCoords_DY;

    // Normalize vertices
    vertices_LX = (2.0f / (float)w * (float)vertices_para.out.x) + (-1.0f);
    vertices_RX = vertices_LX + (2.0f / (float)w * (float)vertices_para.out.w);
    vertices_UY = (2.0f / (float)h * (float)vertices_para.out.y) - 1.0f;
    vertices_DY = vertices_UY - (2.0f / (float)h * (float)vertices_para.out.h);
    vertices_Z = vertices_para.outZ;

    // texture coordinate
    texCoords_LX = vertices_para.tex.x;
    texCoords_RX = vertices_para.tex.x + vertices_para.tex.w;
    texCoords_UY = vertices_para.tex.y;
    texCoords_DY = vertices_para.tex.y + vertices_para.tex.h;

    float verticesRgbaTmp[] = {
            vertices_LX, vertices_DY, vertices_Z,
            vertices_RX, vertices_DY, vertices_Z,
            vertices_RX, vertices_UY, vertices_Z,
            vertices_RX, vertices_UY, vertices_Z,
            vertices_LX, vertices_UY, vertices_Z,
            vertices_LX, vertices_DY, vertices_Z};
    memcpy(avmDrawInfo->vertex + 18 * (avmDrawInfo->rectangles - 1), verticesRgbaTmp, sizeof(verticesRgbaTmp));

    float verticesUvRgbaTmp[]= {
            texCoords_LX, texCoords_DY, // LD
            texCoords_RX, texCoords_DY, // RD
            texCoords_RX, texCoords_UY, // RU
            texCoords_RX, texCoords_UY, // RU
            texCoords_LX, texCoords_UY, // LU
            texCoords_LX, texCoords_DY};  // LD
    memcpy(avmDrawInfo->texCoods + 12 * (avmDrawInfo->rectangles - 1), verticesUvRgbaTmp, sizeof(verticesUvRgbaTmp));
    avmDrawInfo->rectangles++;
}

void copy_memory(unsigned char * mainMem, int mainMemW, unsigned char * source, position_item_t position)
{
    int iy;
    int x = (int)position.x;
    int y = (int)position.y;
    int w = (int)position.w;
    int h = (int)position.h;
    for(iy = y; iy < y + h; iy++)
    {
        memcpy(mainMem +  iy * (mainMemW * 4) + x * 4,
               source + (iy - y) * (w * 4),
               w * 4);

    }
}

void outputRgbaMem_color(unsigned char * mainMem, int mainMemW, int x, int y, int color[])
{
    int ix;
    for (ix=0;ix<4;ix++)
    {
        if(color[ix] != -1)
        {
            memset(mainMem + y * mainMemW * 4 + (x * 4 + ix), color[ix], 1);
        }
    }
}

void highlight(unsigned char * mainMem, int mainMemW, position_item_t position, high_light_color_t color)
{
    int ix, iy;
    int x = (int)position.x;
    int y = (int)position.y;
    int w = (int)position.w;
    int h = (int)position.h;
    int color_array[4];

    switch(color)
    {
        case YELLOW:
            memcpy(&color_array, &YELLOW_COLOR, sizeof(float)*4);
            break;
        case RED:
            memcpy(&color_array, &RED_COLOR, sizeof(float)*4);
            break;
        case BLACK:
            memcpy(&color_array, &BLACK_COLOR, sizeof(float)*4);
            break;
        case DARK:
            memcpy(&color_array, &DARK_COLOR, sizeof(float)*4);
            break;
        case ORANGE:
            memcpy(&color_array, &ORANGE_COLOR, sizeof(float)*4);
            break;
        case GRAY:
            memcpy(&color_array, &GRAY_COLOR, sizeof(float)*4);
            break;
        case NO_TRANS:
            memcpy(&color_array, &NO_TRANSPARENT, sizeof(float)*4);
            break;
        case HALF_TRANS:
            memcpy(&color_array, &HALF_TRANSPARENT, sizeof(float)*4);
            break;
        case RADER_RED:
            memcpy(&color_array, &RADER_RED_COLOR, sizeof(float)*4);
            break;
        case RADER_YEL:
            memcpy(&color_array, &RADER_YELLOW_COLOR, sizeof(float)*4);
            break;
        default:
            break;
    }
    for(iy = y; iy < y + h; iy++)
    {
        for(ix = x; ix < x + w; ix++)
        {
            outputRgbaMem_color(mainMem, mainMemW, ix, iy, color_array);
        }
    }
}

void set_half_tran(unsigned char * mainMem, int mainMemW, position_item_t position, high_light_color_t color)
{
    int ix, iy;
    int x = (int)position.x;
    int y = (int)position.y;
    int w = (int)position.w;
    int h = (int)position.h;
    int color_array[4];
    switch(color)
    {
        case DR_HALF_TRANS:
            memcpy(&color_array, &DR_HALF_TRANSPARENT, sizeof(float)*4);
            break;
        case HALF_TRANS:
            memcpy(&color_array, &HALF_TRANSPARENT, sizeof(float)*4);
            break;
        default:
            break;
    }
    for(iy = y; iy < y + h; iy++)
    {
        for(ix = x; ix < x + w; ix++)
        {
            int colorA = *(mainMem + iy * mainMemW * 4 + (ix * 4 + 3));
            if(colorA >125) // highlight yellow
            {
                outputRgbaMem_color(mainMem, mainMemW, ix, iy, color_array);
            }
        }
    }
}

position_item_t set_center(position_item_t page, float imageW, float imageH, align_t alignW, align_t alignH)
{
    if((page.w == imageW) && (page.h == imageH))
    {
        return page;
    }
    float x,y;
    x = (page.w - imageW)/2;
    y = (page.h - imageH)/2;

    position_item_t output;
    switch(alignW)
    {
        case LEFT:
            output.x = page.x;
            break;
        case CENTER:
            output.x = page.x + x;
            break;
        case RIGHT:
            output.x = page.x + (page.w - imageW);
            break;
        default:
            break;
    }

    switch(alignH)
    {
        case TOP:
            output.y = page.y;
            break;
        case CENTER:
            output.y = page.y + y;
            break;
        case BOTTOM:
            output.y = page.y + (page.h - imageH);
            break;
        default:
            break;
    }
    output.w = imageW;
    output.h = imageH;
    return output;
}





