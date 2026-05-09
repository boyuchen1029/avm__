/**
 *******************************************************************************
 * @file : avm_param.h
 * @describe : 
 *
 * @author : Alfred
 * @verstion : 0.1.0.
 * @date 20240514 0.1.0 Alfred.
 *******************************************************************************
*/

#ifndef _AVM_PARAM_H_
#define _AVM_PARAM_H_
/*==============================================================
*                        SYSTEM
===============================================================*/
#define VERSION_MAJOR 15
#define VERSION_MINOR 1
#define MAX_CAM_NUM 4
/*==============================================================
*                        PAGE
===============================================================*/
#define MAX_PAGE_CONTENTS 30
#define MAX_PAGE_LIST_NUM 20
#define MAX_PAGE_NUM 30
#define DEBUG_SHOWN_CURRENT_PAGE_COMPONECT 1
/*==============================================================
*                            AVM Operate
===============================================================*/
#pragma region AVM Operate
#define VEC3_CPY(inA, inB) \
    do { \
        (inA).X = (inB).X; \
        (inA).Y = (inB).Y; \
        (inA).Z = (inB).Z; \
    } while(0)


#define VEC3_MINUS(in, a, b) \
    do { \
        (in).X = (a).X - (b).X; \
        (in).Y = (a).Y - (b).Y; \
        (in).Z = (a).Z - (b).Z; \
    } while(0)

#define VEC3_PLUS(in, a, b) \
    do { \
        (in).X = (a).X + (b).X; \
        (in).Y = (a).Y + (b).Y; \
        (in).Z = (a).Z + (b).Z; \
    } while(0)

#define VEC3_DIVI(in, divisor) \
    do { \
        (in).X = (in).X / (divisor); \
        (in).Y = (in).Y / (divisor); \
        (in).Z = (in).Z / (divisor); \
    } while(0)

#define VEC3_RESET(a) memset((a).data, 0, sizeof(int) * 3)
#pragma endregion
/*==============================================================
*                        OPENGL PARAM
===============================================================*/
#pragma region OPENL PARAM
#define PARAM_MINIMAP_FUNCTION                         1
#define PARAM_MINIMAP_MAX_LEVEL                        4
#define PARAM_OPENGL_OIT_BUFFER_FBO_ON                 1
#define PARAM_OPENGL_OIT_COLOR_ALPHA_INDIVIUAL         1

#pragma endregion
/*==============================================================
*                        AVM BB PARAM
===============================================================*/
#pragma region BB PARAM
#define PARAM_AVM_BB_FUNCTION                           1 // 1 is turn on;
#define PARAM_AVM_BB_PID_FUNCTION                       1 // 1 is turn on;
#define PARAM_AVM_BB_SAMPLING_FILTER_FUNCTION           0 // 1 is turn on;
#define PARAM_AVM_BB_LIMIT_FILTER_FUNCTION              1 // 1 is turn on;
#define PARAM_AVM_BB_GAMMA_ENHANCE_FUNCTION             1// 1 is turn on;
#define PARAM_AVM_BB_GAMMA_THRESHOLD                    0.4f
//#define PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME            6
#define PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME            g_avm_BB_PID_Latency
#define PARAM_AVM_BB_PID_BRIGNESS_LIST_BUFFER_LENGTH    60
#define PARAM_AVM_BB_PID_BRIGNESS_CAM_NUM               25
#define PARAM_AVM_BB_UPPER_LIMIT                        225
#define PARAM_AVM_BB_LOWER_LIMIT                        25
//#define PARAM_AVM_BB
#pragma endregion
/*==============================================================
*                        AVM IMAGE QUILTY PARAM
===============================================================*/
#pragma region IMAGE QUILTY PARAM
#define FXAA                 1
#if FXAA 
#define FXAA_FISHEYE_MODE    1
#define FXAA_2DAVM_MODE      0
#define FXAA_3DAVM_MODE      0
#define FXAA_LOSS_PARAM_FLAG 0
#define FXAA_LOSS_FRAME      4 // (x / 30) fps
#define PARAM_FXAA_LOSS_PANEL_VOLUME     0.8
#define PARAM_FXAA_PANEL_HEIGHT PARAM_FXAA_LOSS_PANEL_VOLUME * g_PANEL_HEIGHT
#define PARAM_FXAA_PANEL_WIDTH  PARAM_FXAA_LOSS_PANEL_VOLUME * g_PANEL_WIDTH
#endif
#define PARAM_CARMODEL_OIT_SHARE_DEPTH_BUFFER     1

#if(PARAM_CARMODEL_OIT_SHARE_DEPTH_BUFFER == 1)
    #define PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME     1.0
#else
    #define PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME     1.0
#endif

#define PARAM_FISHEYE_BLUR_FUNCTION               0
#define PARAM_FISHEYE_BLUR_GAUSSIAN_FUNCTION      0
#define PARAM_FISHEYE_BLUR_LOSS_PANEL_H_VOLUME    0.9
#define PARAM_FISHEYE_BLUR_LOSS_PANEL_W_VOLUME    0.6

#pragma endregion
/*==============================================================
*                        AVM PGL PARAM
===============================================================*/
#pragma region AVM PGL PARAM
#define PARAM_AVM_TOP2D_ALIGMENT_FUNCTION 1
#define PARAM_AVM_PGL_MAX_LENGTH 10
#define PARAM_AVM_DRAW_LINE_MAX 17
#define PARAM_PGL_ALIGMENT_GOLDENVERSTION 1
#define PARAM_DYNAMIC_PGL_MAX_ANGLE 30.0f
#define PARAM_DYNAMIC_PGL_MIN_ANGLE -30.0f
#define PARAM_DYNAMIC_LINE_MAX_NUM 200 // one line has 200 knife
#define PARAM_SCENEVIEW_WIDTH  400
#define PARAM_SCENEVIEW_HEIGHT 274
#define ENABLE_REPORJUSTION_SCENEVIEW_PGL  0
#define PARAM_PGL_MAX_STYLE_NUM 5
#define PARAM_PGL_MAX_VAO_NUM 20
#pragma endregion
/*==============================================================
*                        AVM SEETHROUGH
===============================================================*/
#define PARAM_AVM_SEETHROUGH 1
#define PARAM_CARMODEL_ALPHA_DEPEND_SEEHTROUGH 0
#define SEETHROUGH_DEMO (PARAM_AVM_SEETHROUGH && 0)
/*==============================================================
*                        AVM MOD
===============================================================*/
#define PARAM_AVM_SMOD 1
/*==============================================================
*                        OFFLINE SHADER
===============================================================*/
/*==============================================================
*                        zoom_birdView
===============================================================*/
#define PARAM_ZOOM_ENABLE 1
#define PARAM_ZOOM_BIRDVIEW_MAX 4
#define PRRAM_ZOOM_DIFF_BY_TOUCH 0
/*==============================================================
*                        Touch function
===============================================================*/
#pragma region touch{
#define PARAM_TOUCH_CLICK_PROCESSTIME 0.5f
#define PRRAM_TOUCH_CLICK_RESPONSECOUNT 200
#pragma endregion}
/*==============================================================
*                        AVM CONFIG DEFAULT
===============================================================*/
#pragma region config{
#define EVSTable_Path_0 "/application/avm_usb/"
#define EVSTable_Path_1 "/media/flash/avm/"
#define EVSTable_Path_2 "/tmp/"
#pragma endregion}

/*==============================================================
*                        AVM animation
===============================================================*/
#pragma region animation{
#define MAX_ANIMATION_NUM 3
#pragma endregion}
/*==============================================================
*                        CARMODEL information
===============================================================*/
#pragma region animation{
#define MAX_STATIC_CAR_FBO_NUM 3
#define MAX_CARMODEL_OBJ_NUM 50
#pragma endregion}

#endif //_AVM_PARAM_H_
