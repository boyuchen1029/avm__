/**
 *******************************************************************************
 * @file : program_line.h
 * @describe : Header of program_line.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211112 0.1.0 Linda.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/

#ifndef _PROGRAM_LINE_H_
#define _PROGRAM_LINE_H_

#include "../../../setup/AVM_STRUCT.h"
//#include "../8368-P/Demo/AVM_PARAM.h"
//reverse path define
#define DRAW_DISTORTION_LINE_MAX 5000
#define LINE_PARA_NUM_PACK 2



extern int AutoSys_mod_front_flag;
extern int AutoSys_mod_back_flag;
extern int AutoSys_mod_left_flag;
extern int AutoSys_mod_right_flag;

enum AVMVIEW
{
    VIEW_FISHEYE = 0,
    VIEW_AVM_3D = 1,
};

enum EDGEMODE
{
    ORIG_binnary = 2,
    TableLeft_ORIGRight = 3,
    ORIGLeft_TABLERight = 4,
    TABLELeft_TABLERight = 5,
};



typedef struct {
    int id;
    int x;
    int y;
    int hor_width;
    int ver_width;
} DYMAP;

typedef struct DynamicPara
{
    int totalPoint;
    double angleSpace;
    double angleStart;
    double xCenter;
    double yCenter;
    double radiusCenter;
    float  car_LONG_HALF;
    double wheelAngle;
    double wheelradius;
    double maxDrawAngle;
}DynamicPara;

// typedef struct {
//     int* lineNumber;
//     int** pixelcoordinate;
//     int** worldcoordinate;
// } STATIC_PACKAGE;

typedef struct
{
    int cam;
    int id;
    DynamicPara dynamicPara;
    double car_angle;
    double* w_y;
    double* w_x;
    double worldy;
    int avm_view;
    int director;
    PointF p1;
}TransForDylineData;

typedef struct {
    int cur_id;
    int* ver_width;
    PointF* junP1;
    PointF* junP2;
    DYMAP* map;
} JunctionItem;
typedef struct {
    int* ID;
    int** junctionP;
} DYNAMIC_TABLE;

typedef struct {
    float car_WHEELTOREAR;
    float car_WHEELBASE;
    float car_WHEELTOFRONT;

    int* lineNumber;
    int** line_strip;
    float** gl_colormap;
    float** linewidth;
    int** worldcoordinate;
    int** pixelcoordinate;
    int director;
    int current_style;
    int current_line;
    int current_cam;
    DYNAMIC_TABLE* table;
    int* map_size;
    JunctionItem* reference;
} DYNAMIC_PACKAGE;


typedef struct
{
    int g_finaly_draw_point;
    float limit_over_y;
    float limit_under_y;
    PointF normal;
}DRAW_LINE_FLAG;
       


/**
 * line paramter new sturcture for line
 * @date 2024/03/26
 * @author alfred
 * @package fishparamter, flag, user_define_line, carangle....
*/
typedef struct
{
    float vert[20000 * 3 * 2 * 2];
    float world[20000 * 2 * 3];
    // float* vert;
    // float* world;
    int indexNum;
    int cur_worldMaxIndex;
    int cur_vrtMaxIndex;
    int cur_glBufferLength;
    float textmap[20000 * 1];
    float texture[20000 * 2 * 2 * 2];
}line_vert_t;

// typedef struct
// {
//     float* vert;
//     float* world;
//     int indexNum;
// }line_vert_dynamic_t;

typedef struct 
{
    int imgHeight;
    int imgWidth;
    double fx;
    double fy;
    double cx;
    double cy;
    double k1;
    double k2;
    double k3;
    double k4;
}FishCamCalibartionPara;

typedef struct 
{
    FishCamCalibartionPara parameter[MAX_CAM_NUM];
}FISHEYEPARAMETER;

typedef struct 
{
    double AVM_TO_Pixel[4][16];
    double Pixel_TO_AVM[4][16];
    double GoldensampleRotate[4][9];
}HomoMatrix;

typedef struct
{
    // line_vert_t DynamicLineBuffer[5 * 2];
    // line_vert_t StylePatch[5 * 2];
    line_vert_t DynamicLineBuffer;
    line_vert_t StylePatch;
}LineBuffer;

typedef enum
{
    rectangle,
    circle,
}LineStyle;

typedef enum
{
    horizontal = 0,
    vertical = 1,
    slop = 2,
}DIRETOR;


typedef struct
{
    LineStyle drawstyle;
    DIRETOR director;
    ViewType type;
}VIEWS;

typedef struct
{
    int line_growth; // 0 is up and down; 1 is down (only);
    int g_finaly_draw_point;
    float limit_over_y;
    float limit_under_y;
    PointF normal;
}DRAW_LINE_RECTANGLE_PATHCH;

typedef struct
{
    int density;
    //List<Vector2>[] PointArray;
    float  radius[10000];
    float  ANGLE[10];
    PointF Point[100];
    int linetype[MAX_CAM_NUM][PARAM_AVM_DRAW_LINE_MAX];
}DRAW_LINE_CIRCLE_PATHCH;

typedef struct 
{
   int ON;
   int Width;
}ALIGNMENT;


typedef struct
{
    FISHEYEPARAMETER fisheye;
    HomoMatrix homo;
    LineBuffer buffer;
    VIEWS view;
    DRAW_LINE_RECTANGLE_PATHCH rect_patch;
    DRAW_LINE_CIRCLE_PATHCH circle_patch;
    ALIGNMENT alignment;
    int   g_is_need_update;
    float g_lineAngle;
    float g_lineWidthModifier;
    int   g_enable_y_axis_limit;
    int   g_PGL_FIFO_ON;
    float g_curTireAngle;
}DISPLAY_PGL;

typedef struct 
{
    double* x_axis;
    double* y_axis;
    float car_angle;
    int cam;
    int style;
    DISPLAY_PGL* display;
    line_vert_t* active;
    DIRETOR* linetype;
    float Top2Ddata[PARAM_AVM_DRAW_LINE_MAX][4];
    /* data */
}Cluster_calculater_Paramter;

// /* End */

extern void init_reverse_program(void);

extern void read_reverse_line(void);

extern void read_homo_matrix(void);

extern void Init_DISPLAY_PGL_PARAMETER();

extern void set_3D_20m_line(void);

extern void open_gl_init_reverse_line(void);

extern void draw_static_reverse_line(void);

extern void draw_dynamic_reverse_line(float carAngle);

extern void read_pPage_file(int mode);

// extern void init_mod_line(float carSizeW, float carSizeH, int w, int h);

// extern void upload_newMODline(int* modbuffer);

// extern void draw_mod();

extern void draw_20m_line(int avm2d);

extern void draw_adas(int adasCount, float *adas, float LR, int flag);

extern void draw_2davm_static_line(float *rotateReslut);

extern void draw_2davm_dynamic_lines(float carAngle, float *rotateReslut);

extern void draw_chassis(float *rotateReslut);

extern void read_reverse_line_v2(int camera);
extern void read_distortion_line(int camera);
extern void draw_static_reverse_line_v2(int camera);
extern void read_UI_line(int folder_index);
extern void draw_3d_avm_staticline(float *rotateReslut, int camera, int style);
extern void draw_2d_Distortion_staticline(float *rotateReslut, int camera, int style, void*);

extern void read_dynamic_line_user_data();
extern void read_static_line_user_data();
extern void read_static_scene_line_user_data();
extern void read_calib_fish_para();
extern void draw_3D_avm_dynamicline(float carAngle, float *rotateReslut, int camera, int style,  ViewType type, PGL_SUPPLY *sup_pgl);
extern void setLineAlignment(int enable, SET_CUR_VIEWDATA *aligmentdata);

extern void draw_fish_line(float carAngle, float *rotateReslut, int camera, int style, cropped_view* cropped, ViewType type,PGL_SUPPLY *sup_pgl, float pglRotateAngle);
extern void read_pgl_colorbar();
extern void setSceneViewCustomLineWidth(float width);
#endif //_PROGRAM_LINE_H_
