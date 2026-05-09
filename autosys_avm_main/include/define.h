/**
 *******************************************************************************
 * @file : define.h
 * @describe : 
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211112 0.1.0 Linda.
 *******************************************************************************
*/

#ifndef _DEFINE_H_
#define _DEFINE_H_

#define STOPnGO 0
#define BSD 0
#define MODsta 1
#define MODdyn 0


#define FORMAT YUYV
#define GLAPI VIV
//set file reverse

#define CAMERA_CH 4

#define HDwidth 1280
#define HDheight 720

#define SCREEN_W 1280
#define SCREEN_H 720

//      LUXGEN_URX   /   CROSS   /   LUXGEN_U6    /    HUANSU_S6
#define CAR_MODEL LUXGEN_URX

#define SOURCE_DIR "."
#define SOURCE_AVM_DIR "."

extern int imageWidth;
extern int imageHeight;
extern int VIEW;

extern int Cam_PixelFMT; //0: NV12, 1: YUYV

enum {
    disable = 0,
    enable
};

#define AVM_w SCREEN_W  //1920
#define AVM_h SCREEN_H //720

//int PANEL_WIDTH;
//int PANEL_HEIGHT;

#endif //_DEFINE_H_