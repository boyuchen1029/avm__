#ifndef _AUTOSYS_H_
#define _AUTOSYS_H_

#include "../setup/AVM_STRUCT.h"
#include "../system.h"
#include "autosys_IVI.h"

static inline float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
typedef enum
{
    GET_INFO,
    SET_INFO,
}autosys_usr_control;

typedef struct {
    int lib_maj, lib_min;
    int tmaj_min, tmaj_max;
    int tmin_min, tmin_max;
} CompatRule;

static const CompatRule compat_table[] = {
    { 10, 0,   0,  0,   0,  0 },
    { 11, 0,  10, 10,   0,  3 },
    { 11, 1,  10, 10,   0,  3 },
    { 12, 0,  11, 11,   0,  0 },
    { 12, 1,  11, 11,   0,  2 },
    { 12, 2,  11, 11,   0,  3 },
    { 12, 5,  11, 11,   5,  5 },
    { 13, 0,  12, 12,   0,  1 },
    { 13, 1,  12, 12,   0,  1 },
    { 14, 0,  13, 13,   0,  0 },
    { 14, 1,  13, 13,   0,  0 },
    { 14, 2,  13, 13,   0,  1 },
    { 14, 3,  13, 13,   3,  3 },
    { 14, 4,  13, 13,   3,  3 },
    { 14, 5,  13, 13,   3,  3 },
    { 14, 6,  13, 13,   5,  5 },
    { 14, 7,  13, 13,   5,  5 },
    { 14, 8,  13, 13,   6,  6 },
    { 14, 9,  13, 13,   7,  7 },
    { 15, 0,  13, 13,   7,  7 },
    { 15, 1,  13, 13,   7,  7 },
};
static const size_t NUM_RULES = sizeof(compat_table) / sizeof(*compat_table);

extern autosys_avm_package autosys;

extern void Initial_autosys_avm_package(char* EVSpath);
extern void autosys_cal_bowl_shadow_range();
extern void avm_ePage_Init();
//extern void OldPage_to_NewPageConfig(stAVMPGLdrawView *oldPage);

/*update or assgin*/
extern void autosys_updata_zoominprojectionmatrix();
extern void autosys_assgin_2Dprojection2core(camera_para_t *para);
extern void autosys_assgin_3Dprojection2core(camera_para_t *para);
extern PointF autosys_get_realP_fromTouchPoint(float src_x, float src_y);
/*set*/
extern void autosys_set_zoomin2DprojectionmartixReset();
extern void autosys_set_zoomin2Dprojectionmatrix(int zoom_value, int sele_num);
extern void autosys_set_zoomin2DprojectionmatrixByVector(float xdiff,float ydiff, int zoom_value);
/*get*/
extern float *autosys_get_zoomin2Dprojectionmatrix();
extern float *autosys_get_zoomin2DCARprojectionmatrix();
extern Scope autosys_get_bowl_shadow_range();
extern camera_para_t *autosys_get_AVM3DProjectionmatrix();

extern void __AUTOSYS_CHECK_VERSION_STATUS__();

#endif //_AUTOSYS_H_