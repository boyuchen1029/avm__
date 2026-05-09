/**
 *******************************************************************************
 * @file     : avm_ui_init.h
 * @describe : Header for avm_ui_init.c file
 * 
 * @author : Bennit Yang.
 * @verstion : 0.1.0.
 * @date 20211028 0.1.0 Bennit Yang.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/
//
// Created by AI611 on 2021/10/28.
//

#ifndef _AVM_UI_INIT_H_
#define _AVM_UI_INIT_H_

#include "avm/ui_memory_func.h"
#include "ui_struct.h"
#include "user_control/ui.h"

#define OUTPUT_RGBA_MEM_W  80
#define OUTPUT_RGBA_MEM_H  80

#define OUTPUT_CAR_MEM_W  20
#define OUTPUT_CAR_MEM_H  40
// main memory

// FBLR memory
#define FBLR_W 126
#define FBLR_H 125

#define DIGIT_W 36
#define DIGIT_H 73

// ODA image
#define ODA_W 768
#define ODA_H 459

#define CAR_2D_W 200.0
#define CAR_2D_H 400.0

typedef struct
{
//    unsigned char * FBLR;
//    unsigned char * iconTime[10];

//    unsigned char * oda;

    icon_info_t FBLR;
    icon_info_t iconTime[10];

    icon_info_t oda;

    icon_info_t car_2davm;
}avm_ui_t;


extern void ui_init(void);

extern void get_rgba_memory(unsigned char ** RgbaMemSta, unsigned char ** RgbaMemDyn, unsigned char ** RgbaMemCar, avm_ui_t ** avmUiprog);

extern void free_icon(void);

extern void ui_info(avm_ui_info_t * uiInfo);


#endif //_AVM_UI_INIT_H_
