#pragma once
#ifndef _BB_FUNC_
#define _BB_FUNC_

#include "../../../setup/AVM_STRUCT.h"

#define EPSILON 0.000001f

#define SAFE_VAL(val, default_val) do { \
    if (isnan(val) || isinf(val)) { \
        (val) = (default_val); \
        clear_color_diff_nan(); \
    } \
} while(0)

typedef enum
{
    BRIGNESS_BLANCES_LINEAR,
};

typedef struct
{


}BrignessBlanceAlgorithm;

typedef struct 
{
    float** frame_list;
    int* is_first_record;
    int* each_record_current_count;
    BrignessBlanceAlgorithm algorithm;
    /* data */
}BrignessBlance;

typedef struct 
{
    int data[3];
    struct 
    {
        int X;
        int Y;
        int Z;
        /* data */
    };
    
    /* data */
}VEC3;

typedef struct 
{
    Point sampler_point[20];
    float nor_value;
    float angle;
    /* data */
}vignetteConfig;

extern AvmBrightnessBalanceConfig g_avmBBConfig;

extern void clear_color_diff_nan();
extern void read_vignette_table();
extern void init_BrignessBlance();
extern void PID_BB_FLOW();
extern void AVM_PID_GROUP_DATA_Clear();
extern void caculate_color_offset(int isCameraFailure);
extern void calculate_color(void);
extern void clear_color_diff();
extern void main_mem_malloc(void);
extern void trans_cam_buffer_to_bb(unsigned char *cameraF, unsigned char *cameraB, unsigned char *cameraL, unsigned char *cameraR);
extern void trans_ReadFile_buffer_to_bb(unsigned char *cameraF, unsigned char *cameraB, unsigned char *cameraL, unsigned char *cameraR);
extern void trans_color_block_to_bb(int total_ov_area, int *colorblk[12]);
#endif // _BB_FUNC_