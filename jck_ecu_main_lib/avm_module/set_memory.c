/**
 *******************************************************************************
 * @file : program_rgba.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 2021/11/16 0.1.0 Linda.
 * @date 2021/12/08 0.1.1 Bennett.
 *******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

// #define DEBUG
#include "../../system.h"
#include "user_control/ui.h"
#include "avm/ui_memory_func.h"

#include "avm_ui_init.h"
#include "display_location.h"
#include "set_memory.h"
#include "program/program_rgba.h"
#include "avm/fp_source.h"

static unsigned char *outputRgbaMemSta;
static unsigned char *outputRgbaMemDyn;
static unsigned char *outputRgbaMemCar;

// static FBO_t fbo_2davm;

// URX status bar
static ui_draw_info_t avmDraw[2];
static avm_ui_t * avmUi;
static avm_view_t nowView;
static avm_view_t nowViewold;
extern int carSizeW,carSizeH;

static ui_draw_info_t avmDrawRader;
static ui_draw_info_t avmDrawDot;

static int recordstartFlag;
static int frame;



vertices_para_t init_texure_vertices(img_id_t imgId)
{
    vertices_para_t vertices;
    switch(imgId)
    {
    case CAR_2DAVM:
        vertices.out.w = 190;
        vertices.out.h = 350;
        vertices.out.x = ((float)(AVM_2D_W - vertices.out.w) / 2.0) - 0; // 426
        vertices.out.y = ((float)(AVM_2D_H + vertices.out.h) / 2.0) + 0; // 720
        vertices.tex.x = 0.0;
        vertices.tex.y = 0.0;
        vertices.tex.w = CAR_2D_W;
        vertices.tex.h = CAR_2D_H;
        vertices.outZ = -1.0;
        vertices.StaDyn = 2;
        break;
    case FBLR:
        vertices.out.x = (float)(SCREEN_W - 0) / 1;
        vertices.out.y = (float)(SCREEN_H + 0) / 1;
        vertices.out.w = 761;
        vertices.out.h = 141;
        vertices.tex.x = 0.0;
        vertices.tex.y = 0.0;
        vertices.tex.w = 761;
        vertices.tex.h = 141;
        vertices.outZ = -1;
        vertices.StaDyn = 2;
        break;
    case FRAME:
        vertices.out.x = (float)(SCREEN_W - (DIGIT_W * 6)) / 2;
        vertices.out.y = DIGIT_H;
        vertices.out.w = (DIGIT_W * 6);
        vertices.out.h = DIGIT_H;
        vertices.tex.x = 0.0;
        vertices.tex.y = 0.0;
        vertices.tex.w = (DIGIT_W * 6);
        vertices.tex.h = DIGIT_H;
        vertices.outZ = 0.0;
        vertices.StaDyn = 1;
        break;
    case ODA:

        vertices.out.x = HALF_W;
        vertices.out.y = SCREEN_H;
        vertices.out.w = ODA_W;
        vertices.out.h = ODA_H;

        vertices.tex.x = 0.0;
        vertices.tex.y = SCREEN_H;
        vertices.tex.w = ODA_W;
        vertices.tex.h = ODA_H;
        vertices.outZ = 0.0;
        vertices.StaDyn = 1;
        break;
    default:
        vertices.out.x = 0.0;
        vertices.out.y = 0.0;
        vertices.out.w = 0.0;
        vertices.out.h = 0.0;
        vertices.tex.x = 0.0;
        vertices.tex.y = 0.0;
        vertices.tex.w = 0.0;
        vertices.tex.h = 0.0;
        vertices.outZ = 0.0;
        vertices.StaDyn = 0;
        break;
    }
    return vertices;
}

static void set_coord(vertices_para_t vertices, ui_draw_info_t * avmDrawInfo)
{
    vertices.tex.x = (float)vertices.tex.x / OUTPUT_RGBA_MEM_W;
    vertices.tex.y = (float)vertices.tex.y / OUTPUT_RGBA_MEM_H;
    vertices.tex.w = (float)vertices.tex.w / OUTPUT_RGBA_MEM_W;
    vertices.tex.h = (float)vertices.tex.h / OUTPUT_RGBA_MEM_H;

    cal_and_store_coords(vertices, avmDrawInfo, SCREEN_W, SCREEN_H);
}

static void set_oda_memory()
{
    // copy_memory(outputRgbaMemDyn, OUTPUT_RGBA_MEM_W, arui[ix].img, arui[ix].position);
    

    // fwrite_rgba(outputRgbaMemDyn, OUTPUT_RGBA_MEM_W, OUTPUT_RGBA_MEM_H, "outputRgbaMemDyn.rgba");
}

static void set_oda_coord()
{    
    vertices_para_t vertices = init_texure_vertices(ODA);
    set_coord(vertices, &avmDraw[vertices.StaDyn]);
    AVM_LOGI("set oda end\n");
}

static void set_fblr_coord()
{
    vertices_para_t vertices = init_texure_vertices(FBLR);
    set_coord(vertices, &avmDraw[vertices.StaDyn]);
}

static void set_frame_coord()
{
    vertices_para_t vertices = init_texure_vertices(FRAME);
    int ix;
    for(ix = 0; ix < 6; ix++)
    {
        float x = DIGIT_W * ix;
        float y = 0;
        int digit = (int)(frame / (int)pow(10, (5 - ix)));

        position_item_t position = {x, y, DIGIT_W, DIGIT_H};
        copy_memory(outputRgbaMemDyn, OUTPUT_RGBA_MEM_W, avmUi->iconTime[digit].data, position);
        frame = (int)(frame % (int)pow(10, (5-ix)));
    }
    if(recordstartFlag)
    {
        highlight(outputRgbaMemDyn, OUTPUT_RGBA_MEM_W, vertices.tex, DARK);
    }
    set_coord(vertices, &avmDraw[vertices.StaDyn]);
}

void set_ui_memory(avm_ui_info_t * uiInfo)
{
    nowView = uiInfo->view;
    nowViewold = uiInfo->viewold;

    if(nowView != nowViewold)
    {
        avmDraw[0].rectangles=1;
        avmDraw[1].rectangles=1;
        switch(nowView)
        {
            case VIEW_ODA_2D_DUAL_SV:
                set_oda_coord();
                break;
            case VIEW_RECORD_4_IN_1:
                set_fblr_coord();
                set_frame_coord();
                break;
            default:
                break;
        }
    }
}

void draw_memory(void)
{
    // AVM_LOGI("static rectangles %d Dyn rectangles %d\n",avmDraw[0].rectangles, avmDraw[1].rectangles);
    if(nowViewold != nowView)
    {
        draw_rgba(avmDraw[0], 0, 1, 0);
        draw_rgba(avmDraw[1], 1, 1, 1);
    }
    else
    {
        draw_rgba(avmDraw[0], 0, 0, 0);
        draw_rgba(avmDraw[1], 1, 0, 0);
    }
}

static ui_draw_info_t avmDrawCar;
void draw_2d_car(int texobj, int viewportW, int viewportH)
{
    vertices_para_t vertices;
    avmDrawCar.rectangles = 1;

    vertices = init_texure_vertices(CAR_2DAVM);

    vertices.tex.x = (float)vertices.tex.x / OUTPUT_CAR_MEM_W;
    vertices.tex.y = (float)vertices.tex.y / OUTPUT_CAR_MEM_H;
    vertices.tex.w = (float)vertices.tex.w / OUTPUT_CAR_MEM_W;
    vertices.tex.h = (float)vertices.tex.h / OUTPUT_CAR_MEM_H;
    cal_and_store_coords(vertices, &avmDrawCar, viewportW, viewportH);

    draw_rgba(avmDrawCar, 5, 1, 0);
}
static ui_draw_info_t avmDrawfblr;

void draw_fblr_car(int texobj, int viewportW, int viewportH)
{
    vertices_para_t vertices;
    avmDrawfblr.rectangles = 1;

    vertices = init_texure_vertices(FBLR);

    // vertices.tex.x = (float)vertices.tex.x / OUTPUT_CAR_MEM_W;
    // vertices.tex.y = (float)vertices.tex.y / OUTPUT_CAR_MEM_H;
    // vertices.tex.w = (float)vertices.tex.w / OUTPUT_CAR_MEM_W;
    // vertices.tex.h = (float)vertices.tex.h / OUTPUT_CAR_MEM_H;
    cal_and_store_coords(vertices, &avmDrawfblr, viewportW, viewportH);

    draw_rgba(avmDrawfblr, 7, 1, 0);
}

void init_outputRgbaMemDyn(void)
{
    vertices_para_t vertices;

    // vertices = init_texure_vertices(FBLR);
    // copy_memory(outputRgbaMemSta, OUTPUT_RGBA_MEM_W, avmUi->FBLR.data, vertices.tex);

    vertices = init_texure_vertices(CAR_2DAVM);

//    set_coord(vertices, &avmDraw[vertices.StaDyn]);

    // copy_memory(outputRgbaMemCar, OUTPUT_CAR_MEM_W, avmUi->car_2davm.data, vertices.tex);

    // fwrite_rgba(outputRgbaMemSta, OUTPUT_RGBA_MEM_W, OUTPUT_RGBA_MEM_H, "outputRgbaMemSta.rgba");
    // free_icon();
}

void set_memory_malloc(void)
{
    get_rgba_memory( &outputRgbaMemSta, &outputRgbaMemDyn, &outputRgbaMemCar, &avmUi);
    init_outputRgbaMemDyn();
}
