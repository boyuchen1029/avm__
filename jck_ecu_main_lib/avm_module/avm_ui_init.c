/**
 *******************************************************************************
 * @file : avm_ui_init.cpp
 * @describe : Initialize memory of UI images and UI memory handle.
 *
 * @author : Bennit Yang.
 * @verstion : 0.1.0.
 * @date 20211101 0.1.0 Bennit Yang.
 *******************************************************************************
*/

#include "../../system.h"
#include "avm/ui_memory_func.h"
#include "avm_ui_init.h"
#include "ui_struct.h"

#include <stdlib.h>
#include <string.h>

static unsigned char * outputRgbaMemSta;
static unsigned char * outputRgbaMemDyn;
static unsigned char * outputRgbaMemCar;

static avm_ui_t avmUi;

static void malloc_rgba(avm_ui_t * ui)
{
    int channels = 4;
}

static void read_icon(avm_ui_t * ui)
{
    // malloc_read_w_h(&ui->car_2davm, CAR_2D_W, CAR_2D_H, "rgba/ui/s6_car_2d.rgba");
    // fwrite_rgba(ui->car_2davm.data, 100, 200, "car_2davm222.rgba");

    // malloc_read_w_h(&ui->FBLR, 761, 141, "test_img/capture_success.rgba");
    // malloc_read_w_h(&ui->oda, ODA_W, ODA_H, "rgba/ui/mcp_FBLR.rgba");
    char dirTime[50];
    int ix;
    // for (ix = 0; ix < 10; ++ix)
    // {
    //     sprintf(dirTime, "rgba/ui/%d.rgba", ix);
    //     malloc_read_w_h(&ui->iconTime[ix], DIGIT_W, DIGIT_H, dirTime);
    // }

}

void ui_init()
{
    AVM_LOGI("ui_init start\n");
    outputRgbaMemSta = (unsigned char *)calloc(OUTPUT_RGBA_MEM_W * OUTPUT_RGBA_MEM_H * 4 * sizeof(unsigned char),sizeof(unsigned char));
    outputRgbaMemDyn = (unsigned char *)calloc(OUTPUT_RGBA_MEM_W * OUTPUT_RGBA_MEM_H * 4 * sizeof(unsigned char),sizeof(unsigned char));
    outputRgbaMemCar = (unsigned char *)calloc(OUTPUT_CAR_MEM_W * OUTPUT_CAR_MEM_H * 4 * sizeof(unsigned char),sizeof(unsigned char));
//    malloc_rgba( &avmUi);
    read_icon(&avmUi);
    AVM_LOGI("ui_init end\n");
}

void get_rgba_memory(unsigned char ** RgbaMemSta, unsigned char ** RgbaMemDyn, unsigned char ** RgbaMemCar, avm_ui_t ** avmUiprog)
{
    *RgbaMemSta = outputRgbaMemSta;
    *RgbaMemDyn = outputRgbaMemDyn;
    *RgbaMemCar = outputRgbaMemCar;
    *avmUiprog = &avmUi;
}

void free_icon()
{
    free(avmUi.FBLR.data);
    AVM_LOGI("free_icon end\n");
}

void ui_info(avm_ui_info_t * uiInfo)
{
    uiInfo->circleStart =0;
}



