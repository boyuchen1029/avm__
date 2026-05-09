/**
 *******************************************************************************
 * @file : avm_struct.h
 * @describe : 
 *
 * @author : Alfred
 * @verstion : 0.1.0.
 * @date 20240521 0.1.0 Alfred.
 *******************************************************************************
*/

#ifndef _AVM_STRUCT_H_
#define _AVM_STRUCT_H_

#include "AVM_PAGE.h"
#include "avm/camera_matrix.h"
#include "../avm_module/avmDefine.h"
#include "../avm_module/include/gl/glContext.h"
#include "avm/GLHeader.h"
#include "program/set_framebuffer.h"
#include <sys/time.h>
/*==============================================================
*                      BB COLOR
===============================================================*/
typedef struct
{
    struct
    {
        int SUM_FL;
        int SUM_FR;
        int SUM_BL;
        int SUM_BR;
        int SUM_LF;
        int SUM_LB;
        int SUM_RF;
        int SUM_RB; 

        int SUM_FM; 
        int SUM_BM; 
        int SUM_LM; 
        int SUM_RM; 

    }block_y_value;

    struct
    {
        /* data */
        float Y_vignette_Cam_1[4][3];
        float Y_vignette_Cam_2[4][3];
    }vignette;
    struct
    {
        /* data */
        float Y_gamma_FL;
        float Y_gamma_FR;
        float Y_gamma_BL;
        float Y_gamma_BR;
        float Y_gamma_LL;
        float Y_gamma_LR;
        float Y_gamma_RL;
        float Y_gamma_RR;
    }gamma_value;

    struct
    {
        /* data */
        float diffFL;
        float diffFR;
        float diffBL;
        float diffBR;
        float diffLF;
        float diffLB;
        float diffRF;
        float diffRB;
    }diff_value;

    struct
    {
        /* data */
        float diffFL;
        float diffFR;
        float diffBL;
        float diffBR;
        float diffLF;
        float diffLB;
        float diffRF;
        float diffRB;
    }diff_pid_value;
}colorObj;

typedef struct
{
    float avg[4];
    float diff[4];
    float homo[4];

    float avg_part1[4];
    float avg_part2[4];
    float diff_part1[4];
    float diff_part2[4];
    float gammaL[4];
    float gammaR[4];
    float gamma_angle[4];
}AvmBrightnessBalanceConfig;

/*==============================================================
*                      CAR MODEL
===============================================================*/
typedef enum
{
    BACK_CULL_MODE         = 0,
    OIT_LIGHTWEIGHT_MODE   = 1,
}Alpha_MODE;

typedef enum
{
    ALPHA_CAR_BODY = 0,
    ALPHA_WINDOWS  = 1,
    ALPHA_TIRE     = 2,
    ALPHA_INTERIOR = 3, 
}ALPHA_CAR_ITEMS_MODE;

typedef enum
{
    CAR_BODY = 0,
    WINDOWS  = 1,
    TIRE     = 2,
    INTERIOR = 3, 
    LAMP     = 4,
}CAR_ITEMS_NAME;

typedef struct 
{
    int flowing_enable;
    int signal;
    int flowing_lamp_id;
    int flowing_time_count;
    int flowing_next_time_gap;
    float flowing_during_time;
    int cur_idx; 
    int pre_idx;
    int flag_flowing_img_exist  ;
    struct timeval time_start;
    struct timeval time_end;
}CAR_LAMP_UPLOAD_INFO;

/*==============================================================
*                      OPENGL struct
===============================================================*/
typedef enum
{
    FISHEYE_DISTORTION = 0,
    FISHEYE_UNDISTORTION = 4,
    AVM_3D_VIEW = 8,
    AVM_2D_VIEW = 12,
    FISHEYE_STATIC_DISTORTION = 16,
    FISHEYE_STATIC_SCENE = 20,
    FISHEYE_DYNAIC_SCENE = 24,
}ViewType;

typedef struct 
{
    camera_para_t Projection_2Davm;
    camera_para_t *Projection_3Davm;
    camera_para_t Projection_2DCAR;
}opengl_projection_matrix;

typedef struct
{
	int count;
	void (*init)(int,struct vector_point2F*);
	PointF* pts;
	void (*add)(float,float,struct vector_point2F*);
    void (*addRange)(float*, int, struct vector_point2F*);
	void (*vfree)(struct vector_point2F*);
}vector_point2F;

typedef struct
{
	int count;
	VECF3* pts;
	void (*init)(int,struct vector_point3F*);
	void (*add)(struct vector_point3F*);
	void (*addRange)(float*, int, struct vector_point3F*);
	void (*vfree)(struct vector_point3F*);
}vector_point3F;
/*==============================================================
*                      TOP2D struct
===============================================================*/
typedef struct
{
    int color_style;
    int car2d_status[5];
    int car3d_status[5];
    uint8_t Shadow_draw_mode;
    //lamp
    uint8_t flowing_lamp_enable[3];
    int flowing_lamp_speed;
    int flowing_lamp_max_length;
    //static-car-system
    int currentPage;
    int nextPage;
    int enable_draw;
    int Triger_Regester;
    int Triger_Destory;
    uint8_t f_record;
    int freeze_frame_count;
    int delay_count;
    VECF4 _xywh_;
    int Pages_key[MAX_PAGE_NUM];
    int Pages_Chosen[MAX_PAGE_NUM];
    int Pages_key_num;
}CAR_MODEL_INFO;

typedef struct
{
    int door_status[5];
    int draw_opendoor_mode;
    float opendoor_color[3];
}AVM_INFO;

/*==============================================================
*                      TOP2D struct
===============================================================*/
typedef struct
{
    Rectangle viewport;
}SET_CUR_VIEWDATA;

/*==============================================================
*                      quene struct
===============================================================*/
typedef struct 
{
    int* data;
    int  used;
    /* data */
}AVMQUENE;

/*==============================================================
*                      IVI struct
===============================================================*/
typedef struct 
{
    char* img_path;
    float position[5][4];
    /* data */
}IVI_WARNING;

/*==============================================================
*                     Autosys AVM struct
===============================================================*/
typedef enum
{
    TOUCH_EMPTY,
    TOUCH_PRESS = 2,
    TOUCH_LEAVE = 4,
    TOUCH_MOVE = 8,
    TOUCH_CLICK = 16,
}TOUCH_HANDLE_CASE;

typedef struct 
{
    int minimap_enable;
    int blur_enable[4];
    int blur_level[4];
    float blur_mask[4][4];
    /* data */
}PARA_BLUR;

// fuc = function, on = 1, off = 0;
typedef struct
{   
    uint8_t fuc_freeze;
    uint8_t fuc_smod;
    uint8_t fuc_touch;
    uint8_t fuc_zoom_birdView;
    uint8_t fuc_pgl;
    uint8_t fuc_top2D_pgl_AA;
    uint8_t fuc_fxaa;
    uint8_t fuc_msaa; 
    uint8_t fuc_seethrough; 
    uint8_t fuc_ivi_control;
    uint8_t fuc_lamp_status_change;
    uint8_t fuc_avm_bb;
    uint8_t fuc_avm_bb_pid;
    uint8_t fuc_crc_check;
    uint8_t fuc_user_control;
}autosys_root_flag;

typedef struct 
{
    uint8_t update_status; 
    uint8_t cur_level;
    uint8_t firs_select_block;
    uint8_t linetable[PARAM_ZOOM_BIRDVIEW_MAX][2]; //vertical, horizal
    int Each_level_num[PARAM_ZOOM_BIRDVIEW_MAX];
    float preXshift;
    float preYshift;
    float preZoom;
    /*20241226 new config*/
    int enable_layer_num;
    int block_size_row;
    int block_size_col;
    float diff_ratio[4][2];
    int enableblock[4][4];
}zoom_birdView;

/**
 * @brief The autosys_system_info struct
 * 
 * @note system_info 結構體為 AVM 系統執行期間的基礎參數集合，主要提供畫面顯示、車型設定、投影矩陣、
 *       觸控處理等相關資訊，屬於唯讀型態，不應在執行時被修改。
 */
typedef struct 
{
    Point SCREEN;
    Point TOP2D;
    Point CarSize;
    Point IMGSize;
    /*Bowl*/
    int   BOWL_offset[4];
    int   BOWL_angle[4];
    Point BOWL_shadow;
    /*carmodel*/
    int   carModel_item;
    int   carModel_obj_map[10][5];
    opengl_projection_matrix projection;
    /*touch limit */
    float click_process_Time;
    int click_during_Count;
    /* seethrough */
    int seethrough_frame_read_flag; // 1, readOK, 0 , readfail
    float seetrough_frame_lineColor[3];
    float seethrough_sigmoid_alpha;
    float seethrough_top2d_alpha_center;
    float seethrough_top2d_alpha_range;
    float seethrough_resolution_level;
    /* carModelSetting */
    int light_mode;
    int sky_mode;
    int car_alpha_mode[4];
    /* mod version*/
    int mod_version;
    /* blur setting */
    PARA_BLUR blur_info;
}autosys_system_info;

typedef struct 
{
    uint8_t flag_line_gpu_refresh;
    /* data */
}LINE_INFO;
/**
 * @brief The autosys_current_info struct
 * 
 * @note current_info 結構體為 AVM 系統執行期間的基礎參數集合，會隨時被修改。主要提供畫面顯示、車型設定、投影矩陣、
 *       觸控處理等相關資訊。
 */
typedef struct 
{
    CAR_MODEL_INFO carmodel_info;
    AVM_INFO avm_info;
    LINE_INFO line_info;
    Point TOP2D_RANGE;
    Point TOP2D_LayoutPoint;
    opengl_projection_matrix projection;
    int cur_frame;
    int notify_show_cur_page_context;
    uint8_t cur_clear_drawpanel;
    /* touch */
    uint8_t mutiple_touch_index;
    /* seethrough */
    float seethrough_process;
    int seethrough_frameline_flag;
    /* canbus*/
    int flag_show_canbus_log;
    /* BB */
    int BB_PID_Latency;
    int BB_VERSION;
    /* disable ivi control*/
    int flag_disable_ivi_control;
    /* system init */
    int flag_system_init;

}autosys_current_info;

typedef struct 
{
   Point first_touch_point;
   Point least_touch_point;
   int flag_lock_2dAVM;
   int first_touch;
   int preious_status;
   int current_status;
}autosys_touch;

typedef struct
{
    GLuint programID;
    GLint attrVertexLocation;
    GLint attrUVLocation;
    GLint attrAlphaLocation;
    GLint attrTransformatiomMatrixLocation;
    int vao[10];
    int vbo[10][5];
    int textureID[10];
    float* vertexCoord[10];
    float* alphaValue[10];
    /* data */
}program_context;


typedef struct 
{
    uint8_t seethrough_finish;
    uint8_t seethrough_switch;
    float alpha_2d_car_value;
    float alpha_3d_car_value;
    float alpha_3d_car_tire_value;
    float alpha_3d_car_windows_value;
    float tune_shift;
    float tune_scale;
    /* data */
}tmp_seethrough;

typedef struct 
{
    /*[0] start, [1] end*/
    PointF TOP2D_Normlocation[2];
    Point TOP2D_realWorldlocation[2];
    VECF3 color;
    /* data */
}tmp_shadow;


typedef struct
{
    autosys_current_info current_info; // each page parameter
    autosys_system_info system_info; // put static parameter
    autosys_root_flag g_flag;

    autosys_page avm_page;

    zoom_birdView zoom;
    autosys_touch touch;
    tmp_seethrough seethrough;
    tmp_shadow shadow;

    program_context program_context[20];
    FBO_t frame_fbo[20];
}autosys_avm_package;


#endif //_AVM_STRUCT_H_