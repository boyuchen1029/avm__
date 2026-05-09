/**
 *******************************************************************************
 * @file : program_car.h
 * @describe : Header of program_car.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211116 0.1.0 Linda.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/

#ifndef _PROGRAM_CAR_H_
#define _PROGRAM_CAR_H_

#include "avmDefine.h"
#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
#define CAR_MODEL_NUMBER 2
#include "../../setup/AVM_PAGE.h"
#include "program/set_framebuffer.h"


#ifdef __cplusplus
extern "C" {
#endif

void upload_selection_car_color(int _ix);

#ifdef __cplusplus
}
#endif
//
typedef enum
{
    MODEL_CAR,
    MODEL_WHEEL_BL,
    MODEL_WHEEL_BR,
    MODEL_WHEEL_FL,
    MODEL_WHEEL_FR,
    MODEL_GLASS,
    MODEL_LAMP,
    MODEL_DOOR,
    MODEL_LICENCE,
    MODEL_INTERIOR,
    MODEL_CONCEALER,
    MODEL_OTHER,
}car_model_t;

typedef struct
{
    int name;
    float* shift;
    int TexId;
    int num;
}car_coord_3d_t;

typedef struct MEMPACKED         //定义一个mem occupy的结构体  
{  
    char name1[20];      //定义一个char类型的数组名name有20个元素
    unsigned long MemTotal;
    char name2[20];
    unsigned long MemFree;
    char name3[20];
    unsigned long Buffers;
    char name4[20];
    unsigned long Cached;
    char name5[20];
    unsigned long SwapCached;
}MEM_OCCUPY; 


//const char* Lamp_classify_name[] = { "FrontLamp", "RearLamp", "SideMirrorLamp"};
typedef enum
{
    Empty,
    FrontLamp_on_01,
    FrontLamp_on_02,
    FrontLamp_on_03,
    FrontLamp_off_01,
    FrontLamp_off_02,
    FrontLamp_off_03,
    FrontLamp_RESERVED_START,
    FrontLamp_RESERVED_END = FrontLamp_RESERVED_START + 11,
    FrontLamp,
    RearLamp_on_01,
    RearLamp_on_02,
    RearLamp_on_03,
    RearLamp_on_04,
    RearLamp_off_01,
    RearLamp_off_02,
    RearLamp_off_03,
    RearLamp_off_04,
    RearLamp_RESERVED_START,
    RearLamp_RESERVED_END = RearLamp_RESERVED_START + 11,
    RearLamp,
    SideMirrorLamp_on_01,
    SideMirrorLamp_off_01,
    SideMirrorLamp_RESERVED_START,
    SideMirrorLamp_RESERVED_END = SideMirrorLamp_RESERVED_START + 11,
    SideMirrorLamp,
    TotalLampIMG,
}car_lamp_status;


static char Lamp_Status_Name[][100] = {
    [Empty]                 = "Empty",
    [FrontLamp_on_01]       = "FrontLamp_on_01",
    [FrontLamp_on_02]       = "FrontLamp_on_02",
    [FrontLamp_on_03]       = "FrontLamp_on_03",
    [FrontLamp_off_01]      = "FrontLamp_off_01",
    [FrontLamp_off_02]      = "FrontLamp_off_02",
    [FrontLamp_off_03]      = "FrontLamp_off_03",
    [FrontLamp]             = "FrontLamp",
    
    [RearLamp_on_01]        = "RearLamp_on_01",
    [RearLamp_on_02]        = "RearLamp_on_02",
    [RearLamp_on_03]        = "RearLamp_on_03",
    [RearLamp_on_04]        = "RearLamp_on_04",
    [RearLamp_off_01]       = "RearLamp_off_01",
    [RearLamp_off_02]       = "RearLamp_off_02",
    [RearLamp_off_03]       = "RearLamp_off_03",
    [RearLamp_off_04]       = "RearLamp_off_04",
    [RearLamp]              = "RearLamp",
    
    [SideMirrorLamp_on_01]  = "SideMirrorLamp_on_01",
    [SideMirrorLamp_off_01] = "SideMirrorLamp_off_01",
    [SideMirrorLamp]        = "SideMirrorLamp",
};

/*Lamp*/
#define Lamp_obj_max  TotalLampIMG
#define Classify_LAMP_max  3
#define Classify_LAMP_mini_max  2

static char Lamp_Classify_Name[Classify_LAMP_max][50] = 
{ 
    {"front_lamp"}, 
    {"rear_lamp"}, 
    {"side_mirror_lamp"}, 
};
static char Lamp_classify_name_mini[Classify_LAMP_mini_max][50] = 
{ 
    {"(L)"}, 
    {"(R)"}, 
};

typedef struct 
{
    int Lamp_gl_ID[Lamp_obj_max];
    int Lamp_TexUnit[Lamp_obj_max];
    int maxGenIMG;
    char** Lamp_all_obj_name;
    char** Lamp_STD_obj_name;
    char* file_path;
}PACK_CAR_LAMP;

extern void init_car_program(void);

extern void car_mem_malloc(void);

extern void read_car_model(void);

extern void open_gl_init_car(void);

extern void open_gl_carModel_parameter(AXIS_DENSITY axis_density);

//extern void draw_car(camera_para_t para,  Car_item_Alpha *alpha, float Rotateangle, float carAngle, AXIS_DENSITY axis_density);
extern void draw_car(camera_para_t para, OPENGL_3D_CARMODEL_CONTEXT CarmodelContext);

extern void setting_CarModel_display(int ix, const int *item_flag);//Atlas20251002

extern void draw_car_2d(float _alpha);

extern void read_TESTIMG_func();
extern void upload_selection_car_color(int _ix);

extern void upload_car_lamp_status(int part, int id, int status);

extern void draw_runtime_car(camera_para_t para, AVM_CARMODEL *CarmodelContext, int* alphamode, int* froce_disable_item);

extern void open_gl_2dcarModel_parameter(axis_density axis_density);

extern void set_OIT_Idx(FBO_t* src);

extern void draw_runtime_car_oit(camera_para_t para, AVM_CARMODEL *CarmodelContext, int* alphamode, int* froce_disable_item);
extern void draw_pre_oit_real_car(camera_para_t para, AVM_CARMODEL *CarmodelContext, int* alphamode, int* froce_disable_item);
extern void draw_main_oit(bool useStaticIMG);
extern void draw_static_car_png(float _alpha, GLuint texture);
extern void init_flowing_init();
extern void backup_oit_main(bool backupStaticIMG);

// extern void draw_car(camera_para_t para, int vaoCar, int windows, Car_item_Alpha *alpha);
// extern void draw_tire(float * result3, float Rotateangle, float carAngle, Car_item_Alpha *alpha);
// extern void draw_Glass(camera_para_t para, int vaoCar, int windows,  Car_item_Alpha *alpha);

#endif //_PROGRAM_CAR_H_