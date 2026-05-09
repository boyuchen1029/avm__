/**
 *******************************************************************************
 * @file : program_main.h
 * @describe : Header of program_main.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211116 0.1.0 Linda.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/

#ifndef _PROGRAM_MAIN_H_
#define _PROGRAM_MAIN_H_
#include "canbus.h"
#include "user_control/ui.h"
#include "../../../setup/AVM_STRUCT.h"
#define LINE_20M_STEP 50


#ifndef GL_VIV_NV12
#define GL_VIV_NV12 0x8FEE
#endif

#ifndef GL_VIV_YUY2
#define GL_VIV_YUY2 0x8FEF
#endif


typedef enum
{
    AVM_3D =  0,
    CAMERA = 1,
    CORRECT_CAMERA = 2,
    AVM_2D =  3,
}avm3d_windows_t;


typedef enum
{
    AVM =  0,
    AVM_RV = 1,
    NORMAL = 2,
}cameraPosition_t;

typedef struct
{
    float* vertice;
    float* texture;
    int num;
}gl_coord_t;

typedef struct
{
    float* vertice;
    float* texture;
    float* alpha;
    int num;
    int num_2d;
}gl_coord_3d_t;

typedef struct 
{
    int distribution[300];
    int max_box_index;
    int min_box_index;
    /* data */
}STACK_BB_BOX;



// extern unsigned char *adas_ims_image_buf;
extern void draw_adas_ims(cam_view_t detemineFisheye, cam_num_t cameraNumber); // IMS AND ADAS

extern void init_main_program(void);

extern void alloc_cam_buf(void);

extern void read_camera_file(void);

extern void read_file_vec_2d(int determinInitialCount);

extern void read_file_vec(int determinInitialCount);

extern void read_camera_file_one(void);

extern void get_camera(trans_camera_t transCamera, can_bus_info_t canbus);

extern void routine_camera(trans_camera_t transCamera, can_bus_info_t canbus);

extern void open_gl_init_3davm_cam2d(void);

extern void trans_cam_cpu_to_gpu(trans_camera_t *transCamera, can_bus_info_t canbus,int cam_ch);

extern void draw_3d_avm(float *rotateReslut, avm3d_windows_t windows,int cameraPosition);

extern void draw_cam_2d(cam_view_t detemineFisheye, cam_num_t cameraNumber,  void *);

extern void draw_cam_correct_2d(float *rotateReslut, cam_view_t detemineFisheye, cam_num_t cameraNumber, int style);

extern void draw_car_shadow(float *rotateReslut);

extern void draw_ADAS_icon(cam_num_t cameraNumber);

extern void draw_camera_warning(int position);

extern void Init_AVM_Camera_Warining();

extern void draw_car_shadow_seethrough(float *rotateReslut, Scope scope);

extern void draw_car_shadow_process_seethrough(float *rotateReslut, Scope scope);

extern void alloc_cam_one_buffer();

extern void draw_cam_2d_raw_yinverse(cam_view_t detemineFisheye, cam_num_t cameraNumber, void *cameraParameter);

void draw_cam_fish(cam_view_t detemineFisheye, cam_num_t cameraNumber, cropped_view *cameraParameter, float fishRotateAngle);

extern int get_fisheye_raw_textrueID(int cameraNumber);
extern IVI_WARNING ivi_warning;

#endif //_PROGRAM_MAIN_H_