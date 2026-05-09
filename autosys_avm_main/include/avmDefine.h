/**
 *******************************************************************************
 * @file : avmDefine.h
 * @describe : .
 *
 * @author : Alfred
 * @verstion : 0.1.0.
 * @date 20230203 0.1.0 Alfred.
 *******************************************************************************
*/
#ifndef _AVMDEFINE_H_
#define _AVMDEFINE_H_

#include "define.h"
#include "stdbool.h"

/*2D AVM*/
// #define AVM_2D_CAR_limitH 200
// #define AVM_2D_CAR_limitW 100
// #define AVM_SD_REFERENCE_W 426
extern int rolate_360_flag;
extern int lock_page_roate;
/*touch panel*/
#define Capacitancepanel 0
#if !Capacitancepanel
#define ResistorPanel 1
#endif

#define touch_panel_on 1
#define ZaxisOpenView 1
#define avm_3D_MoveFunctionOpen 1 // touch 3D panel view open 
#define z_axis_ViewSensitive 1.5 // phi view sensitive  0 < v < 2
#define KEYPORTECT 0 // lock panel status
#define ReferAngle 270

#define fps_forward_count 30
#define max_ViewPage 16

#define DEBUG_MOVE_PANELVIEW 0
#ifdef DEBUG_MOVE_PANELVIEW
    #define CARPOINT_X 0
    #define CARPOINT_Y 0
    #define FACKPOINT1 0 // 0 is off; 1-2 find look at table
    #define MODESHOE 0
    #define PANELSTATUS_SHOW 0
#endif


#define Print(a) printf("%d, \n",(a))
#define Prflo(a) printf("%f, \n",(a))


/*****line UI*****/
#define lineDebug 1
#define maxPGLStyle 5
/*PGL*/


/*View Setting*/
#define maxAlphaItem 30

/*eume*/
typedef enum{

    CAL_THETA = 0,
    CAL_PHI,
    CAL_THETA_PHI,

}stCalcultorPanelMode;


/*touch struct*/
typedef struct 
{
    int x;
    int y;
    int offsetX;
    int offsetY;
    int car_minH;
    int car_minW;
    int car_centerX;
    int car_centerY;
    int init_thread;
    int init_para;
    int avm_2Dpanel_xlimit;

    int Key;

    float avmModlueTofbo_callibrationValue_W;
    float avmModlueTofbo_callibrationValue_H;

    float avmTorealAxis_callibrationValue_W;
    float avmTorealAxis_callibrationValue_H;

    // 3D
    int tempCenterX;
    int tempCenterY;
    float avm_3D_theta;
    float avm_3D_phi;
    float avm_3D_temptheta;
    float avm_3D_tempphi;


    int minCahngeDelta;
    unsigned int isFirstcount;

}stTouchData;

/*rectangle struct*/
typedef struct
{
    int Top;
    int Left;
    int Width;
    int Height;
}stRectangle;

/*View setting alpha parameter*/
typedef union 
{
    float alpha[maxAlphaItem];
    struct {
        float windows;
        float tires;
        float car;
        float empty[27];
    } item;
}Car_item_Alpha;

typedef union
{
    double buffer[7];
    struct
    {
        double shift_x;
        double shift_y;
        double shift_z;
        double start_x;
        double start_y;
        double end_x;
        double end_y;
    } para;
}stCamera;

typedef struct
{
    stCamera Fish_cam;  
    stCamera Car_body;  
    stCamera Empty;  
}Item_cam;

typedef struct 
{
    int SceneView_key;
    int CarBodyMoving_key;
    int FishCamScope_key;
}Panel_Key;



/*PGL struct*/
typedef struct
{
    bool MODEVIW[10]; // 0-> 2D-3D; 1-> fisheye, 2-10 empty buffer
    bool SUBVIEW[3]; // 0-> left; 1-> right, 3 empty buffer
    bool CAM[3][10]; // �M�w�n�}�������Y
    bool SHOW[3][10];// �M�w�i�ܭ�������
    bool  PGL[3][20]; // 0-> 2D Front-line, 1-> 3D Front-line, 2-> 2D back-line......
    double eyeRadius[3][1];
    float theta[3][1];
    float phi[3][1];
    double cam_x[3][1];
    double cam_y[3][1];
    double cam_z[3][1];
    int style;
    stRectangle viewport[3];
    Car_item_Alpha ItemApha[3];
    Item_cam itemCam[3];
    Panel_Key panel_key;
}stAVMPGLdrawView;

#endif // _AVMDEFINE_H_

extern void Get_Panel_Axis(stTouchData *data);
extern void Init_Panel_monitor();

