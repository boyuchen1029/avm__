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

#ifndef _AVM_PAGE_H_
#define _AVM_PAGE_H_

#include "AVM_PARAM.h"
//#include "AVM_STRUCT.h"
#include "../avm_module/avmDefine.h"

typedef unsigned char uint8_t; 
typedef signed char int8_t;
/*==============================================================
*                      OPENGL struct
===============================================================*/
typedef struct
{
	float start[2];
	float end[2];
	/* data */
} Scope;
typedef struct 
{
    float X, Y;
} PointF;

typedef struct 
{
    int X, Y;
} Point;

typedef union 
{
    float data[4];
    struct 
    {
        float X;
        float Y;
        float Z;
        float W;
        /* data */
    };
    /* data */
}VECF4;
typedef union 
{
    float data[3];
    struct 
    {
        float X;
        float Y;
        float Z;
        /* data */
    };
    /* data */
}VECF3;

typedef union 
{
    int data[4];
    struct 
    {
        int X;
        int Y;
        int W;
        int H;
        /* data */
    };
    
    /* data */
}Rectangle;

typedef struct 
{
   float theta;
   float phi;
   float eyeRadius;
   float x_position;
   float y_position;
   float z_position;
}virtual_cam;

// typedef struct 
// {
//     double shift_x;
//     double shift_y;
//     double shift_z;
//     double start_x;
//     double start_y;
//     double end_x;
//     double end_y;
// }cropped_view;

typedef union 
{
    float buffer[4];
    struct 
    {
        float x;
        float y;
        float z;
        float w;
    };
    /* data */
}axis_density;
/*==============================================================
*                      PAGE  struct
===============================================================*/
typedef enum
{
    PAGE_START,
    PAGE_3DAVM,
    PAGE_2DAVM,
    PAGE_SHADOW,
    PAGE_2DCAR,
    PAGE_CARMODEL,
    PAGE_PGL,
    PAGE_MOD,
    PAGE_FISHEYE,
    PAGE_SCENE,
    PAGE_TOUCH,
    PAGE_CARMODEL_STATIC,
    PAGE_EIGHT,
    PAGE_END,
    //PAGE_SEETHROUGH = PAGE_END + PAGE_SHADOW,
}eAVMPage;

typedef enum
{
    MODE_3DAVM,
    MODE_2DAVM,
}eProjectionMode;

typedef enum
{
    MODE_RUNTIME,
    MODE_STATIC,
}eShaderMode;

typedef enum
{
    EMPTY,
    FISHEYE,
    AVM2D,
    AVM3D,
    SCENE,
}ePGLMode;

typedef enum
{
    FISH_RAW,
}efisheyeMode;

typedef enum
{
    None      = 0,
    CIRCLE    = 2,
    FREEANGLE = 4,
    EIGHT     = 8,
    ANIMATION = 16,
}eRotateMode;

/*=======================*/
    /* global setting */
/*=======================*/
typedef struct 
{
    bool display_mod;
    bool display_seeth;
    bool display_avm3d;
    bool display_fisheye;
    bool disaply_fxaa;
    bool disaply_inner_sixtouch;
    bool display_scene;
    bool display_shadow_line;
}AVMflag;

typedef union 
{
    float buffer[30];
    struct
    {
        float lightNum;
        float lightBright[2];
        float lightPos[6];
        float lightColor[4];
    }para;
}set_global_carLight;


// typedef struct
// {
// 	int Flag_Enablerun;
// 	int Direction;
// 	int Cycle;
// 	float StartPoint;
// 	float EndPoint;
// 	float Time;
// }animation_theta_group;

typedef struct
{
	int Flag_Enablerun;
	int Direction;
	int Cycle;
	float StartPoint;
	float EndPoint;
	float Time;
}animation_para;

typedef struct
{
	animation_para theta_para;
    animation_para phi_para;
}animation_group;

typedef union 
{
    int buffer[61];
    struct
    {
        int TurnFlag;
        int Direction;
        int Cycle;
        float StartPoint;
        float EndPoint;
        float Time;
        int only_one_key;
    }para;

    struct
    {
		int TurnanimationFlag;
        animation_group group[3];
		int only_one_key;
    }para2;
}set_global_animation;

typedef union 
{
    int buffer[20];
    struct 
    {
        int colorRGBA[4];
        int width;
        int outRectangle[4];
        int inRectangle[4];
        int sensity;
        /* data */
    };
    /* data */
}set_global_mod;

typedef struct 
{
    FREETOUCH free;
    EIGHTFINGERTOUCH eight;
    FOURTEENFINGERTOUCH fourteen;
}set_global_touch;

typedef struct 
{
    uint8_t flag_seethrough_mode;
    uint8_t flag_img_mode;
}set_global_shadow;

typedef struct 
{
    set_global_carLight light;
}set_global_carmodel;

typedef struct
{
    int context_num;
    Rectangle viewport[PAGE_END];
    set_global_mod       mod;
    set_global_touch     touch;
    set_global_animation animation;
    set_global_shadow    shadow;
    set_global_carmodel  carmodel;
}global_setting;

/*=======================*/
        /* END*/
/*=======================*/
typedef struct 
{
    virtual_cam vcam;
    /* data */
    float empty[30];
}AVM_3D_BOWL;

typedef struct 
{
    /* data */
    virtual_cam vcam;
    float empty[30];
}AVM_2D_BOWL;
typedef struct 
{
    virtual_cam vcam;
    uint8_t flag_draw_show;
    float empty[30];
    /* data */
}AVM_SMOD;

typedef struct 
{
    /* data */
}AVM_ZOOM;

typedef struct 
{
    virtual_cam vcam;
    float empty[30];
    /* data */
}AVM_PGL;

typedef struct 
{
    int8_t Select_cam[4];
    uint8_t Select_style;
    ePGLMode mode;
    cropped_view cropped;
    int flag_draw_type;
    float y_limit[PARAM_PGL_MAX_STYLE_NUM];
    int flag_enable_patch[PARAM_PGL_MAX_STYLE_NUM];
    float rotate;
    /* data */
}PGL_SUPPLY;
typedef struct 
{
    virtual_cam vcam;
    float empty[30];
    /* data */
}AVM_FISHEYE;

typedef struct 
{
    int8_t Select_cam;
    efisheyeMode mode;
    cropped_view cropped;
    float rotate;
    /* data */
}FISHEYE_SUPPLY;


typedef struct 
{
    virtual_cam vcam;
    axis_density axis_density;
    float offset1[5];
    Car_item_Alpha alpha;
    CARMODEL_POWER power;
    float Tire_RotateAngle;
    float Car_RotateAngle;
    float emptyy;
    float empty[30];
    int *OffroadMode_displayItem; //之後修改
    /* data */
}AVM_CARMODEL;

typedef struct 
{
    float alpha;
    axis_density axis_density;
    float empty[30];
    /* data */
}AVM_2DCARMODEL;

typedef struct 
{
    virtual_cam vcam[2];
    float view_x_axis_ratio[2];
    float view_y_axis_ratio[2];
    float width_customized;
    float empty[25];
    /* data */
}AVM_SCENE;
typedef struct 
{
    int Select_virtualCam_id;
    int Style;
    /* data */
}SCENE_SUPPLY;

typedef struct 
{
    int Select_middle_view_id;
    int Select_cam;
    int CarModel_view[4];
    /* data */
}OFFROAD_SUPPLY;
typedef struct 
{
    /* data */
    virtual_cam vcam;
    float empty[30];
}AVM_SHADOW;
;
typedef struct
{
    AVM_3D_BOWL    avm_3d_bowl;
    AVM_2D_BOWL    avm_2d_bowl;
    AVM_SHADOW     avm_shadow;
    AVM_2DCARMODEL avm_2d_car;
    AVM_CARMODEL   avm_3d_car;
    AVM_SMOD       avm_mod;
    AVM_PGL        avm_pgl;
    AVM_FISHEYE    avm_fisheye;
    AVM_SCENE      avm_scene;
    /* data */
}avm_contexts;

typedef struct 
{
    uint8_t flag_projection_mode;
    PGL_SUPPLY sup_pgl;
    FISHEYE_SUPPLY sup_fisheye;
    SCENE_SUPPLY sup_scene;
    OFFROAD_SUPPLY sup_offroad;
    /* data */
}Information_supplement;

typedef struct
{
    uint8_t ID;
    Rectangle viewport;
    Information_supplement supply;
    /* data */
}avm_draw_list;

typedef struct 
{
    global_setting set;
    avm_contexts  contexts;
    avm_draw_list list[MAX_PAGE_LIST_NUM];
    /* data */
}ePage;

typedef struct
{
    int max_page;
    int current_page;
    int periously_page;
    char version[11];
    AVMflag flag;
    ePage page[MAX_PAGE_NUM + 2];
    /* data */
}autosys_page;


#endif //_AVM_STRUCT_H_