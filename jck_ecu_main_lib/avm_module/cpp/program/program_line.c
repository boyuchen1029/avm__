/**
 *******************************************************************************
 * @file : program_line.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211116 0.1.0 Linda.
 * @date 20211213 0.1.1 Linda.
 *******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

#include "../../system.h"
#include "avm/maths.h"
#include "user_control/ui.h"
#include "avm/GLHeader.h"
#include FRAGSHADER
#include VERTSHADER
#include "canbus.h"
#include "avmDefine.h"
#include "mod/mod_main.h"
#include "avm/fp_source.h"
#include "gl/glShaderUtils.h"
#include "gl/glFunUtils.h"
#include "program/program_line.h"
#include "display_location.h"
#include "../../autosys/autosys.h"
#include "avm/stb_image.h"

DISPLAY_PGL Display_pgl;
DYNAMIC_PACKAGE dynamicData[LINE_PARA_NUM_PACK][MAX_CAM_NUM];
DYNAMIC_PACKAGE staticData[LINE_PARA_NUM_PACK][MAX_CAM_NUM];
DRAW_LINE_FLAG line_modify_flag;

// GL reverse line define
static GLuint programHandleReverseLine[1] = {0};
static GLint glAttrrlineVertex = 0;
// other line
static GLuint VAOLine[VAO_NUM]; // 0:MOD  , 1:ADAS
//static GLuint vertexbuffer_mod[2] = {0};
static GLuint vertexbuffer_adas[2] = {0};
// 3d/2d avm line
static GLuint VAO_3Davm_Line[PARAM_PGL_MAX_VAO_NUM]; 
static GLuint VBO_3Davm_line[PARAM_PGL_MAX_VAO_NUM][2];
// 2d avm line
static GLuint VAO_2Davm_Line[PARAM_PGL_MAX_VAO_NUM]; 
static GLuint VBO_2Davm_line[PARAM_PGL_MAX_VAO_NUM][2];
static GLuint VAO_2Davm_StylePatch_line[PARAM_PGL_MAX_VAO_NUM]; 
static GLuint VBO_2Davm_StylePatch_line[PARAM_PGL_MAX_VAO_NUM][2]; 

// fisheye distorion line
static GLuint VAO_fisheye_Line[PARAM_PGL_MAX_VAO_NUM]; 
static GLuint VBO_fisheye_line[PARAM_PGL_MAX_VAO_NUM][2];
static GLuint VAO_fisheye_StylePatch_line[PARAM_PGL_MAX_VAO_NUM]; 
static GLuint VBO_fisheye_StylePatch_line[PARAM_PGL_MAX_VAO_NUM][2]; 

// line style (circle & rectangle)
static GLuint VAO_StylePatch_line[PARAM_PGL_MAX_VAO_NUM]; 
static GLuint VBO_StylePatch_line[PARAM_PGL_MAX_VAO_NUM][2]; 
//reverse line color
const float gl_red[4]={1.0, 0.2, 0.0, 1.0};
const float gl_green[4]={0.0, 1.0, 0.0, 1.0};
const float gl_yellow[4]={1.0, 1.0, 0.0, 1.0};
const float gl_orange[4]={1.0, 0.5, 0.0, 1.0};
const float gl_gray[4]={0.5, 0.5, 0.5, 1.0};
extern float chassisColor[4];
double m_goldensample_diff_calibration[2] = {0};

static GLfloat lineADAS[20 * 4 * 3];
// static float modPosition[192];
// float m_mod_color[4] = {1.0,0.0,0.0,1.0};

float carAnglePrev[MAX_CAM_NUM];

int g_staticLine_Turn = 1;
int g_staticSceneLine_Turn = 1;
int g_dynamicLine_Turn = 1;
int g_2ddynamicLine_Turn = 1;
int g_static_fisheye_parameter_readOK = 0;

extern camera_para_t para2Davm;
SET_CUR_VIEWDATA *m_Aligment_data;
static int fisheye_avm_display_loading_flag[MAX_CAM_NUM][5] = {0};

static program_context glContext_ReverseLine;
static float* m_sceneview_projection_matrix = NULL;
static float m_sceneview_CustomlineWidth = 0.0f;

void init_reverse_program(void)
{
    AVM_LOGI("-----------------------------\n");
#if READ_OFFLINE_SHADER
    LoadBinary2Program(&glContext_ReverseLine.programID , "shader_program.bin_format_36805_0");
    if (glContext_ReverseLine.programID  == GL_NONE)
    {
        AVM_LOGI(LOG_RED("[ERROR]") "GL Loading ReverseLine offline file error!!!\n");
        AVM_LOGI("[WARNING] Using online program!!!\n");
        glContext_ReverseLine.programID  = create_program(vertexShader_line, fragmentShader_line, 0);
    }
#else
        glContext_ReverseLine.programID = create_program(vertexShader_line, fragmentShader_line,0);
        write_offline_shader_program(glContext_ReverseLine.programID, 0);
#endif

    glContext_ReverseLine.attrTransformatiomMatrixLocation = glGetUniformLocation(glContext_ReverseLine.programID , "transformatiomMatrix");
    glContext_ReverseLine.attrVertexLocation               = glGetAttribLocation(glContext_ReverseLine.programID , "position");
    glContext_ReverseLine.attrUVLocation                   = glGetAttribLocation(glContext_ReverseLine.programID , "InTexCoord");
    //glContext_ReverseLine.attrAlphaLocation                = glGetAttribLocation(glContext_ReverseLine.programID , "transformatiomMatrix");
    AVM_LOGI("AVM program_ReverseLine[0]: %d \n", glContext_ReverseLine.programID);
    AVM_LOGI("-----------------------------\n");
}

void Init_DISPLAY_PGL_PARAMETER()
{
    // Display_pgl.view.drawstyle = rectangle;
    // Display_pgl.rect_patch.line_growth = 1;
    // Display_pgl.g_enable_y_axis_limit = 1;

    Display_pgl.view.drawstyle = circle;
    Display_pgl.rect_patch.line_growth = 0;
    Display_pgl.g_enable_y_axis_limit = 0;

    memset(fisheye_avm_display_loading_flag[0], 0 , sizeof(fisheye_avm_display_loading_flag[0]));
    memset(fisheye_avm_display_loading_flag[1], 0 , sizeof(fisheye_avm_display_loading_flag[1]));
    memset(fisheye_avm_display_loading_flag[2], 0 , sizeof(fisheye_avm_display_loading_flag[2]));
    memset(fisheye_avm_display_loading_flag[3], 0 , sizeof(fisheye_avm_display_loading_flag[3]));
}

static void window_mat(double* camera, double *coorToPixel)
{
    double coor[3] = {coorToPixel[0], coorToPixel[1], coorToPixel[2]};

    coorToPixel[0] = (camera[0] * coor[0] + camera[4] * coor[1] + camera[8] * coor[2] + camera[12] * 1);
    coorToPixel[1] = (camera[1] * coor[0] + camera[5] * coor[1] + camera[9] * coor[2] + camera[13] * 1);
    coorToPixel[2] = (camera[2] * coor[0] + camera[6] * coor[1] + camera[10] * coor[2] + camera[14] * 1);

    coorToPixel[0] = coorToPixel[0] / coorToPixel[2];
    coorToPixel[1] = coorToPixel[1] / coorToPixel[2];
    coorToPixel[2] = 0.0;
}
int isHorizonline(float a, float b)
{
    float delta = a - b;
    return (delta > 0.0001 || delta < -0.0001)?0:1;
}
static void line_width(float *linePoint,float angle, float *point, int avm2d)
{
    float lineWidth=0.0;
    if(avm2d == 0)
    {
        lineWidth = LINEWIDTH_NOR;
    }
    else if(avm2d == 1)
    {
        lineWidth = LINEWIDTH_NOR_2D;
    }
    else
    {
        lineWidth = LINEWIDTH_NOR*2;
    }

    if(angle < 45.0)
    {
        if(avm2d == 0)
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1] + lineWidth/2.0;
            linePoint[2] = point[2];
            linePoint[3] = point[0];
            linePoint[4] = point[1] - lineWidth/2.0;
            linePoint[5] = point[2];
        }
        else if(avm2d == 1)
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1];
            linePoint[2] = point[2] + lineWidth/2.0;
            linePoint[3] = point[0];
            linePoint[4] = point[1];
            linePoint[5] = point[2] - lineWidth/2.0;
        }   
        else
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1] + lineWidth/2.0;
            linePoint[2] = point[2];
            linePoint[3] = point[0];
            linePoint[4] = point[1] - lineWidth/2.0;
            linePoint[5] = point[2];
        }        
    }
    else
    {
        linePoint[0] = point[0] - lineWidth/2.0;
        linePoint[1] = point[1];
        linePoint[2] = point[2];
        linePoint[3] = point[0] + lineWidth/2.0;
        linePoint[4] = point[1];
        linePoint[5] = point[2];
    }
}
static void set_color(int selectColor)
{
    float *ptr_color = NULL;
    if(selectColor == GL_COLOR_RED)
    {
        ptr_color = (float *) gl_red;
    }
    else if(selectColor == GL_COLOR_YELLOW)
    {
        ptr_color = (float *) gl_yellow;
    }
    else if(selectColor == GL_COLOR_GREEN)
    {
        ptr_color = (float *) gl_green;
    }
    else if(selectColor == GL_COLOR_ORAHGE)
    {
        ptr_color = (float *) gl_orange;
    }
    else if(selectColor == GL_COLOR_GRAY)
    {
        ptr_color = (float *) gl_gray;
    }
 
    glUniform4fv(glGetUniformLocation(programHandleReverseLine[0], "color"), 1, ptr_color);
}



/*======================================
*                                      *            
*        word transfer function        *    
*                                      *
=======================================*/
void world2cam(double* wp, double *camp, int cam)
{
    double x, y;
    double in_x, in_y, in_z, fx, fy, cx, cy, k1, k2, k3, k4, r, theta, theta2, theta4, theta6, theta8, theta_d, scale, out_x, out_y;

    in_x = wp[0]; //return Fish.calib2fish(out.y, out.x); /*rocver*/
    in_y = wp[1];
    in_z = wp[2];

    //FishCamCalibartionPara *fishpara = &Display_pgl.fisheye.parameter;

	fx = Display_pgl.fisheye.parameter[cam].fx;
	fy = Display_pgl.fisheye.parameter[cam].fy;
	cx = Display_pgl.fisheye.parameter[cam].cx;
	cy = Display_pgl.fisheye.parameter[cam].cy;
	k1 = Display_pgl.fisheye.parameter[cam].k1;
	k2 = Display_pgl.fisheye.parameter[cam].k2;
	k3 = Display_pgl.fisheye.parameter[cam].k3;
	k4 = Display_pgl.fisheye.parameter[cam].k4;
	x = (in_x - cx + 0) / fx;
	y = (in_y - cy + 0) / fy;

	r = sqrt(x * x + y * y);
	theta = atan(r);
	theta2 = theta * theta;
	theta4 = theta2 * theta2;
	theta6 = theta4 * theta2;
	theta8 = theta4 * theta4;
	theta_d = theta * (1 + k1 * theta2 + k2 * theta4 + k3 * theta6 + k4 * theta8);
	scale = (r == 0) ? 1.0 : theta_d / r;
	out_x = fx * x * scale + cx + 0;
	out_y = fy * y * scale + cy + 0;
	camp[0] = out_x;
	camp[1] = out_y;
}

/*======================================
*                                      *            
*        MOD paramter setting          *    
*                                      *
=======================================*/


static void set_mod_line_width(float *linePoint,float angle, float *point, int avm2d, float width, int slop_flag)
{
    float lineWidth=0.0;
    int screen_width = SCREEN_W;
    int screen_height = SCREEN_H;
    float ratio = (float)screen_height / (float)screen_width;
    if(avm2d == 0)
    {
        lineWidth = width * 2.0f / 400.0f;
    }


    if(slop_flag > 0)
    {
        float ndc_coord_lenght_x = lineWidth / 2.0f * ratio;
        float ndc_coord_lenght_y = lineWidth / 2.0f * 0.5f;

        if (slop_flag == 1)
        {
            linePoint[0] = point[0] - ndc_coord_lenght_x;
            linePoint[1] = point[1] + ndc_coord_lenght_y * 2.0f;
            linePoint[2] = point[2];
            linePoint[3] = point[0] + ndc_coord_lenght_x;
            linePoint[4] = point[1];
            linePoint[5] = point[2];

        }
        else if(slop_flag == 2)
        {
            linePoint[0] = point[0] - ndc_coord_lenght_x;
            linePoint[1] = point[1] - ndc_coord_lenght_y * 2.0f;
            linePoint[2] = point[2];
            linePoint[3] = point[0] + ndc_coord_lenght_x;
            linePoint[4] = point[1];
            linePoint[5] = point[2];
        }
        else if (slop_flag == 3)
        {
            linePoint[0] = point[0] - ndc_coord_lenght_x;
            linePoint[1] = point[1] ;
            linePoint[2] = point[2];
            linePoint[3] = point[0] + ndc_coord_lenght_x;
            linePoint[4] = point[1] + ndc_coord_lenght_y * 2.0f;
            linePoint[5] = point[2];
        }
        else
        {
            linePoint[0] = point[0] - ndc_coord_lenght_x;
            linePoint[1] = point[1];
            linePoint[2] = point[2];
            linePoint[3] = point[0] + ndc_coord_lenght_x;
            linePoint[4] = point[1] - ndc_coord_lenght_y * 2.0f;
            linePoint[5] = point[2];
        }

        return;
    }


    if(angle < 45.0)
    {
        if(avm2d == 0)
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1] + lineWidth/2.0;
            linePoint[2] = point[2];
            linePoint[3] = point[0];
            linePoint[4] = point[1] - lineWidth/2.0;
            linePoint[5] = point[2];
        }
        else if(avm2d == 1)
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1];
            linePoint[2] = point[2] + lineWidth/2.0;
            linePoint[3] = point[0];
            linePoint[4] = point[1];
            linePoint[5] = point[2] - lineWidth/2.0;
        }   
        else
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1] + lineWidth/2.0;
            linePoint[2] = point[2];
            linePoint[3] = point[0];
            linePoint[4] = point[1] - lineWidth/2.0;
            linePoint[5] = point[2];
        }        
    }
    else
    {
        linePoint[0] = point[0] - lineWidth/2.0f*ratio;
        linePoint[1] = point[1];
        linePoint[2] = point[2];
        linePoint[3] = point[0] + lineWidth/2.0f*ratio;
        linePoint[4] = point[1];
        linePoint[5] = point[2];
    }
}

/*======================================
*                                      *            
*        read init paramter setting    *    
*              like txt                *
=======================================*/

void read_homo_matrix(void)
{
    char read_homo_matrix_path[1024];
    
    FILE *fp_homoMatrixCoorToPixelIN       = fp_source_app("EVSTable/pgl/makeline_matrial/homoMatrixCoorToPixel.bin", "r");
    FILE *fp_homoMatrixPositionIN          = fp_source_app("EVSTable/pgl/makeline_matrial/homoMatrixPosition.bin", "r");
    FILE *fp_Front_homoMatrixCoorToPixelIN = fp_source_app("EVSTable/pgl/makeline_matrial/Front_homoMatrixCoorToPixel.bin", "r");
    FILE *fp_Front_homoMatrixPositionIN    = fp_source_app("EVSTable/pgl/makeline_matrial/Front_homoMatrixPosition.bin", "r");

    FILE *fp_Left_homoMatrixCoorToPixelIN   = fp_source_app("EVSTable/pgl/makeline_matrial/Left_homoMatrixCoorToPixel.bin", "r");
    FILE *fp_Left_homoMatrixPositionIN      = fp_source_app("EVSTable/pgl/makeline_matrial/Left_homoMatrixPosition.bin", "r");
    FILE *fp_Right_homoMatrixCoorToPixelIN  = fp_source_app("EVSTable/pgl/makeline_matrial/Right_homoMatrixCoorToPixel.bin", "r");
    FILE *fp_Right_homoMatrixPositionIN     = fp_source_app("EVSTable/pgl/makeline_matrial/Right_homoMatrixPosition.bin", "r");

    if(fp_homoMatrixCoorToPixelIN == NULL || fp_homoMatrixPositionIN == NULL ||
       fp_Front_homoMatrixCoorToPixelIN == NULL || fp_Front_homoMatrixPositionIN == NULL ||
       fp_Left_homoMatrixCoorToPixelIN == NULL  || fp_Left_homoMatrixPositionIN == NULL ||
       fp_Right_homoMatrixCoorToPixelIN == NULL || fp_Right_homoMatrixPositionIN == NULL)
    {
        SET_STATUS_CALIBRATION_IVI(pgl, ADNORMAL_APP, 1, DATA_NONFIND_OR_EMPTY);
    }

    FILE *fp_CoorToPixelIN_Array[] = {fp_Front_homoMatrixCoorToPixelIN, fp_homoMatrixCoorToPixelIN, fp_Left_homoMatrixCoorToPixelIN, fp_Right_homoMatrixCoorToPixelIN};
    FILE *fp_PositionIN_Array[] = {fp_Front_homoMatrixPositionIN, fp_homoMatrixPositionIN, fp_Left_homoMatrixPositionIN, fp_Right_homoMatrixPositionIN};

    for(int cam = 0; cam < MAX_CAM_NUM; cam++)
    {
        fp_read_data_safty(Display_pgl.homo.AVM_TO_Pixel[cam], 1, 16 * sizeof(double), fp_CoorToPixelIN_Array[cam]);
        fp_read_data_safty(Display_pgl.homo.Pixel_TO_AVM[cam], 1, 16 * sizeof(double), fp_PositionIN_Array[cam]);
        fp_close_data_safty(fp_CoorToPixelIN_Array[cam]);
        fp_close_data_safty(fp_PositionIN_Array[cam]);
    }

    // fp_read_data_safty(Display_pgl.homo.AVM_TO_Pixel[0], 1, 16 * sizeof(double), fp_Front_homoMatrixCoorToPixelIN);
    // fp_read_data_safty(Display_pgl.homo.Pixel_TO_AVM[0], 1, 16 * sizeof(double), fp_Front_homoMatrixPositionIN);
    // fp_read_data_safty(Display_pgl.homo.AVM_TO_Pixel[1], 1, 16 * sizeof(double), fp_homoMatrixCoorToPixelIN);
    // fp_read_data_safty(Display_pgl.homo.Pixel_TO_AVM[1], 1, 16 * sizeof(double), fp_homoMatrixPositionIN);
    // fp_close_data_safty(fp_Front_homoMatrixCoorToPixelIN);
    // fp_close_data_safty(fp_Front_homoMatrixPositionIN);
    // fp_close_data_safty(fp_homoMatrixCoorToPixelIN);
    // fp_close_data_safty(fp_homoMatrixPositionIN);
    FILE *fgoldensample = fp_source_avm_nonCRC("EVSTable/golden/goldensample_result.bin", "rb", 1);
    if(fp_read_data_safty(m_goldensample_diff_calibration, 2, sizeof(double), fgoldensample)!= 1)
    {
        m_goldensample_diff_calibration[0] = 0.0;
        m_goldensample_diff_calibration[1] = 0.0;
    }
    double unitmatrix[9] = 
    {
        1.0,0.0,0.0,
        0.0,1.0,0.0,
        0.0,0.0,1.0,
    };
    if(PARAM_PGL_ALIGMENT_GOLDENVERSTION == 1)
    {
        memcpy(Display_pgl.homo.GoldensampleRotate[2], unitmatrix, sizeof(double) * 9);
        memcpy(Display_pgl.homo.GoldensampleRotate[3], unitmatrix, sizeof(double) * 9);
        if(m_goldensample_diff_calibration[0] == 0.0 && m_goldensample_diff_calibration[1] == 0.0)
        {
            AVM_LOGI("\033[31m using default unitmatrix \033[0m\n");
            memcpy(Display_pgl.homo.GoldensampleRotate[0], unitmatrix, sizeof(double) * 9);
            memcpy(Display_pgl.homo.GoldensampleRotate[1], unitmatrix, sizeof(double) * 9);
            memcpy(Display_pgl.homo.GoldensampleRotate[2], unitmatrix, sizeof(double) * 9);
            memcpy(Display_pgl.homo.GoldensampleRotate[3], unitmatrix, sizeof(double) * 9);
        }
        else
        {
            AVM_LOGI("\033[31m using modifymatrix \033[0m\n");
            m_goldensample_diff_calibration[0] = 0.0;
            m_goldensample_diff_calibration[1] = 0.0;
            FILE *fgoldensampleHmtrix = fp_source_avm_nonCRC("EVSTable/golden/current_sample_HF.bin", "rb", false);
            if(fgoldensampleHmtrix != NULL)
            {
                fp_read_data_safty(Display_pgl.homo.GoldensampleRotate[0], 1, 9 * sizeof(double), fgoldensampleHmtrix);
                fp_close_data_safty(fgoldensampleHmtrix);
                fgoldensampleHmtrix = fp_source_avm_nonCRC("EVSTable/golden/current_sample_HB.bin", "rb", false);
                fp_read_data_safty(Display_pgl.homo.GoldensampleRotate[1], 1, 9 * sizeof(double), fgoldensampleHmtrix);
                fp_close_data_safty(fgoldensampleHmtrix);
            }
            else
            {
                memcpy(Display_pgl.homo.GoldensampleRotate[0], unitmatrix, sizeof(double) * 9);
                memcpy(Display_pgl.homo.GoldensampleRotate[1], unitmatrix, sizeof(double) * 9);
                memcpy(Display_pgl.homo.GoldensampleRotate[2], unitmatrix, sizeof(double) * 9);
                memcpy(Display_pgl.homo.GoldensampleRotate[3], unitmatrix, sizeof(double) * 9);
            }
        }
    }
    else
    {
        memcpy(Display_pgl.homo.GoldensampleRotate[0], unitmatrix, sizeof(double) * 9);
        memcpy(Display_pgl.homo.GoldensampleRotate[1], unitmatrix, sizeof(double) * 9);
        memcpy(Display_pgl.homo.GoldensampleRotate[2], unitmatrix, sizeof(double) * 9);
        memcpy(Display_pgl.homo.GoldensampleRotate[3], unitmatrix, sizeof(double) * 9);
    }

    AVM_LOGI("read homo matrix end\n");
}

void free_pgl_data_if_exist(int mode)
{
    switch(mode)
    {
        case FISHEYE_STATIC_SCENE:
            for (int camM = 0; camM < MAX_CAM_NUM; camM++)
            {
                DYNAMIC_PACKAGE *pack = &staticData[1][camM];
                if (!pack) continue; 

                #define SAFE_FREE_NESTED(ptr_array, size) \
                    if (ptr_array != NULL) { \
                        for (int i = 0; i < size; i++) { \
                            if (ptr_array[i] != NULL) { free(ptr_array[i]); ptr_array[i] = NULL; } \
                        } \
                        free(ptr_array); \
                        ptr_array = NULL; \
                    }

                int current_style_count = 1; 

                SAFE_FREE_NESTED(pack->line_strip, current_style_count);
                SAFE_FREE_NESTED(pack->gl_colormap, current_style_count);
                SAFE_FREE_NESTED(pack->linewidth, current_style_count);
                SAFE_FREE_NESTED(pack->worldcoordinate, current_style_count);
                SAFE_FREE_NESTED(pack->pixelcoordinate, current_style_count);
            }
            break;
    }
}
void read_pPage_file(int mode)
{
    #if true
    g_static_fisheye_parameter_readOK = 0;
    static int prePage = -1;
    static int preMode = -1;

    int curPage = autosys.avm_page.current_page;
    if (curPage < 0 || curPage > MAX_PAGE_NUM) {
        curPage = 0; 
    }
    char pPage_path[256];
    sprintf(pPage_path, "EVSTable/pgl/Each/pPage%d.bin",  curPage);
    if(prePage != curPage || preMode != mode)
    {
        prePage = curPage;
        preMode = mode;
        switch(mode)
        {
            case FISHEYE_STATIC_SCENE:
                free_pgl_data_if_exist(FISHEYE_STATIC_SCENE);
                FILE *file = fp_source_app(pPage_path, "rb");
                for(int camM = 0; camM < MAX_CAM_NUM; camM++)
                {
                    if (file == NULL)
                    {
                        AVM_LOGI("\033[31m[AutoSys] fail.......\033[0m\n");
                        AVM_LOGI("[AutoSys] Read scene_line fail in [%s].......\n", pPage_path);
                        AVM_LOGI("[AutoSys] scene_line parameter no reading, So cencel the program\n");
                        if (camM == 2 || camM == 3)
                        {
                            g_staticSceneLine_Turn &= 0;
                        }
                        continue;
                    }
                    DYNAMIC_PACKAGE *pack = &staticData[1][camM];
                    AVM_LOGI("[READ FILE] Current Read scene file in [%s].......\n", pPage_path);
                    AVM_LOGI("[READ FILE] Current CAM is [%d]\n", camM);
                    // Read lineNumber array
                    int lineNumberSize = 1; //only one style
                    pack->lineNumber = (int *)malloc(lineNumberSize * sizeof(int));
                    fp_read_data_safty(pack->lineNumber, sizeof(int), lineNumberSize, file);

                    pack->line_strip      = (int **)malloc(lineNumberSize * sizeof(int *));
                    pack->gl_colormap     = (float **)malloc(lineNumberSize * sizeof(float *));
                    pack->linewidth       = (float **)malloc(lineNumberSize * sizeof(float *));
                    pack->worldcoordinate = (int **)malloc(lineNumberSize * sizeof(int *));
                    pack->pixelcoordinate = (int **)malloc(lineNumberSize * sizeof(int *));
                    for (int style = 0; style < 1; style++)
                    {
                        int line_number = pack->lineNumber[style];
                        pack->line_strip[style]      = (int *)malloc(line_number * sizeof(int));
                        pack->gl_colormap[style]     = (float *)malloc(line_number * 4 * sizeof(float));
                        pack->linewidth[style]       = (float *)malloc(line_number * sizeof(float));
                        pack->worldcoordinate[style] = (int *)malloc(line_number * 4 * sizeof(int));
                        pack->pixelcoordinate[style] = (int *)malloc(line_number * 4 * sizeof(int));
                        int buffer[4];

                        int color_idx = 0;
                        int linew_idx = 0;
                        int world_idx = 0;
                        int pixel_idx = 0;
                        for(int line_index = 0; line_index < line_number; line_index++)
                        {
                            fp_read_data_safty(buffer,   sizeof(int), 4, file);
                            float fColor[4] = {(float)buffer[1] / 255.0f, (float)buffer[2] / 255.0f, (float)buffer[3] / 255.0f, (float)buffer[0] / 255.0f};
                            memcpy(&pack->gl_colormap[style][color_idx], fColor, sizeof(float) * 4);         
                            fp_read_data_safty(&pack->linewidth[style][linew_idx],   sizeof(float), 1, file);

                            fp_read_data_safty(buffer,   sizeof(int), 4, file);

                            fp_read_data_safty(&pack->worldcoordinate[style][world_idx],   sizeof(int), 4, file);
                            fp_read_data_safty(&pack->pixelcoordinate[style][pixel_idx],   sizeof(int), 4, file);

                            if(line_index == 0)
                            {
                                AVM_LOGI("[READ FILE] line_number is [%d]\n", line_number);
                                AVM_LOGI("[READ FILE] line_color is [%f, %f, %f, %f]\n", pack->gl_colormap[style][color_idx], pack->gl_colormap[style][color_idx + 1], pack->gl_colormap[style][color_idx + 2], pack->gl_colormap[style][color_idx + 3]);
                                AVM_LOGI("[READ FILE] line_width is [%f]\n", pack->linewidth[style][linew_idx]);
                                AVM_LOGI("[READ FILE] worldcoordinate is [%d, %d, %d, %d]\n", pack->worldcoordinate[style][world_idx], pack->worldcoordinate[style][world_idx + 1], pack->worldcoordinate[style][world_idx + 2], pack->worldcoordinate[style][world_idx + 3]);
                                AVM_LOGI("[READ FILE] pixelcoordinate is [%d, %d, %d, %d]\n", pack->pixelcoordinate[style][pixel_idx], pack->pixelcoordinate[style][pixel_idx + 1], pack->pixelcoordinate[style][pixel_idx + 2], pack->pixelcoordinate[style][pixel_idx + 3]);
                            }
                            color_idx += 4;
                            linew_idx += 1;
                            world_idx += 4;
                            pixel_idx += 4;
                        }
                    }
                }
                fp_close_data_safty(file);
                AVM_LOGI("[READ FILE] Success Read scene_line in [%s].......\n", pPage_path);
                break;
        }
    }
    g_static_fisheye_parameter_readOK = 1;
    #endif
}

/*======================================
*                                      *            
*        init opengl paramter setting  *    
*              VAO VBO                 *
=======================================*/
void open_gl_init_reverse_line(void)
{
    GLint program      = glContext_ReverseLine.programID;
    GLint glAttrVertex = glContext_ReverseLine.attrVertexLocation;
    GLint glAttrUV     = glContext_ReverseLine.attrUVLocation;
    glUseProgram(program);
    //glAttrrlineVertex = glGetAttribLocation(glContext_ReverseLine.programID, "position");

    glGenVertexArrays(VAO_NUM, VAOLine);

    // glGenBuffers(1, &vertexbuffer_mod[0]);
    // {
    //     glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_mod[0]);                            
    //     glBufferData(GL_ARRAY_BUFFER, 200*3*sizeof(float), NULL, GL_DYNAMIC_DRAW);

    //     glBindVertexArray(VAOLine[0]);
    //         glEnableVertexAttribArray(glAttrrlineVertex);
    //         glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_mod[0]);
    //         glVertexAttribPointer(glAttrrlineVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    // }
    // glBindVertexArray(0);

    glGenBuffers(1, &vertexbuffer_adas[0]);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_adas[0]);                            
        glBufferData(GL_ARRAY_BUFFER, 20*4*3*sizeof(float), NULL, GL_DYNAMIC_DRAW);

        glBindVertexArray(VAOLine[1]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_adas[0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    }
    glBindVertexArray(0);


    /**
     *  @brief line style patch // end point style
     */
    // VBO_StylePatch_line
    glGenVertexArrays(PARAM_PGL_MAX_VAO_NUM, VAO_3Davm_Line);
    glGenVertexArrays(PARAM_PGL_MAX_VAO_NUM, VAO_2Davm_Line);
    glGenVertexArrays(PARAM_PGL_MAX_VAO_NUM, VAO_fisheye_Line);

    for (int cam = 0; cam < MAX_CAM_NUM; cam++)
    {
        int style = 0;
        //for (int style = 0; style < 5; style++)
        {
            int VAO_index = cam * MAX_CAM_NUM + style;

        GLuint *VAO_arrays[3]      = {VAO_3Davm_Line, VAO_2Davm_Line, VAO_fisheye_Line};
        GLuint (*VBO_arrays[3])[2] = {VBO_3Davm_line, VBO_2Davm_line, VBO_fisheye_line};

        for (int j = 0; j < 3; j++) 
        {
            // Front camera
            glGenBuffers(2, &VBO_arrays[j][VAO_index][0]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index][0]);
            glBufferData(GL_ARRAY_BUFFER, DRAW_DISTORTION_LINE_MAX  * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index][1]);
            glBufferData(GL_ARRAY_BUFFER, DRAW_DISTORTION_LINE_MAX  * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO_arrays[j][VAO_index]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index][0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index][1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindVertexArray(0);             

            // Back camera
            glGenBuffers(2, &VBO_arrays[j][VAO_index + 1][0]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 1][0]);
            glBufferData(GL_ARRAY_BUFFER, DRAW_DISTORTION_LINE_MAX * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 1][1]);
            glBufferData(GL_ARRAY_BUFFER, DRAW_DISTORTION_LINE_MAX  * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO_arrays[j][VAO_index + 1]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 1][0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 1][1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindVertexArray(0);    

            // left camera
            glGenBuffers(2, &VBO_arrays[j][VAO_index + 2][0]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 2][0]);
            glBufferData(GL_ARRAY_BUFFER, DRAW_DISTORTION_LINE_MAX * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 2][1]);
            glBufferData(GL_ARRAY_BUFFER, DRAW_DISTORTION_LINE_MAX  * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO_arrays[j][VAO_index + 2]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 2][0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 2][1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindVertexArray(0);        

            // right camera
            glGenBuffers(2, &VBO_arrays[j][VAO_index + 3][0]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 3][0]);
            glBufferData(GL_ARRAY_BUFFER, DRAW_DISTORTION_LINE_MAX * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 3][1]);
            glBufferData(GL_ARRAY_BUFFER, DRAW_DISTORTION_LINE_MAX  * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO_arrays[j][VAO_index + 3]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 3][0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 3][1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindVertexArray(0);    
        }
    }
    }

    /**
     *  @brief line style patch // end point style
    */
   //VBO_StylePatch_line
    glGenVertexArrays(PARAM_PGL_MAX_VAO_NUM, VAO_StylePatch_line);
    glGenVertexArrays(PARAM_PGL_MAX_VAO_NUM, VAO_fisheye_StylePatch_line);
    glGenVertexArrays(PARAM_PGL_MAX_VAO_NUM, VAO_2Davm_StylePatch_line);

    for (int cam = 0; cam < MAX_CAM_NUM; cam++)
    {
        int style = 0;
        //for (int style = 0; style < 5; style++)
        {
            int VAO_index = cam * MAX_CAM_NUM + style;

        GLuint *VAO_arrays[3]      = {VAO_StylePatch_line, VAO_2Davm_StylePatch_line, VAO_fisheye_StylePatch_line};
        GLuint (*VBO_arrays[3])[2] = {VBO_StylePatch_line, VBO_2Davm_StylePatch_line, VBO_fisheye_StylePatch_line};

        for (int j = 0; j < 3; j++) 
        {
            // Front camera
            glGenBuffers(2, &VBO_arrays[j][VAO_index][0]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index][0]);
            glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index][1]);
            glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO_arrays[j][VAO_index]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index][0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index][1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindVertexArray(0);             

            // Back camera
            glGenBuffers(2, &VBO_arrays[j][VAO_index + 1][0]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 1][0]);
            glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 1][1]);
            glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO_arrays[j][VAO_index + 1]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 1][0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 1][1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindVertexArray(0);          

            // left camera
            glGenBuffers(2, &VBO_arrays[j][VAO_index + 2][0]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 2][0]);
            glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 2][1]);
            glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO_arrays[j][VAO_index + 2]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 2][0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 2][1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindVertexArray(0);   

            // right camera
            glGenBuffers(2, &VBO_arrays[j][VAO_index + 3][0]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 3][0]);
            glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 3][1]);
            glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glBindVertexArray(VAO_arrays[j][VAO_index + 3]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 3][0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_arrays[j][VAO_index + 3][1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindVertexArray(0);     
        }
    }
    }
}


/*======================================
*                                      *            
*        linevert paramter setting     *    
*                                      *
=======================================*/


#if 0
void line_buffer_allocate(line_vert_t* buffer, int indexNum, int checkType)
{
    if (!buffer || indexNum <= 0) return;
    float* newPtr = NULL;

    switch (checkType)
    {
    case 0: // world
        if (buffer->world == NULL)
        {
            buffer->world = (float *)calloc(indexNum, sizeof(float));
            buffer->cur_worldMaxIndex = indexNum;
        }
        else
        {
            if (buffer->cur_worldMaxIndex < indexNum)
            {
                newPtr = (float *)calloc(indexNum, sizeof(float));
                if (newPtr)
                {
                    memcpy(newPtr, buffer->world, buffer->cur_worldMaxIndex * sizeof(float));
                    free(buffer->world);
                    buffer->world = newPtr;
                    buffer->cur_worldMaxIndex = indexNum;
                }
            }
        }

        break;
    case 1: // vert
        if (buffer->vert == NULL)
        {
            buffer->vert = (float *)calloc(indexNum, sizeof(float));
            buffer->cur_vrtMaxIndex = indexNum;
        }
        else
        {
            if (buffer->cur_vrtMaxIndex < indexNum)
            {
                AVM_LOGI("using\n");
                newPtr = (float *)calloc(indexNum, sizeof(float));
                if (newPtr)
                {
                    memcpy(newPtr, buffer->vert, buffer->cur_vrtMaxIndex * sizeof(float));
                    free(buffer->vert);
                    buffer->vert = newPtr;
                    buffer->cur_vrtMaxIndex = indexNum;
                }
            }
        }
        break;
    }
}
#endif


void draw_adas(int adasCount, float *adas, float LR, int flag)
{
    glUseProgram(programHandleReverseLine[0]);
    glEnable(GL_DEPTH_TEST);
    int ix =0;
    int arrindex = 0;
    if(flag == 1)
    {
        float verticesRgbaTmp[12] = {
                    0.99, 0.99, 0.0,
                    -0.99, 0.99, 0.0,
                    -0.99, -0.99, 0.0,
                    0.99, -0.99, 0.0};
        memcpy(lineADAS, verticesRgbaTmp, sizeof(verticesRgbaTmp));

        arrindex = 1;
        //printf("adasCount1 = %d\n", adasCount);
        for(ix=0;ix<(adasCount);ix++)
        {
            int index = ix*5;
            float minx = adas[index + 0];
            float miny = adas[index + 1];
            float maxx = adas[index + 2];
            float maxy = adas[index + 3];
            float Z = 0.0;
            //printf("adas%f %f %f %f\n", minx, miny, maxx, maxy);
            if(adas[index + 4] == LR)
            {
                float verticesRgbaTmpblc[12] = {
                    minx, miny, Z,
                    minx, maxy, Z,
                    maxx, maxy, Z,
                    maxx, miny, Z};
                int iy;
                // for(iy=0;iy<12;iy+=3)
                // {
                //     AVM_LOGI("%f %f %f\n", verticesRgbaTmp[iy+0], verticesRgbaTmp[iy+1], verticesRgbaTmp[iy+2]);
                // }
                memcpy(lineADAS + 12 * arrindex, verticesRgbaTmpblc, sizeof(verticesRgbaTmpblc));
                arrindex++;
            }
            
        }
    }
    else
    {
        //printf("adasCount0 = %d\n", adasCount);
        for(ix=0;ix<(adasCount);ix++)
        {
            int index = ix*5;
            float minx = adas[index + 0];
            float miny = adas[index + 1];
            float maxx = adas[index + 2];
            float maxy = adas[index + 3];
            float Z = 0.0;
            //printf("adas%f %f %f %f\n", minx, miny, maxx, maxy);
            if(adas[index + 4] == LR)
            {
                float verticesRgbaTmpblc[12] = {
                    minx, miny, Z,
                    minx, maxy, Z,
                    maxx, maxy, Z,
                    maxx, miny, Z};
                int iy;
                // for(iy=0;iy<12;iy+=3)
                // {
                //     AVM_LOGI("%f %f %f\n", verticesRgbaTmp[iy+0], verticesRgbaTmp[iy+1], verticesRgbaTmp[iy+2]);
                // }
                memcpy(lineADAS + 12 * arrindex, verticesRgbaTmpblc, sizeof(verticesRgbaTmpblc));
                arrindex++;
            }
            
        }
    }
    
    //printf("arrindex = %d\n", arrindex);

    glBindVertexArray(VAOLine[1]);

    glUniform1i(glGetUniformLocation(programHandleReverseLine[0], "switchPosition"), 0);
    glUniform1i(glGetUniformLocation(programHandleReverseLine[0], "windows"), 1);
    glUniform1f(glGetUniformLocation(programHandleReverseLine[0], "yRervsed"), yReverse);


    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_adas[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineADAS), lineADAS);
    if(flag == 1)
        set_color(GL_COLOR_RED);
    else if(flag == 0)
        set_color(GL_COLOR_YELLOW);

    glLineWidth(5.0);

    for(ix=0;ix<arrindex;ix++)
    {
        glDrawArrays(GL_LINE_LOOP, 4 * ix, 4);
    }
    // AVM_LOGI("draw_adas end\n");

    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}







/**
 * @brief : dynamic line used function below
 * @note : just for ELEAD case
*/

FishCamCalibartionPara ReadCalibrationData(const char* path) {
    FishCamCalibartionPara calibration = {0};
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        AVM_LOGI("Error opening fisheye file\n");
        g_2ddynamicLine_Turn = 0;
        return calibration;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "K matrix")) {
            if (fgets(line, sizeof(line), file)) {
                char *token = strtok(line, " ");
                calibration.fx = atof(token);
                token = strtok(NULL, " ");
                token = strtok(NULL, " ");
                calibration.cx = atof(token);
                
                if (fgets(line, sizeof(line), file)) {
                    token = strtok(line, " ");
                    token = strtok(NULL, " ");
                    calibration.fy = atof(token);
                    token = strtok(NULL, " ");
                    calibration.cy = atof(token);
                }
            }
        } else if (strstr(line, "distortionCoef")) {
            if (fgets(line, sizeof(line), file)) calibration.k1 = atof(line);
            if (fgets(line, sizeof(line), file)) calibration.k2 = atof(line);
            if (fgets(line, sizeof(line), file)) calibration.k3 = atof(line);
            if (fgets(line, sizeof(line), file)) calibration.k4 = atof(line);
        } else if (strstr(line, "image Height")) {
            if (fgets(line, sizeof(line), file)) calibration.imgHeight = atoi(line);
        } else if (strstr(line, "image Width")) {
            if (fgets(line, sizeof(line), file)) calibration.imgWidth = atoi(line);
        }
    }
    fclose(file);



    return calibration;
}

void read_calib_fish_para()
{
    AVM_LOGI("------------------------------------------\n");
    AVM_LOGI("O. read fisheye calibration information\n");
    char front_calib_path[1024];char rear_calib_path[1024];
    char left_calib_path[1024];char right_calib_path[1024];
    sprintf(front_calib_path,"%sEVSTable/input_img/front.txt",EVSTable_path[0]);
    sprintf(rear_calib_path,"%sEVSTable/input_img/back.txt",EVSTable_path[0]);
    sprintf(left_calib_path,"%sEVSTable/input_img/left.txt",EVSTable_path[0]);
    sprintf(right_calib_path,"%sEVSTable/input_img/right.txt",EVSTable_path[0]);

    AVM_LOGI("  read front.txt in [%s]\n", front_calib_path);
    Display_pgl.fisheye.parameter[0] = ReadCalibrationData(front_calib_path);
    AVM_LOGI("  read rear.txt in [%s]\n", rear_calib_path);
    Display_pgl.fisheye.parameter[1] = ReadCalibrationData(rear_calib_path);
    AVM_LOGI("  read rear.txt in [%s]\n", left_calib_path);
    Display_pgl.fisheye.parameter[2] = ReadCalibrationData(left_calib_path);
    AVM_LOGI("  read rear.txt in [%s]\n", right_calib_path);
    Display_pgl.fisheye.parameter[3] = ReadCalibrationData(right_calib_path);

#if 1 //debug
    AVM_LOGI("fx fy cx cy [%f] [%f] [%f] [%f] \n",Display_pgl.fisheye.parameter[0].fx, Display_pgl.fisheye.parameter[0].fy,Display_pgl.fisheye.parameter[0].cx,Display_pgl.fisheye.parameter[0].cy);
    AVM_LOGI("d1 d2 d3 d4 [%f] [%f] [%f] [%f] \n",Display_pgl.fisheye.parameter[0].k1, Display_pgl.fisheye.parameter[0].k2,Display_pgl.fisheye.parameter[0].k3,Display_pgl.fisheye.parameter[0].k4);
    AVM_LOGI("h w [%d] [%d] \n",Display_pgl.fisheye.parameter[0].imgHeight, Display_pgl.fisheye.parameter[0].imgWidth);
#endif

    AVM_LOGI(".......OK\n");
}

void read_static_scene_line_user_data()
{
    g_static_fisheye_parameter_readOK = 0;
    char STATIC_front_path[256];
    char STATIC_back_path[256];
    char STATIC_left_path[256];
    char STATIC_right_path[256];
    sprintf(STATIC_front_path, "EVSTable/pgl/Dynamic/PARA%d/fisheye_static_scene_front_rawpixel.bin",  4);
    sprintf(STATIC_back_path,  "EVSTable/pgl/Dynamic/PARA%d/fisheye_static_scene_back_rawpixel.bin",   4);
    sprintf(STATIC_left_path,  "EVSTable/pgl/Dynamic/PARA%d/fisheye_static_scene_left_rawpixel.bin",   4);
    sprintf(STATIC_right_path,  "EVSTable/pgl/Dynamic/PARA%d/fisheye_static_scene_right_rawpixel.bin",   4);

    char* file_path[4] = {STATIC_front_path, STATIC_back_path, STATIC_left_path, STATIC_right_path};

    for (int camM = 0; camM < 4; camM++)
    {
        FILE *file = fp_source_app(file_path[camM], "rb");

        if (file == NULL)
        {
            AVM_LOGI("\033[31m[AutoSys] fail.......\033[0m\n");
            AVM_LOGI("[AutoSys] Read scene_line fail in [%s].......\n", file_path[camM]);
            AVM_LOGI("[AutoSys] scene_line parameter no reading, So cencel the program\n");
            if(camM == 2 || camM == 3)
            {
                g_staticSceneLine_Turn = 0;
            }
            return;
        }
        //AVM_LOGI("Current Read file in [%s].......\n", Dydata_front_path);

        DYNAMIC_PACKAGE *pack = &staticData[1][camM];
        AVM_LOGI("Current CAM is [%d]\n", camM);
        // Read lineNumber array
        int lineNumberSize;
        fp_read_data_safty(&lineNumberSize, sizeof(int), 1, file);
        AVM_LOGI("Current lineNumberSize is [%d]\n", lineNumberSize);

        pack->lineNumber = (int *)malloc(lineNumberSize * sizeof(int));
        fp_read_data_safty(pack->lineNumber, sizeof(int), lineNumberSize, file);

        // Reading line_strip 2D array
        int line_stripOuterSize;
        fp_read_data_safty(&line_stripOuterSize, sizeof(int), 1, file);
        pack->line_strip = (int **)malloc(line_stripOuterSize * sizeof(int *));
        for (int i = 0; i < line_stripOuterSize; i++)
        {
            int line_stripInnerSize;
            fp_read_data_safty(&line_stripInnerSize, sizeof(int), 1, file);
            pack->line_strip[i] = (int *)malloc(line_stripInnerSize * sizeof(int));
            fp_read_data_safty(pack->line_strip[i], sizeof(int), line_stripInnerSize, file);
        }

        // Reading gl_colormap 2D array
        int gl_colormapOuterSize;
        fp_read_data_safty(&gl_colormapOuterSize, sizeof(int), 1, file);
        pack->gl_colormap = (float **)malloc(gl_colormapOuterSize * sizeof(float *));
        for (int i = 0; i < gl_colormapOuterSize; i++)
        {
            int gl_colormapInnerSize;
            fp_read_data_safty(&gl_colormapInnerSize, sizeof(int), 1, file);
            pack->gl_colormap[i] = (float *)malloc(gl_colormapInnerSize * sizeof(float));
            fp_read_data_safty(pack->gl_colormap[i], sizeof(float), gl_colormapInnerSize, file);
        }

        // Reading linewidth 2D array
        int linewidthOuterSize;
        fp_read_data_safty(&linewidthOuterSize, sizeof(int), 1, file);
        pack->linewidth = (float **)malloc(linewidthOuterSize * sizeof(float *));
        for (int i = 0; i < linewidthOuterSize; i++)
        {
            int linewidthInnerSize;
            fp_read_data_safty(&linewidthInnerSize, sizeof(int), 1, file);
            pack->linewidth[i] = (float *)malloc(linewidthInnerSize * sizeof(float));
            fp_read_data_safty(pack->linewidth[i], sizeof(float), linewidthInnerSize, file);
        }

        // Reading worldcoordinate 2D array
        int worldcoordinateOuterSize;
        fp_read_data_safty(&worldcoordinateOuterSize, sizeof(int), 1, file);
        pack->worldcoordinate = (int **)malloc(worldcoordinateOuterSize * sizeof(int *));
        pack->pixelcoordinate = (int **)malloc(worldcoordinateOuterSize * sizeof(int *));
        for (int i = 0; i < worldcoordinateOuterSize; i++)
        {
            int worldcoordinateInnerSize;
            fp_read_data_safty(&worldcoordinateInnerSize, sizeof(int), 1, file);
            pack->worldcoordinate[i] = (int *)malloc(worldcoordinateInnerSize * sizeof(int));
            pack->pixelcoordinate[i] = (int *)malloc(worldcoordinateInnerSize * sizeof(int));
            fp_read_data_safty(pack->worldcoordinate[i], sizeof(int), worldcoordinateInnerSize, file);
            memcpy(pack->pixelcoordinate[i],pack->worldcoordinate[i], sizeof(int) * worldcoordinateInnerSize);
            if(i == 0 || i == 1)
            {
                AVM_LOGI("style : %d, scene p1: %d %d\n", i,  
                pack->worldcoordinate[i][0 * 4 + 0], 
                pack->worldcoordinate[i][0 * 4 + 1]);
                AVM_LOGI("style : %d, scene p2: %d %d\n", i,
                pack->worldcoordinate[i][0 * 4 + 2], 
                pack->worldcoordinate[i][0 * 4 + 3]);
            }
        }
        fp_close_data_safty(file);
    }
    AVM_LOGI("[AutoSys] static_scene parameter reading OK\n");
    g_static_fisheye_parameter_readOK = 1;
}

void read_static_line_user_data()
{
    char STATIC_front_path[1024];
    char STATIC_back_path[1024];
    char STATIC_left_path[1024];
    char STATIC_right_path[1024];
    sprintf(STATIC_front_path, "EVSTable/pgl/Dynamic/PARA%d/fisheye_static_undistpixel_front.bin",  3);
    sprintf(STATIC_back_path,  "EVSTable/pgl/Dynamic/PARA%d/fisheye_static_undistpixel_rear.bin",   3);
    sprintf(STATIC_left_path,  "EVSTable/pgl/Dynamic/PARA%d/fisheye_static_undistpixel_left.bin",   3);
    sprintf(STATIC_right_path,  "EVSTable/pgl/Dynamic/PARA%d/fisheye_static_undistpixel_right.bin",   3);
    char* file_path[4] = {STATIC_front_path, STATIC_back_path, STATIC_left_path, STATIC_right_path};
    for (int camM = 0; camM < 4; camM++)
    {
        FILE *file = fp_source_app(file_path[camM], "rb");

        if (file == NULL)
        {
            AVM_LOGI("\033[31m[AutoSys] fail.......\033[0m\n");
            AVM_LOGI("[AutoSys] Read dyinformation fail in [%s].......\n", STATIC_front_path);
            AVM_LOGI("[AutoSys] Static line parameter no reading, So using dynamicparameter\n");
            g_staticLine_Turn = 0;
            return;
        }
        //AVM_LOGI("Current Read file in [%s].......\n", Dydata_front_path);

        DYNAMIC_PACKAGE *pack = &staticData[0][camM];
        AVM_LOGI("Current CAM is [%d]\n", camM);
        // Read lineNumber array
        int lineNumberSize;
        fp_read_data_safty(&lineNumberSize, sizeof(int), 1, file);
        AVM_LOGI("Current lineNumberSize is [%d]\n", lineNumberSize);

        pack->lineNumber = (int *)malloc(lineNumberSize * sizeof(int));
        fp_read_data_safty(pack->lineNumber, sizeof(int), lineNumberSize, file);

        // Reading line_strip 2D array
        int line_stripOuterSize;
        fp_read_data_safty(&line_stripOuterSize, sizeof(int), 1, file);
        pack->line_strip = (int **)malloc(line_stripOuterSize * sizeof(int *));
        for (int i = 0; i < line_stripOuterSize; i++)
        {
            int line_stripInnerSize;
            fp_read_data_safty(&line_stripInnerSize, sizeof(int), 1, file);
            pack->line_strip[i] = (int *)malloc(line_stripInnerSize * sizeof(int));
            fp_read_data_safty(pack->line_strip[i], sizeof(int), line_stripInnerSize, file);
        }

        // Reading gl_colormap 2D array
        int gl_colormapOuterSize;
        fp_read_data_safty(&gl_colormapOuterSize, sizeof(int), 1, file);
        pack->gl_colormap = (float **)malloc(gl_colormapOuterSize * sizeof(float *));
        for (int i = 0; i < gl_colormapOuterSize; i++)
        {
            int gl_colormapInnerSize;
            fp_read_data_safty(&gl_colormapInnerSize, sizeof(int), 1, file);
            pack->gl_colormap[i] = (float *)malloc(gl_colormapInnerSize * sizeof(float));
            fp_read_data_safty(pack->gl_colormap[i], sizeof(float), gl_colormapInnerSize, file);
        }

        // Reading linewidth 2D array
        int linewidthOuterSize;
        fp_read_data_safty(&linewidthOuterSize, sizeof(int), 1, file);
        pack->linewidth = (float **)malloc(linewidthOuterSize * sizeof(float *));
        for (int i = 0; i < linewidthOuterSize; i++)
        {
            int linewidthInnerSize;
            fp_read_data_safty(&linewidthInnerSize, sizeof(int), 1, file);
            pack->linewidth[i] = (float *)malloc(linewidthInnerSize * sizeof(float));
            fp_read_data_safty(pack->linewidth[i], sizeof(float), linewidthInnerSize, file);
        }

        // Reading worldcoordinate 2D array
        int worldcoordinateOuterSize;
        fp_read_data_safty(&worldcoordinateOuterSize, sizeof(int), 1, file);
        pack->worldcoordinate = (int **)malloc(worldcoordinateOuterSize * sizeof(int *));
        pack->pixelcoordinate = (int **)malloc(worldcoordinateOuterSize * sizeof(int *));
        for (int i = 0; i < worldcoordinateOuterSize; i++)
        {
            int worldcoordinateInnerSize;
            fp_read_data_safty(&worldcoordinateInnerSize, sizeof(int), 1, file);
            pack->worldcoordinate[i] = (int *)malloc(worldcoordinateInnerSize * sizeof(int));
            pack->pixelcoordinate[i] = (int *)malloc(worldcoordinateInnerSize * sizeof(int));
            fp_read_data_safty(pack->worldcoordinate[i], sizeof(int), worldcoordinateInnerSize, file);
            memcpy(pack->pixelcoordinate[i],pack->worldcoordinate[i], sizeof(int) * worldcoordinateInnerSize);
            if(i == 0 || i == 1)
            {
                AVM_LOGI("style : %d, scene p1: %d %d\n", i,  
                pack->worldcoordinate[i][0 * 4 + 0], 
                pack->worldcoordinate[i][0 * 4 + 1]);
                AVM_LOGI("style : %d, scene p2: %d %d\n", i,
                pack->worldcoordinate[i][0 * 4 + 2], 
                pack->worldcoordinate[i][0 * 4 + 3]);
            }
        }
        fp_close_data_safty(file);
    }
    AVM_LOGI("[AutoSys] Static line parameter reading OK\n");
    g_static_fisheye_parameter_readOK = 1;
}

void read_pgl_colorbar()
{
    program_context* context = &glContext_ReverseLine;
    for(int ix = 0; ix < PARAM_PGL_MAX_STYLE_NUM; ix++)
    {
        int w1,h1,n;
        unsigned char *data = NULL;
        char dir[1024] = "\0";
        sprintf(dir, "EVSTable/pgl/Dynamic/ColorBar/ColorBar_style%d.png",ix);
        fp_read_image_app(dir, &data, &w1, &h1, &n, 4, 1 );
        if(data != NULL)
        {
            glGenTextures(1, &context->textureID[ix]);
	        glBindTexture(GL_TEXTURE_2D, context->textureID[ix]);
	        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w1, h1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	        stbi_image_free(data);
        }
    }
}

void read_dynamic_line_user_data()
{
    //FILE *fgoldensample = fopen(carInforpath, "rb");
    for (int para = 0; para < LINE_PARA_NUM_PACK; para++)
    {
        char carInforpath[1024], Dydata_front_path[1024], Dydata_back_path[1024], Dydata_left_path[1024], Dydata_right_path[1024];
        
        sprintf(carInforpath,      "EVSTable/pgl/Dynamic/PARA%d/carinformation.bin", para + 1);
        sprintf(Dydata_front_path, "EVSTable/pgl/Dynamic/PARA%d/front.bin",          para + 1);
        sprintf(Dydata_back_path,  "EVSTable/pgl/Dynamic/PARA%d/rear.bin",           para + 1);
        sprintf(Dydata_left_path,  "EVSTable/pgl/Dynamic/PARA%d/left.bin",           para + 1);
        sprintf(Dydata_right_path, "EVSTable/pgl/Dynamic/PARA%d/right.bin",          para + 1);
        char* file_path[4] = {Dydata_front_path, Dydata_back_path, Dydata_left_path, Dydata_right_path};
        //sprintf(STATIC_path, "%sEVSTable/pgl/Dynamic/PARA%d/fisheye_static.bin", EVSTable_path[0], para + 1);//fisheye_static.bin
        AVM_LOGI("------------------------------------------\n");
        AVM_LOGI("O. read car information\n");
        FILE *carinfor = fp_source_app(carInforpath, "rb");
        if (carinfor == NULL)
        {
            AVM_LOGI("No car information bin [%s], so use default value\n", carInforpath);
            dynamicData[para][0].car_WHEELTOREAR = 100.25f;
            dynamicData[para][0].car_WHEELBASE = 426.25f;
            dynamicData[para][0].car_WHEELTOFRONT = 100.25f;
        }
        else
        {
            fp_read_data_safty(&dynamicData[para][0].car_WHEELTOREAR, sizeof(float), 1, carinfor);
            fp_read_data_safty(&dynamicData[para][0].car_WHEELBASE, sizeof(float), 1, carinfor);
            fp_read_data_safty(&dynamicData[para][0].car_WHEELTOFRONT, sizeof(float), 1, carinfor);
            dynamicData[para][1].car_WHEELBASE    = dynamicData[para][0].car_WHEELBASE;
            dynamicData[para][1].car_WHEELTOREAR  = dynamicData[para][0].car_WHEELTOREAR;
            dynamicData[para][1].car_WHEELTOFRONT = dynamicData[para][0].car_WHEELTOFRONT;
            fp_close_data_safty(carinfor);
        }
        AVM_LOGI("read car information OK\n");

        AVM_LOGI("I. read dyinformation .......");
        for (int camM = 0; camM < MAX_CAM_NUM; camM++)
        {
            FILE *file;
            file = fp_source_app(file_path[camM],"rb");

            if (file == NULL)
            {
                AVM_LOGI("fail.......\n");
                AVM_LOGI("[ERROR] non read the dyinformation data\n");
                AVM_LOGE("Read dyinformation pgl fail\n");
                AVM_LOGI("Read dyinformation fail in [%s].......\n", Dydata_front_path);
                g_dynamicLine_Turn = 0;
                return;
            }
            AVM_LOGI("Current Read file in [%s].......\n", Dydata_front_path);

            DYNAMIC_PACKAGE *pack = &dynamicData[para][camM];
            AVM_LOGI("Current CAM is [%d]\n", camM);
            // Read lineNumber array
            int lineNumberSize;
            fp_read_data_safty(&lineNumberSize, sizeof(int), 1, file);
            AVM_LOGI("Current lineNumberSize is [%d]\n", lineNumberSize);

            pack->lineNumber = (int *)malloc(lineNumberSize * sizeof(int));
            fp_read_data_safty(pack->lineNumber, sizeof(int), lineNumberSize, file);

            // Reading line_strip 2D array
            int line_stripOuterSize;
            fp_read_data_safty(&line_stripOuterSize, sizeof(int), 1, file);
            pack->line_strip = (int **)malloc(line_stripOuterSize * sizeof(int *));
            for (int i = 0; i < line_stripOuterSize; i++)
            {
                int line_stripInnerSize;
                fp_read_data_safty(&line_stripInnerSize, sizeof(int), 1, file);
                pack->line_strip[i] = (int *)malloc(line_stripInnerSize * sizeof(int));
                fp_read_data_safty(pack->line_strip[i], sizeof(int), line_stripInnerSize, file);
            }

            // Reading gl_colormap 2D array
            int gl_colormapOuterSize;
            fp_read_data_safty(&gl_colormapOuterSize, sizeof(int), 1, file);
            pack->gl_colormap = (float **)malloc(gl_colormapOuterSize * sizeof(float *));
            for (int i = 0; i < gl_colormapOuterSize; i++)
            {
                int gl_colormapInnerSize;
                fp_read_data_safty(&gl_colormapInnerSize, sizeof(int), 1, file);
                pack->gl_colormap[i] = (float *)malloc(gl_colormapInnerSize * sizeof(float));
                fp_read_data_safty(pack->gl_colormap[i], sizeof(float), gl_colormapInnerSize, file);
            }

            // Reading linewidth 2D array
            int linewidthOuterSize;
            fp_read_data_safty(&linewidthOuterSize, sizeof(int), 1, file);
            pack->linewidth = (float **)malloc(linewidthOuterSize * sizeof(float *));
            for (int i = 0; i < linewidthOuterSize; i++)
            {
                int linewidthInnerSize;
                fp_read_data_safty(&linewidthInnerSize, sizeof(int), 1, file);
                pack->linewidth[i] = (float *)malloc(linewidthInnerSize * sizeof(float));
                fp_read_data_safty(pack->linewidth[i], sizeof(float), linewidthInnerSize, file);
            }

            // Reading worldcoordinate 2D array
            int worldcoordinateOuterSize;
            fp_read_data_safty(&worldcoordinateOuterSize, sizeof(int), 1, file);
            pack->worldcoordinate = (int **)malloc(worldcoordinateOuterSize * sizeof(int *));
            for (int i = 0; i < worldcoordinateOuterSize; i++)
            {
                int worldcoordinateInnerSize;
                fp_read_data_safty(&worldcoordinateInnerSize, sizeof(int), 1, file);
                pack->worldcoordinate[i] = (int *)malloc(worldcoordinateInnerSize * sizeof(int));
                fp_read_data_safty(pack->worldcoordinate[i], sizeof(int), worldcoordinateInnerSize, file);
            }

            // DEBUG
            // if(para == 1 && camM == 1)
            // {
            //     AVM_LOGI("lineNumberSize is [%d]\n", lineNumberSize);
            //     AVM_LOGI("line_stripOuterSize is [%d]\n", line_stripOuterSize);
            //     AVM_LOGI("gl_colormapOuterSize is [%d]\n", gl_colormapOuterSize);
            //     AVM_LOGI("linewidthOuterSize is [%d]\n", linewidthOuterSize);
            //     AVM_LOGI("worldcoordinateOuterSize is [%d]\n", worldcoordinateOuterSize);
            //     int colorindex = 0;
            //     int wpindex = 0;
            //     for(int count = 0; count < 8; count++)
            //     {
            //         AVM_LOGI("lineColor id[%d] rgba[%f, %f, %f, %f]\n",count,
            //         pack->gl_colormap[1][colorindex++],
            //         pack->gl_colormap[1][colorindex++],
            //         pack->gl_colormap[1][colorindex++],
            //         pack->gl_colormap[1][colorindex++]);
            //         AVM_LOGI("worldcoordinate id[%d] wp1[%d, %d] wp2[%d, %d]\n",count,
            //         pack->worldcoordinate[1][wpindex++],
            //         pack->worldcoordinate[1][wpindex++],
            //         pack->worldcoordinate[1][wpindex++],
            //         pack->worldcoordinate[1][wpindex++]);
            //     }
            // }

            // Reading other fields of DYNAMIC_PACKAGE
            fp_read_data_safty(&(pack->director), sizeof(int), 1, file);
            fp_read_data_safty(&(pack->current_style), sizeof(int), 1, file);
            fp_read_data_safty(&(pack->current_line), sizeof(int), 1, file);
            fp_read_data_safty(&(pack->current_cam), sizeof(int), 1, file);

            // Reading DYNAMIC_TABLE
            int tableLength;
            fp_read_data_safty(&tableLength, sizeof(int), 1, file);
            pack->table = (DYNAMIC_TABLE *)malloc(tableLength * sizeof(DYNAMIC_TABLE));
            for (int i = 0; i < tableLength; i++)
            {
                int idLength;
                fp_read_data_safty(&idLength, sizeof(int), 1, file);
                pack->table[i].ID = (int *)malloc(idLength * sizeof(int));
                fp_read_data_safty(pack->table[i].ID, sizeof(int), idLength, file);

                // Reading junctionP of DYNAMIC_TABLE
                int junctionPLength;
                fp_read_data_safty(&junctionPLength, sizeof(int), 1, file);
                pack->table[i].junctionP = (int **)malloc(junctionPLength * sizeof(int *));
                for (int j = 0; j < junctionPLength; j++)
                {
                    pack->table[i].junctionP[j] = (int *)malloc(2 * sizeof(int));
                    fp_read_data_safty(pack->table[i].junctionP[j], sizeof(int), 2, file);
                }
            }

            // Reading JunctionItem reference
            int referenceLength;
            fp_read_data_safty(&referenceLength, sizeof(int), 1, file);
            pack->reference = (JunctionItem *)malloc(referenceLength * sizeof(JunctionItem));
            pack->map_size = (int *)malloc(referenceLength * sizeof(int));
            for (int i = 0; i < referenceLength; i++)
            {
                fp_read_data_safty(&(pack->reference[i].cur_id), sizeof(int), 1, file);

                // Reading junP1 and junP2 which are PointF lists
                int junP1Length, junP2Length;
                fp_read_data_safty(&junP1Length, sizeof(int), 1, file);
                pack->reference[i].junP1 = (PointF *)malloc(junP1Length * sizeof(PointF));
                fp_read_data_safty(pack->reference[i].junP1, sizeof(PointF), junP1Length, file);

                fp_read_data_safty(&junP2Length, sizeof(int), 1, file);
                pack->reference[i].junP2 = (PointF *)malloc(junP2Length * sizeof(PointF));
                fp_read_data_safty(pack->reference[i].junP2, sizeof(PointF), junP2Length, file);

                // Reading ver_width which is an int list
                int verWidthLength;
                fp_read_data_safty(&verWidthLength, sizeof(int), 1, file);
                pack->reference[i].ver_width = (int *)malloc(verWidthLength * sizeof(int));
                fp_read_data_safty(pack->reference[i].ver_width, sizeof(int), verWidthLength, file);

                // Reading map which is a DYMAP list
                int mapLength;
                fp_read_data_safty(&mapLength, sizeof(int), 1, file);
                pack->map_size[i] = mapLength;
                pack->reference[i].map = (DYMAP *)malloc(mapLength * sizeof(DYMAP));
                fp_read_data_safty(pack->reference[i].map, sizeof(DYMAP), mapLength, file);
            }

            fp_close_data_safty(file);
        }
    }

    AVM_LOGI(".......OK\n");
#if 0 //if it need to debug
    AVM_LOGI("------------------Debug----------------------\n");
    AVM_LOGI("front dynamic data\n");

    AVM_LOGI("***table ID[0]***\n");
    
    for (int i = 0; i < 10; i++)
    {
        AVM_LOGI("%d, ", dynamicData[0].table[0].ID[i]);
    }
    AVM_LOGI("\n");
    AVM_LOGI("***table ID[1]***\n");
    for (int i = 0; i < 10; i++)
    {
        AVM_LOGI("%d, ", dynamicData[0].table[1].ID[i]);
    }
    AVM_LOGI("\n");

    AVM_LOGI("***JunctionP[0]***\n");
    for (int i = 0; i < 10; i++)
    {
        AVM_LOGI("%d, ", dynamicData[0].table[0].junctionP[i][0]);
        AVM_LOGI("%d, **", dynamicData[0].table[0].junctionP[i][1]);
    }
    AVM_LOGI("\n");
    AVM_LOGI("***JunctionP[1]***\n");
    for (int i = 0; i < 10; i++)
    {
        AVM_LOGI("%d, ", dynamicData[0].table[1].junctionP[i][0]);
        AVM_LOGI("%d, **", dynamicData[0].table[1].junctionP[i][1]);
    }
    AVM_LOGI("\n");

    AVM_LOGI("***reference[0] map***\n");
    for (int i = 0; i < dynamicData[0].map_size[0]; i++)
    {
        AVM_LOGI("x = %d, ", dynamicData[0].reference[0].map[i].x);
        AVM_LOGI("y = %d, ", dynamicData[0].reference[0].map[i].y);
    }
    AVM_LOGI("\n");
    AVM_LOGI("***reference[1] map***\n");
    for (int i = 0; i < dynamicData[0].map_size[1]; i++)
    {
        AVM_LOGI("x = %d, ", dynamicData[0].reference[1].map[i].x);
        AVM_LOGI("y = %d, ", dynamicData[0].reference[1].map[i].y);
    }
    AVM_LOGI("\n");
   AVM_LOGI("back dynamic data\n");


   AVM_LOGI("***table ID[0]***\n");

   for (int i = 0; i < 10; i++)
   {
       AVM_LOGI("%d, ", dynamicData[1].table[0].ID[i]);
   }
   AVM_LOGI("\n");
   AVM_LOGI("***table ID[1]***\n");
   for (int i = 0; i < 10; i++)
   {
       AVM_LOGI("%d, ", dynamicData[1].table[1].ID[i]);
   }
   AVM_LOGI("\n");

   AVM_LOGI("***JunctionP[0]***\n");
   for (int i = 0; i < 10; i++)
   {
       AVM_LOGI("%d, ", dynamicData[1].table[0].junctionP[i][0]);
       AVM_LOGI("%d, **", dynamicData[1].table[0].junctionP[i][1]);
   }
   AVM_LOGI("\n");
   AVM_LOGI("***JunctionP[1]***\n");
   for (int i = 0; i < 10; i++)
   {
       AVM_LOGI("%d, ", dynamicData[1].table[1].junctionP[i][0]);
       AVM_LOGI("%d, **", dynamicData[1].table[1].junctionP[i][1]);
   }
   AVM_LOGI("\n");
   AVM_LOGI("***reference[0] map***\n");
   for (int i = 0; i < dynamicData[1].map_size[0]; i++)
   {
       AVM_LOGI("x = %d, ", dynamicData[1].reference[0].map[i].x);
       AVM_LOGI("y = %d, ", dynamicData[1].reference[0].map[i].y);
   }
   AVM_LOGI("\n");
   AVM_LOGI("***reference[1] map***\n");
   for (int i = 0; i < dynamicData[0].map_size[1]; i++)
   {
       AVM_LOGI("x = %d, ", dynamicData[1].reference[1].map[i].x);
       AVM_LOGI("y = %d, ", dynamicData[1].reference[1].map[i].y);
   }
   AVM_LOGI("\n");
#endif


    AVM_LOGI("------------------------------------------\n");

}
int resort_dynamic_buffer(DYNAMIC_PACKAGE *active, int *linetype, int cam, int style)
{
    if ((active == NULL) || (linetype == NULL))
    {
        return 0;
    }

    int para = ((Display_pgl.view.type == AVM_3D_VIEW) || (Display_pgl.view.type == AVM_2D_VIEW)) ? 1 : 0;

    if (active->lineNumber == NULL)
    {
        return 0;
    }

    for (int ix = 0; ix < active->lineNumber[style]; ix++)
    {
        int index = ix * 4;
        int* world = active->worldcoordinate[style];

        if (world == NULL)
        {
            return 0;
        }

        linetype[ix] = ((world[index + 1] == world[index + 3])) ? horizontal : vertical;

        if ((Display_pgl.view.type == FISHEYE_STATIC_DISTORTION) || (Display_pgl.view.type == FISHEYE_STATIC_SCENE))
        {
            linetype[ix] = slop;
        }
    }

    return 1;
}

DynamicPara cal_base_dynamic_line_parameter(int cam, double car_angle, double* w_y, double* w_x, int i, int narrow, int holdmaxlinelength)
{
    //int slect_para = Display_pgl.view.type;
    int slect_para = (Display_pgl.view.type == AVM_3D_VIEW || Display_pgl.view.type == AVM_2D_VIEW)?1:0;

    double car_WHEELBASE = (double)dynamicData[slect_para][0].car_WHEELBASE;
    float car_LONG_HALF = (dynamicData[slect_para][0].car_WHEELBASE / 2.0f);
    double CAR_WHEELTOHEAD = (cam == 0)?(double)dynamicData[slect_para][0].car_WHEELTOFRONT: (double)dynamicData[slect_para][0].car_WHEELTOREAR;
    //double car_WHEELTOREAR = (double)dynamicData[slect_para][0].car_WHEELTOREAR;
    double car_WIDTH, car_LINE_START, car_PATH_MAX_2D;

    //car_WHEELTOREAR = w_y[i * 2];

    car_PATH_MAX_2D = (holdmaxlinelength > 0) ? holdmaxlinelength : w_y[(2 * i) + 1] - w_y[i * 2];
    car_LINE_START =  w_y[i * 2] - (CAR_WHEELTOHEAD);
    if (cam == CAM_NUM_FRONT)
    {
        CAR_WHEELTOHEAD +=   2.0*car_LONG_HALF;
        CAR_WHEELTOHEAD *= -1.0;
        car_LONG_HALF   *= -1.0f;
        car_PATH_MAX_2D *= -1.0; 
        car_LINE_START  *= -1.0;
    }


    /**
     **@input
     ** 0. CAR_WHEELBASE: wl                 // 車輛軸距
     ** 1. CAR_WHEELTOREAR: w_y[i * 2]       // 車輪到車尾距離 (line_start_y)
     ** 2. CAR_WHEELTOREAR: iy               // 同上，可統一為 iy
     ** 3. car_width: w_x[i * 2 + narrow] * 2 // 車寬 (line_start_x)
     ** 4. car_width: ix                      // 同上，可統一為 ix
     *
     ** @output
     ** 1. R = wl / tan(angle)                // 轉向半徑
     ** 2. Wheel_R = sqrt((ix / 2)^2 + iy^2)  // 車輪中心至車輛參考點的距離
     ** 3. Wheel_angle = arctan(iy / (ix / 2))// 車輪偏轉角度
     */

    double angleStart, xCenter, yCenter;
    double radiusCenter = (car_WHEELBASE) / tan(DEG_TO_RAD(fabs(car_angle))); //車中心半徑

    car_WIDTH = abs(w_x[i * 2 + narrow] * 2);

    double wheelradius = sqrt((CAR_WHEELTOHEAD * CAR_WHEELTOHEAD) + ((car_WIDTH / 2.0) * (car_WIDTH / 2.0))); //外圓輪半徑
    //if(cam == 0) wheelradius = sqrt((car_WHEELTOREAR * car_WHEELTOREAR) + ((car_WIDTH / 2.0) * (car_WIDTH / 2.0))) - 3.0; //外圓輪半徑
    double wheelAngle = RAD_TO_DEG(atan(fabs(CAR_WHEELTOHEAD) / fabs(car_WIDTH / 2.0))); //內圓輪半徑


    float lineStart = (float)(RAD_TO_DEG(car_LINE_START) / radiusCenter);
    double maxDrawAngle = RAD_TO_DEG(car_PATH_MAX_2D) / radiusCenter;

    int totalPoint = (int)ceil(maxDrawAngle) + 1;
    totalPoint = (totalPoint < 50)? 100:totalPoint;
    double angleSpace = maxDrawAngle / (totalPoint - 1);

    if (car_angle < 0)
    {
        angleStart = 0.0 + lineStart;
        xCenter = (radiusCenter) * -1;
        yCenter = 0.0;
    }
    else
    {
        angleStart = 180.0 - maxDrawAngle - lineStart;
        xCenter = radiusCenter;
        yCenter = 0.0;
    }
    
    DynamicPara para;
    para.totalPoint = totalPoint;
    para.angleSpace = angleSpace;
    para.angleStart = angleStart;
    para.xCenter = xCenter;
    para.yCenter = yCenter;
    para.radiusCenter = radiusCenter;
    para.car_LONG_HALF = car_LONG_HALF;
    para.wheelAngle = wheelAngle;
    para.wheelradius = wheelradius;
    para.maxDrawAngle = maxDrawAngle;
    return para;
}

PointF generate_dynamic_world_vert_single(DynamicPara para, int cam, double car_angle, double* w_y, double* w_x, int id, double worldy)
{
    float junctionStart = (float)(RAD_TO_DEG(worldy) / para.radiusCenter);
    int index_fortotal;

    if (car_angle >= 0)
    {
        //index_fortotal = (int)(para.totalPoint - ((junctionStart) / para.maxDrawAngle) * para.totalPoint);
        index_fortotal = (int)((double)para.totalPoint - ((double)junctionStart / para.maxDrawAngle) * (double)para.totalPoint);
    }
    else
    {
        //index_fortotal = (int)((junctionStart / para.maxDrawAngle * para.totalPoint) + 0.5) - 1;
        index_fortotal = (int)( ((double)junctionStart / para.maxDrawAngle * (double)para.totalPoint) + 0.5 ) - 1;
    }

    //int index_fortotal = (int)((junctionStart - lineStart) / angleSpace)
    /*計算連接點處並分配於buffer 給水平線用*/
    double angle, adjustedAngle;
    float fsin, fcos;
    float carCenterX = 0.0f, carCenterY = 0.0f;
    double outerAngle = 0.0;
    double innerAngle = 0.0;
    PointF point = 
    {
        .X = 0,
        .Y = 0
    };

    if (cam == CAM_NUM_FRONT)
    {
        angle = (double)index_fortotal * para.angleSpace + para.angleStart;
        adjustedAngle = angle + 180.0;


        fsin = (float)sin(DEG_TO_RAD(angle));
        fcos = (float)cos(DEG_TO_RAD(angle));

        carCenterX = (float)(para.xCenter) + (float)(para.radiusCenter) * fcos;
        carCenterY = ((float)(para.yCenter) + (float)(para.radiusCenter) * fsin);

        if (car_angle < 0)
        {
            outerAngle = 180.0 + angle + para.wheelAngle ;
            innerAngle = angle - para.wheelAngle ;
        }
        else
        {
            outerAngle = angle + para.wheelAngle ;
            innerAngle = angle - para.wheelAngle + 180.0 ;
        }
        if (w_x[id * 2] <= 0)
        {
            point.X = carCenterX + (float)para.wheelradius * (float)cos(DEG_TO_RAD(outerAngle));
            point.Y = -para.car_LONG_HALF + carCenterY + (float)para.wheelradius * (float)sin(DEG_TO_RAD(outerAngle));
        }
        else
        {
            point.X = carCenterX + (float)para.wheelradius * (float)cos(DEG_TO_RAD(innerAngle));
            point.Y = -para.car_LONG_HALF + carCenterY + (float)para.wheelradius * (float)sin(DEG_TO_RAD(innerAngle));
        }
    }
    else if (cam == CAM_NUM_BACK)
    {
        angle = (double)index_fortotal * para.angleSpace + para.angleStart;
        fsin = (float)sin(DEG_TO_RAD(angle));
        fcos = (float)cos(DEG_TO_RAD(angle));

        carCenterX = (float)(para.xCenter) + (float)(para.radiusCenter) * fcos;
        carCenterY = ((float)(para.yCenter) + (float)(para.radiusCenter) * fsin);
        if (car_angle < 0)
        {
            outerAngle = 180.0 + angle - para.wheelAngle;
            innerAngle = angle + para.wheelAngle;
        }
        else
        {
            outerAngle = angle - para.wheelAngle;
            innerAngle = 0.0 - (180.0 - angle) + para.wheelAngle;
        }

        if (w_x[id * 2] <= 0)
        {
            point.X = carCenterX + (float)para.wheelradius * (float)cos(DEG_TO_RAD(outerAngle));
            point.Y = para.car_LONG_HALF + carCenterY + (float)para.wheelradius * (float)sin(DEG_TO_RAD(outerAngle));
        }
        else
        {
            point.X = carCenterX + (float)para.wheelradius * (float)cos(DEG_TO_RAD(innerAngle));
            point.Y = para.car_LONG_HALF + carCenterY + (float)para.wheelradius * (float)sin(DEG_TO_RAD(innerAngle));
        }
    }

    return point;
}
void generate_dynamic_world_vert(DynamicPara para, int cam, line_vert_t* active, double car_angle, double* w_y, double* w_x, int i, int narrow)
{
    float fsin, fcos;
    double angle, adjustedAngle;
    double outerAngle = 0.0;
    double innerAngle = 0.0;
    float carCenterX = 0.0f, carCenterY = 0.0f;
    
    //line_buffer_allocate(active, para.totalPoint * 2, 0);
    float color_gap = 1.0f / (float)para.totalPoint;
    for (int ix = 0; ix < para.totalPoint; ix++)
    {
        int index = ix * 2;

        if (cam == CAM_NUM_FRONT)
        {
            angle = (double)ix * para.angleSpace + para.angleStart;
            adjustedAngle = angle + 180.0;
            fsin = (float)sin(DEG_TO_RAD(angle));
            fcos = (float)cos(DEG_TO_RAD(angle));

            carCenterX = (float) (para.xCenter) + (float)(para.radiusCenter) * fcos;
            carCenterY = ((float)(para.yCenter) + (float)(para.radiusCenter) * fsin);

            if (car_angle < 0)
            {
                outerAngle = 180.0 + angle + para.wheelAngle ;
                innerAngle = angle - para.wheelAngle ;
            }
            else
            {
                outerAngle = angle + para.wheelAngle ;
                innerAngle = angle - para.wheelAngle + 180.0 ;
            }
            if (w_x[i * 2 + narrow] <= 0)
            {
                active->world[index + 0] = carCenterX + (float)para.wheelradius * (float)cos(DEG_TO_RAD(outerAngle));
                active->world[index + 1] = -para.car_LONG_HALF + carCenterY + (float)para.wheelradius * (float)sin(DEG_TO_RAD(outerAngle));
            }
            else
            {
                active->world[index + 0] = carCenterX + (float)para.wheelradius * (float)cos(DEG_TO_RAD(innerAngle));
                active->world[index + 1] = -para.car_LONG_HALF + carCenterY + (float)para.wheelradius * (float)sin(DEG_TO_RAD(innerAngle));
            }
            active->textmap[ix] = color_gap * (float)ix;
        }
        else if (cam == CAM_NUM_BACK)
        {
            angle = (double)ix * para.angleSpace + para.angleStart;
            fsin = (float)sin(DEG_TO_RAD(angle));
            fcos = (float)cos(DEG_TO_RAD(angle));

            carCenterX = (float)(para.xCenter) + (float)(para.radiusCenter) * fcos;
            carCenterY = ((float)(para.yCenter) + (float)(para.radiusCenter) * fsin);
            if (car_angle < 0)
            {
                outerAngle = 180.0 + angle - para.wheelAngle;
                innerAngle = angle + para.wheelAngle;
            }
            else
            {
                outerAngle = angle - para.wheelAngle;
                innerAngle = 0.0 - (180.0 - angle) + para.wheelAngle;
            }
            if (w_x[i * 2 + narrow] <= 0)
            {
                active->world[index + 0] = carCenterX + (float)para.wheelradius * (float)cos(DEG_TO_RAD(outerAngle));
                active->world[index + 1] = para.car_LONG_HALF + carCenterY + (float)para.wheelradius * (float)sin(DEG_TO_RAD(outerAngle));
            }
            else
            {
                active->world[index + 0] = carCenterX + (float)para.wheelradius * (float)cos(DEG_TO_RAD(innerAngle));
                active->world[index + 1] = para.car_LONG_HALF + carCenterY + (float)para.wheelradius * (float)sin(DEG_TO_RAD(innerAngle));
            }
            active->textmap[ix] = color_gap * (float)ix;
        }


    }

}

void find_closeP2(TransForDylineData Findp2data, PointF* p2, double TOLERANCE, DISPLAY_PGL *display)
{

    int id = Findp2data.id;
    int cam = Findp2data.cam;
    double car_angle = Findp2data.car_angle;
    double* w_y = Findp2data.w_y;
    double* w_x = Findp2data.w_x;
    DynamicPara para = Findp2data.dynamicPara;
    int director = Findp2data.director;
    double worldy = Findp2data.worldy;

    double* world2pixel_matrix = Display_pgl.homo.AVM_TO_Pixel[cam];


    PointF p1_w = {Findp2data.p1.X, Findp2data.p1.Y};
    double number = 0.1;
    int iter = 0;
    double stepper = 30;
    double last_cur_Y = p1_w.Y;
    int maxIterations = 10000;
    float cur_Y;
    do
    {
        PointF temp = generate_dynamic_world_vert_single(para, cam, car_angle, w_y, w_x, id, worldy + (number) * director);
        p2->X = temp.X;
        p2->Y = temp.Y;

        if (display->view.type == FISHEYE_DISTORTION)
        {
            double coorToPixel[3] = { (double)p2->X, (double)p2->Y, 0.0 };
            window_mat(world2pixel_matrix, coorToPixel);
            double wp[3] = { coorToPixel[0], coorToPixel[1], 0 };
            world2cam(wp, coorToPixel, cam);
            //cur_Y = (float)coorToPixel[1] / 360.0f;
            cur_Y = (float)coorToPixel[1] / ((float)Display_pgl.fisheye.parameter[cam].imgHeight / 2.0f);
        }
        else
        {
            cur_Y = p2->Y;
        }


        if (fabs(cur_Y - p1_w.Y) < TOLERANCE)
        {
            break;
        }

        if ((last_cur_Y - p1_w.Y) < 0)
        {
            stepper /= 10;
        }

        if (director < 0)
        {
            if ( (display->view.type == AVM_3D_VIEW || display->view.type == AVM_2D_VIEW) && cam == 1)
            {
                if (cur_Y < p1_w.Y)
                {
                    number -= stepper;
                }
                else
                {
                    number += stepper;
                }
            }
            else
            {
                if (cur_Y < p1_w.Y)
                {
                    number += stepper;
                }
                else
                {
                    number -= stepper;
                } 
            }

        }
        else
        {
            if ( (display->view.type == AVM_3D_VIEW|| display->view.type == AVM_2D_VIEW) && cam == 1)
            {
                if (cur_Y < p1_w.Y)
                {
                    number += stepper;
                }
                else
                {
                    number -= stepper;
                }
            }
            else
            {
                if (cur_Y < p1_w.Y)
                {
                    number -= stepper;
                }
                else
                {
                    number += stepper;
                } 
            }

        }
        last_cur_Y = cur_Y;
        iter++;
    } while (iter < maxIterations);


}

void cal_junctionPoint(int cam, DYMAP* map, int mapSize, JunctionItem* table, double car_angle, double* w_y, double* w_x, DISPLAY_PGL *display)
{
    int size = mapSize;
    table->cur_id = 0;

    for (int i = 0; i < size; i++)
    {
        table->junP1[i].X = 0.0f ;
        table->junP1[i].Y = 0.0f ;
        table->junP2[i].X = 0.0f ;
        table->junP2[i].Y = 0.0f ;
        table->ver_width[i] = 1;
    }

    for (int i = 0; i < size; i++)
    {
        int id = map[i].id;

        DynamicPara para = cal_base_dynamic_line_parameter(cam, car_angle, w_y, w_x, id, 0, 0);

        double worldy = (double)map[i].y;
        int director = 1;

        if (cam == CAM_NUM_FRONT)
        {
            worldy += para.car_LONG_HALF;
            director = (worldy > w_y[id * 2]) ? -1 : 1;
            worldy -= w_y[id * 2];
        }
        else
        {
            worldy -= para.car_LONG_HALF;
            director = (worldy > w_y[id * 2]) ? -1 : 1;
            worldy -= w_y[id * 2];
        }

                //car_angle = 26.0;
        PointF p1 = generate_dynamic_world_vert_single(para, cam, car_angle, w_y, w_x, id, worldy);
        PointF p2 = generate_dynamic_world_vert_single(para, cam, car_angle, w_y, w_x, id, worldy + (double)(map[i].hor_width * director) * 3.2);
#if 1

    if (display->view.type == AVM_3D_VIEW || display->view.type == AVM_2D_VIEW)
    {
        float targetY = (130.0f / 25.0f / 1.70f) * (float)map[i].hor_width * director;
        if (cam == 1) targetY *= -1;
        PointF p1_w = {p1.X, p1.Y - targetY};

        TransForDylineData data;
        data.worldy = worldy;
        data.dynamicPara = para;
        data.id = id;
        data.car_angle = car_angle;
        data.director = director;
        data.w_x = w_x;
        data.w_y = w_y;
        data.p1 = p1_w;
        data.cam = cam;
        find_closeP2(data, &p2, 0.5, display);
    }
    else
    {

        double* world2pixel_matrix = Display_pgl.homo.AVM_TO_Pixel[cam];

        double coorToPixel[3] = { (double)p1.X, (double)p1.Y, 0.0 };
        window_mat(world2pixel_matrix, coorToPixel);
        double wp[3] = { coorToPixel[0], coorToPixel[1], 0.0 };
        world2cam(wp, coorToPixel, cam);

        director *= -1;

        float targetY = (0.01f) * (float)map[i].hor_width * 2.0f;
        targetY *= director;
        if (cam == 1) director *= -1;

        //PointF p1_w = {(float)coorToPixel[0], (float)coorToPixel[1] / 360.0f + targetY};
        PointF p1_w = {(float)coorToPixel[0], (float)coorToPixel[1] / ((float)Display_pgl.fisheye.parameter[cam].imgHeight / 2.0f) + targetY};


        TransForDylineData data ;
        data.worldy = worldy;
        data.dynamicPara = para;
        data.id = id;
        data.car_angle = car_angle;
        data.director = director;
        data.w_x = w_x;
        data.w_y = w_y;
        data.p1 = p1_w;
        data.cam = cam;
        find_closeP2(data, &p2, 0.003, display);

    }

#endif
        if (cam == CAM_NUM_FRONT)
        {
            if (p1.Y <= p2.Y)
            {
                table->junP1[table->cur_id] = p1;
                p2.Y *=1.005f;
                table->junP2[table->cur_id] = p2;
            }
            else
            {
                table->junP1[table->cur_id] = p2;
                p1.Y *=1.005f;
                table->junP2[table->cur_id] = p1;
            }
        }
        else
        {
            if (p1.Y <= p2.Y)
            {
                table->junP1[table->cur_id] = p2;
                p1.Y *=1.005f;
                table->junP2[table->cur_id] = p1;
            }
            else
            {
                table->junP1[table->cur_id] = p1;
                p2.Y *=1.005f;
                table->junP2[table->cur_id] = p2;
            }
        }


        table->ver_width[table->cur_id] = map[i].ver_width;
        table->cur_id = table->cur_id + 1;
    }

//debug
#if 0
    printf("--------------------------------------\n");

    for (int i = 0; i < size; i++)
    {
        double worldy = (double)map[i].y;
        int id = map[i].id;
        int dir  = (worldy > w_y[id * 2]) ? -1 : 1;
        DynamicPara para = cal_base_dynamic_line_parameter(cam, car_angle, w_y, w_x, id, 0, 0);
        printf("[%d] ID: %d, worldy[%d], direct[%d], width[%d]\n",i,map[i].id,map[i].y,dir,map[i].hor_width);
        printf("totalPoint: %d\n", para.totalPoint);
        printf("angleSpace: %lf\n", para.angleSpace);
        printf("angleStart: %lf\n", para.angleStart);
        printf("xCenter: %lf\n", para.xCenter);
        printf("yCenter: %lf\n", para.yCenter);
        printf("radiusCenter: %lf\n", para.radiusCenter);
        printf("car_LONG_HALF: %f\n", para.car_LONG_HALF);
        printf("wheelAngle: %lf\n", para.wheelAngle);
        printf("wheelradius: %lf\n", para.wheelradius);
        printf("maxDrawAngle: %lf\n", para.maxDrawAngle);
    }

    printf("\n");

    for (int i = 0; i < size; i++)
    {
        printf("p1[%d], x = %f, y = %f\n", i , table->junP1[i].X,table->junP1[i].Y);
        
    }
    printf("\n");
    for (int i = 0; i < size; i++)
    {
        printf("p2[%d], x = %f, y = %f\n", i , table->junP2[i].X,table->junP2[i].Y);
    }
    printf("\n");
#endif
//end
}
int cal_vertical_line(int cam, line_vert_t* active, double car_angle, double* w_y, double* w_x, int i, JunctionItem* table)
{
    DynamicPara para = cal_base_dynamic_line_parameter(cam, car_angle, w_y, w_x, i, 0, 0);
    generate_dynamic_world_vert(para, cam, active, car_angle, w_y, w_x, i, 0);
    return para.totalPoint;
}

int cal_horizontal_line(int cam, line_vert_t* active, double car_angle, double* w_y, double* w_x, int i)
{
    
    DynamicPara Lpara = cal_base_dynamic_line_parameter(cam, car_angle, w_y, w_x, i, 0, 20);
    DynamicPara Rpara = cal_base_dynamic_line_parameter(cam, car_angle, w_y, w_x, i, 1, 20);
    line_vert_t left; 
    line_vert_t right;
    
    generate_dynamic_world_vert(Lpara, cam, &left, car_angle, w_y, w_x, i, 0);
    generate_dynamic_world_vert(Rpara, cam, &right, car_angle, w_y, w_x, i, 1);

    if (car_angle > -0.05 && car_angle < 0.0)
    {
        
        float max_y_axis_vert = max(left.world[1], right.world[1]);
        active->world[0] = left.world[0];
        active->world[1] = max_y_axis_vert;
        active->world[2] = right.world[0];
        active->world[3] = max_y_axis_vert;
    }
    else if(car_angle < 0.05 && car_angle > 0.0)
    {
        float max_y_axis_vert = max(left.world[(Lpara.totalPoint - 2) * 2 + 1] , right.world[(Rpara.totalPoint - 1) * 2 + 1]);
        active->world[0] = left.world[(Lpara.totalPoint - 2) * 2 + 0];
        active->world[1] = max_y_axis_vert;

        active->world[2] = right.world[(Rpara.totalPoint - 1) * 2 + 0];
        active->world[3] = max_y_axis_vert;
    }
    else if (car_angle < 0.0)
    {
        active->world[0] = left.world[0];
        active->world[1] = left.world[1];
        active->world[2] = right.world[0];
        active->world[3] = right.world[1];
    }
    else
    {
        float Toleration = (cam == 0) ? 1.2f : -1.2f;
        active->world[0] = left.world[(Lpara.totalPoint - 2) * 2 + 0];
        active->world[1] = left.world[(Lpara.totalPoint - 2) * 2 + 1] + Toleration;
        active->world[2] = right.world[(Rpara.totalPoint - 1) * 2 + 0];
        active->world[3] = right.world[(Rpara.totalPoint - 1) * 2 + 1];
    }
    
    // free(left.world);
    // free(right.world);
    
    return Lpara.totalPoint;
}

void tyajectory_line_fish_distortion_width(float* linePoint, float* point, int index, DYNAMIC_PACKAGE data, DISPLAY_PGL *display)
{
    int style = data.current_style;
    int current_line = data.current_line;
    float lineWidth = LINEWIDTH_NOR * (float)data.linewidth[style][current_line] / 2.0f;
    float ratio = (float)g_PANEL_WIDTH/(float)g_PANEL_HEIGHT;

    if (1)
    {
        float offsetX = line_modify_flag.normal.X * lineWidth / 2.0f;
        float offsetY = line_modify_flag.normal.Y * lineWidth*(ratio) / 2.0f;
        linePoint[index + 0] = point[0] - offsetX;
        linePoint[index + 1] = point[1] - offsetY;
        linePoint[index + 2] = point[2];
        linePoint[index + 3] = point[0] + offsetX;
        linePoint[index + 4] = point[1] + offsetY;
        linePoint[index + 5] = point[2];
    
        //flag
        if(display->g_enable_y_axis_limit == 1)
        {
            if (line_modify_flag.limit_over_y < linePoint[index + 1])
            {
                linePoint[index + 0] = point[0] - lineWidth/2.0f ;
                linePoint[index + 1] = line_modify_flag.limit_over_y;
                linePoint[index + 4] = line_modify_flag.limit_over_y;
            }
    
            if (line_modify_flag.limit_over_y < linePoint[index + 4])
            {
                linePoint[index + 3] = point[0] + lineWidth/2.0f;
                linePoint[index + 1] = line_modify_flag.limit_over_y;
                linePoint[index + 4] = line_modify_flag.limit_over_y;
            }
        }
    }
    // else //line end&top using orig method <-- . -->
    // {
    //     linePoint[index + 0] = point[0] - lineWidth;
    //     linePoint[index + 1] = point[1];
    //     linePoint[index + 2] = point[2];
    //     linePoint[index + 3] = point[0] + lineWidth;
    //     linePoint[index + 4] = point[1];
    //     linePoint[index + 5] = point[2];
    // }
}

void tyajectory_line_fish_scene_slop_width(float* linePoint, float* point, int index, DYNAMIC_PACKAGE data, DISPLAY_PGL *display)
{
    int style = data.current_style;
    int current_line = data.current_line;
    float gl_width = (float)m_Aligment_data->viewport.W ;
    float LINE_BASIC = 1.0f / gl_width * 10.0f;
    float lineWidth = LINE_BASIC * (float)data.linewidth[style][current_line] / 2.0f;
    // if(m_sceneview_CustomlineWidth >= 1.0f)
    // {
    //     lineWidth *= m_sceneview_CustomlineWidth;
    // }
    float ratio = (float) m_Aligment_data->viewport.W/(float)m_Aligment_data->viewport.H;
    float l_index = (float)data.current_line;
    float z_bufferValue = 0.0f;
    if (1)
    {
        float offsetX = line_modify_flag.normal.X * lineWidth / 2.0f;
        float offsetY = line_modify_flag.normal.Y * lineWidth*(ratio) / 2.0f;
        linePoint[index + 0] = point[0] - offsetX;
        linePoint[index + 1] = point[1] - offsetY;
        linePoint[index + 2] = point[2] + z_bufferValue;
        linePoint[index + 3] = point[0] + offsetX;
        linePoint[index + 4] = point[1] + offsetY;
        linePoint[index + 5] = point[2] + z_bufferValue;
    
        //flag
        if(display->g_enable_y_axis_limit == 1)
        {
            if (line_modify_flag.limit_over_y < linePoint[index + 1])
            {
                linePoint[index + 0] = point[0] - lineWidth/2.0f ;
                linePoint[index + 1] = line_modify_flag.limit_over_y;
                linePoint[index + 4] = line_modify_flag.limit_over_y;
            }
    
            if (line_modify_flag.limit_over_y < linePoint[index + 4])
            {
                linePoint[index + 3] = point[0] + lineWidth/2.0f;
                linePoint[index + 1] = line_modify_flag.limit_over_y;
                linePoint[index + 4] = line_modify_flag.limit_over_y;
            }
        }
    }
}
void tyajectory_line_fish_distortion_slop_width(float* linePoint, float* point, int index, DYNAMIC_PACKAGE data, DISPLAY_PGL *display)
{
    int style = data.current_style;
    int current_line = data.current_line;
    float lineWidth = LINEWIDTH_NOR * (float)data.linewidth[style][current_line] / 2.0f;
    float ratio = (float)g_PANEL_WIDTH/(float)g_PANEL_HEIGHT;
    float l_index = (float)data.current_line;
    float z_bufferValue = l_index * 0.01f * 2.0f;
    if (1)
    {
        float offsetX = line_modify_flag.normal.X * lineWidth / 2.0f;
        float offsetY = line_modify_flag.normal.Y * lineWidth*(ratio) / 2.0f;
        linePoint[index + 0] = point[0] - offsetX;
        linePoint[index + 1] = point[1] - offsetY;
        linePoint[index + 2] = point[2] + z_bufferValue;
        linePoint[index + 3] = point[0] + offsetX;
        linePoint[index + 4] = point[1] + offsetY;
        linePoint[index + 5] = point[2] + z_bufferValue;
    
        //flag
        if(display->g_enable_y_axis_limit == 1)
        {
            if (line_modify_flag.limit_over_y < linePoint[index + 1])
            {
                linePoint[index + 0] = point[0] - lineWidth/2.0f ;
                linePoint[index + 1] = line_modify_flag.limit_over_y;
                linePoint[index + 4] = line_modify_flag.limit_over_y;
            }
    
            if (line_modify_flag.limit_over_y < linePoint[index + 4])
            {
                linePoint[index + 3] = point[0] + lineWidth/2.0f;
                linePoint[index + 1] = line_modify_flag.limit_over_y;
                linePoint[index + 4] = line_modify_flag.limit_over_y;
            }
        }
    }

}
void tyajectory_line_3d_avm_width(float* linePoint, float* point, int index, DYNAMIC_PACKAGE data, float angle, DISPLAY_PGL *display)
{
    int style = data.current_style;
    int current_line = data.current_line;
    float lineWidth = LINEWIDTH_NOR * (float)data.linewidth[style][current_line];

    float offsetX = line_modify_flag.normal.X * lineWidth / 2.0f;
    float offsetY = line_modify_flag.normal.Y * lineWidth / 2.0f;

    linePoint[index + 0] = point[0] - offsetX;
    linePoint[index + 1] = point[1];
    linePoint[index + 2] = point[2] - offsetY;
    linePoint[index + 3] = point[0] + offsetX;
    linePoint[index + 4] = point[1];
    linePoint[index + 5] = point[2] + offsetY;
}
void tyajectory_line_top_avm_width(float* linePoint, float* point, int index, DYNAMIC_PACKAGE data, float angle, DISPLAY_PGL *display)
{
    int style = data.current_style;
    int current_line = data.current_line;
    float lineWidth = LINEWIDTH_NOR * (float)data.linewidth[style][current_line];

    if (angle < 45.0)
    {
         linePoint[index + 0] = point[0];
         linePoint[index + 1] = point[1];
         linePoint[index + 2] = point[2] + lineWidth / 2.0f;
         linePoint[index + 3] = point[0];
         linePoint[index + 4] = point[1];
         linePoint[index + 5] = point[2] - lineWidth / 2.0f;
    }
    else
    {
        linePoint[index + 0] = point[0] - lineWidth / 2.0f;
        linePoint[index + 1] = point[1];
        linePoint[index + 2] = point[2];
        linePoint[index + 3] = point[0] + lineWidth / 2.0f;
        linePoint[index + 4] = point[1];
        linePoint[index + 5] = point[2];
    }
}

void trajector_line_colormap(float* linePoint, int accum_index, int cur_index, float* textmap, float angle, DYNAMIC_PACKAGE data)
{
    float inverse = 0.0f;
    switch(data.current_cam)
    {
        case 0:
            if (angle > 0.0f) inverse = 1.0f;
            break;
        case 1:
            if (angle <= 0.0f) inverse = 0.0f;
            else inverse = 1.0f;
            break;
    }
    float value = fabs(inverse - textmap[cur_index / 2]);
    value = (value >= 0.99f) ? 0.99f : (value < 0.01f) ? 0.01f : value;
    linePoint[(accum_index * 2) + 0] = 0.0f;
    linePoint[(accum_index * 2) + 1] = value;
    linePoint[(accum_index * 2) + 2] = 1.0f;
    linePoint[(accum_index * 2) + 3] = value;
}
void trajectory_line_width(float* linePoint, float angle, float* point, int index, DYNAMIC_PACKAGE data, DISPLAY_PGL *display)
{

    if (display->view.type == FISHEYE_DISTORTION || display->view.type == FISHEYE_STATIC_DISTORTION) //fisheye
    {
        if(display->view.director != slop)
        {
            tyajectory_line_fish_distortion_width(linePoint, point, index ,data, display);
        }
        else
        {
            tyajectory_line_fish_distortion_slop_width(linePoint, point, index ,data, display);
        }
    }
    else if(display->view.type == FISHEYE_STATIC_SCENE )
    {
        tyajectory_line_fish_scene_slop_width(linePoint, point, index ,data, display);
    }
    else if (display->view.type == AVM_3D_VIEW )
    {
        tyajectory_line_3d_avm_width(linePoint, point, index, data, angle , display);
    }
    else if(display->view.type == AVM_2D_VIEW)
    {
        tyajectory_line_top_avm_width(linePoint, point, index, data, angle , display);
    }
    else { return; }
}

void trajectory_line_horizon_width(float* linePoint, float angle, float* point, int index, DYNAMIC_PACKAGE data, DISPLAY_PGL* display)
{
    float lineWidth = 0.0f;
    float director = -1.0f;
    float ratio = 1280.0f / 720.0f; 

    if (display->view.type == FISHEYE_DISTORTION || display->view.type == FISHEYE_STATIC_DISTORTION) 
    {
       int style = data.current_style;
        int current_line = data.current_line;
        lineWidth = LINEWIDTH_NOR * (float)data.linewidth[style][current_line]/2.0f * ratio;
    }
    else if (display->view.type == AVM_3D_VIEW || display->view.type == AVM_2D_VIEW ) 
    {
        int style = data.current_style;
        int current_line = data.current_line;
        lineWidth = LINEWIDTH_NOR * (float)data.linewidth[style][current_line];
        if (data.current_cam == 0) director = 1.0f;
    }
    else
    {
        lineWidth = LINEWIDTH_NOR * 2;
    }



    if (angle < 45.0)
    {
        if (display->view.type == FISHEYE_DISTORTION || display->view.type == FISHEYE_STATIC_DISTORTION)
        {
            linePoint[index + 0] = point[0];
            linePoint[index + 1] = point[1];
            linePoint[index + 2] = point[2] + 0.2f;
            linePoint[index + 3] = point[0];
            linePoint[index + 4] = point[1];
            linePoint[index + 5] = point[2] + 0.2f;

            if (display->rect_patch.line_growth == 1)
            {
                linePoint[index + 4] = point[1] + (lineWidth * director);
            }
            else
            {
                linePoint[index + 1] = point[1] - (lineWidth * director / 2.0f);
                linePoint[index + 4] = point[1] + (lineWidth * director / 2.0f);
            }
        }
        else if (display->view.type == AVM_3D_VIEW || display->view.type == AVM_2D_VIEW )
        {
            linePoint[index + 0] = point[0];
            linePoint[index + 1] = point[1];
            linePoint[index + 2] = point[2];
            linePoint[index + 3] = point[0];
            linePoint[index + 4] = point[1];
            linePoint[index + 5] = point[2];
            if (display->rect_patch.line_growth == 1)
            {
                linePoint[index + 5] = point[2] + (lineWidth * director);
            }
            else
            {
                linePoint[index + 2] = point[2] - (lineWidth * director / 2.0f);
                linePoint[index + 5] = point[2] + (lineWidth * director / 2.0f);
            }
        }
    }

}

void mat_mult(double *H_world, const double ptMat[3], double *result) 
{
    for (int i = 0; i < 3; i++) 
    {
        result[i] = 0;
        for (int j = 0; j < 3; j++) 
        {
            result[i] += H_world[i * 3 + j] * ptMat[j];
        }
    }
}

void trajectory_path_point(float* carReversePath, float locationX, float locationY, int cam, DISPLAY_PGL* display)
{
    if (display->view.type == FISHEYE_DISTORTION)
    {
        
        double coorToPixel[3] = { locationX, locationY, 0.0 };
        if (cam == 1) coorToPixel[0] = (-locationX);

        window_mat(Display_pgl.homo.AVM_TO_Pixel[cam], coorToPixel);
        double wp[3] = { coorToPixel[0], coorToPixel[1], 0.0 };
        //if (cam == 0)
        {
            double input[3] = {coorToPixel[0],coorToPixel[1],1.0};
            double PP[3];
            mat_mult(Display_pgl.homo.GoldensampleRotate[cam], input, PP);
            double transformedX = PP[0] / PP[2];
            double transformedY = PP[1] / PP[2];
            wp[0] = transformedX;
            wp[1] = transformedY;
        }
        world2cam(wp, coorToPixel, cam);

        carReversePath[0] = (float)(coorToPixel[0] / Display_pgl.fisheye.parameter[cam].imgWidth * 2 - 1);
        carReversePath[1] = (float)(1.0 - coorToPixel[1] / Display_pgl.fisheye.parameter[cam].imgHeight * 2);
        carReversePath[2] = 0.01f;
    }
    else if (display->view.type == AVM_3D_VIEW || display->view.type == AVM_2D_VIEW )
    {

        carReversePath[0] = (locationX) / 130.0f;
         if (cam == 1) carReversePath[0] = (-locationX)/130.0f;
        carReversePath[1] = 0.01f;
        carReversePath[2] = (locationY) / 130.0f;
    }
    else if(display->view.type == FISHEYE_STATIC_DISTORTION)
    {
        double coorToPixel[3] = { locationX, locationY, 0.0 };
        double wp[3] = { coorToPixel[0], coorToPixel[1], 0.0 };
        {
            double input[3] = {coorToPixel[0],  coorToPixel[1], 1.0};
            double PP[3];
            mat_mult(Display_pgl.homo.GoldensampleRotate[cam], input, PP);
            double transformedX = PP[0] / PP[2];
            double transformedY = PP[1] / PP[2];
            wp[0] = transformedX;
            wp[1] = transformedY;
        }
        world2cam(wp, coorToPixel, cam);
        carReversePath[0] = (float)(coorToPixel[0] / Display_pgl.fisheye.parameter[cam].imgWidth * 2 - 1);
        carReversePath[1] = (float)(1.0 - coorToPixel[1] / Display_pgl.fisheye.parameter[cam].imgHeight * 2);
        carReversePath[2] = 0.01f;
    }
    else if(display->view.type == FISHEYE_STATIC_SCENE)
    {
        double coorToPixel[3] = { locationX, locationY, 0.0 };
        double wp[3] = { coorToPixel[0], coorToPixel[1], 0.0 };
        carReversePath[0] = (float)(coorToPixel[0] / PARAM_SCENEVIEW_WIDTH * 2 - 1);
        carReversePath[1] = (float)(1.0 - coorToPixel[1] / PARAM_SCENEVIEW_HEIGHT * 2);
        carReversePath[2] = 0.01f;
    }
    else
    {
        carReversePath[0] = locationX;
        carReversePath[1] = locationY;
        carReversePath[2] = 0.0f;
    }
}
void set_trajectory_path_texture_line(float* DynamicTexture,int* indexnum,float* colormap,float angle, int index, DYNAMIC_PACKAGE data, DISPLAY_PGL* display)//, int* indexnum
{
    trajector_line_colormap(DynamicTexture, indexnum, index,colormap, angle, data);
}
void set_trajectory_path_point_line(float* DynamicVert, int* indexnum, float* location, float* angle, int index, DYNAMIC_PACKAGE data,  DISPLAY_PGL* display)//, int* indexnum
{
    float point[3];
    float pointlast[3];
    float anglepoint[2];
    float anglepointlast[2];


    trajectory_path_point(point, location[index + 0], location[index + 1],  data.current_cam,display);
    if (index >= 2)
        trajectory_path_point(pointlast, location[index - 2], location[index - 1],  data.current_cam, display);
    else
    {
        display->rect_patch.limit_over_y = point[1];
        //return;
        trajectory_path_point(pointlast, location[index + 2], location[index + 3],  data.current_cam, display);
    }

    if (display->view.type == FISHEYE_DISTORTION || display->view.type == FISHEYE_STATIC_DISTORTION || display->view.type == FISHEYE_STATIC_SCENE)
    {
        anglepoint[0] = point[0];
        anglepoint[1] = point[1];
        anglepointlast[0] = pointlast[0];
        anglepointlast[1] = pointlast[1];
        if(index == 0)
        {
            anglepoint[0] = pointlast[0];
            anglepoint[1] = pointlast[1];
            anglepointlast[0] = point[0];
            anglepointlast[1] = point[1];
        }
    }
    else if (display->view.type == AVM_3D_VIEW || display->view.type == AVM_2D_VIEW )
    {
        anglepoint[0] = point[0];
        anglepoint[1] = point[2];
        anglepointlast[0] = pointlast[0];
        anglepointlast[1] = pointlast[2];
    }
    else
    {
        anglepoint[0] = point[0];
        anglepoint[1] = point[1];
        anglepointlast[0] = pointlast[0];
        anglepointlast[1] = pointlast[1];
    }

    float lineAngle = *angle;

    if(index == 0)
    {
        line_modify_flag.limit_over_y = point[1];
    }

     PointF end = 
     {
        .X = anglepointlast[0], 
        .Y = anglepointlast[1]
     };
    PointF start = 
     {
        .X = anglepoint[0], 
        .Y = anglepoint[1]
     };

    PointF diff = {end.X - start.X, end.Y - start.Y};
    float length = sqrt(diff.X * diff.X + diff.Y * diff.Y);
    PointF tangent = {diff.X / length, diff.Y / length};

    line_modify_flag.normal.X = -tangent.Y;
    line_modify_flag.normal.Y = tangent.X;

    if (index == 0)
    {
        display->circle_patch.Point[0].X = anglepointlast[0];
        display->circle_patch.Point[0].Y = anglepointlast[1];
        //display->circle_patch.ANGLE[0] = (float)Math.Atan2(tangent.Y, tangent.X);
    }

    if(display->rect_patch.g_finaly_draw_point == 1)
    {
        display->circle_patch.Point[1].X = anglepoint[0];
        display->circle_patch.Point[1].Y = anglepoint[1];
        //display->circle_patch.ANGLE[1] = (float)Math.Atan2(tangent.Y, tangent.X);
    }

    trajectory_line_width(DynamicVert, lineAngle, point, (*indexnum) * 3, data, display);
    *indexnum = *indexnum  + 2;
    *angle = lineAngle;
}

float CalculateStripLength(float endPoint, float startPoint, int div)
{
    return (endPoint - startPoint) / (float)div;
}

void ProcessForOrigBinary(line_vert_t* DynamicVert,  DYNAMIC_PACKAGE data, int div, float strip_x_length, float strip_y_length, int* num, DISPLAY_PGL* display)
{
    float point[3];
    float pointlast[3];
    float point_left[3];
    float point_right[3];
    for (int i = 0; i < div - 1; i++)
    {
        trajectory_path_point(pointlast, DynamicVert->world[0] + (float)(i) * strip_x_length, DynamicVert->world[1] + (float)(i) * strip_y_length,  data.current_cam, display);
        trajectory_path_point(point, DynamicVert->world[0] + (float)(i + 1) * strip_x_length, DynamicVert->world[1] + (float)(i + 1) * strip_y_length,  data.current_cam, display);
        trajectory_line_horizon_width(DynamicVert->vert, 0, pointlast, (*num) * 3, data, display);
        *num += 2;
        trajectory_line_horizon_width(DynamicVert->vert, 0, point, (*num) * 3, data, display);
        *num += 2;
    }

    trajectory_path_point(pointlast, DynamicVert->world[0] + (float)(div - 1) * strip_x_length, DynamicVert->world[1] + (float)(div - 1) * strip_y_length, data.current_cam, display);
    trajectory_path_point(point, DynamicVert->world[2], DynamicVert->world[3], data.current_cam, display);
    trajectory_line_horizon_width(DynamicVert->vert, 0, pointlast, (*num) * 3, data, display);
    *num += 2;
    trajectory_line_horizon_width(DynamicVert->vert, 0, point, (*num) * 3, data, display);
    *num += 2;
}

void ProcessForTableLeftOrigRight(line_vert_t* DynamicVert, DYNAMIC_PACKAGE data, int div, float strip_x_length, float strip_y_length,int* num, DISPLAY_PGL* display)
{
    float point[3];
    float pointlast[3];
    float point_left[3];
    float point_right[3];

    if (display->view.type == FISHEYE_DISTORTION)
    {
        trajectory_path_point(pointlast, DynamicVert->world[4 + 0], DynamicVert->world[4 + 1] , data.current_cam, display);
        trajectory_path_point(point_left, DynamicVert->world[4 + 2], DynamicVert->world[4 + 3] , data.current_cam, display);
        trajectory_path_point(point, DynamicVert->world[4 + 0] + strip_x_length, DynamicVert->world[4 + 1] + strip_y_length, data.current_cam,display);
        DynamicVert->vert[(*num) * 3 + 0] = pointlast[0];
        DynamicVert->vert[(*num) * 3 + 1] = pointlast[1];
        DynamicVert->vert[(*num) * 3 + 2] = pointlast[2];// + (lineWidth * director);
        DynamicVert->vert[(*num) * 3 + 3] = point_left[0];
        DynamicVert->vert[(*num) * 3 + 4] = point_left[1];
        DynamicVert->vert[(*num) * 3 + 5] = point_left[2];
        *num += 2;
        trajectory_line_horizon_width(DynamicVert->vert, 0, point, (*num) * 3, data, display);
        *num += 2;
        for (int i = 1; i < div - 1; i++)
        {
            trajectory_path_point(pointlast, DynamicVert->world[4 + 0] + (float)(i) * strip_x_length, DynamicVert->world[4 + 1] + (float)(i) * strip_y_length, data.current_cam,display);
            trajectory_path_point(point, DynamicVert->world[4 + 0] + (float)(i + 1) * strip_x_length, DynamicVert->world[4 + 1] + (float)(i + 1) * strip_y_length, data.current_cam,display);
            trajectory_line_horizon_width(DynamicVert->vert, 0, pointlast, (*num) * 3, data, display);
            *num += 2;
            trajectory_line_horizon_width(DynamicVert->vert, 0, point, (*num) * 3, data, display);
            *num += 2;
        }
    }


    if (display->view.type != FISHEYE_DISTORTION)
    {
        trajectory_path_point(pointlast,  DynamicVert->world[4 + 0] + (float)(div - 1) * strip_x_length, DynamicVert->world[4 + 1] + (float)(div - 1) * strip_y_length, data.current_cam, display);
        trajectory_path_point(point_left, DynamicVert->world[4 + 2] + (float)(div - 1) * strip_x_length, DynamicVert->world[4 + 3] + (float)(div - 1) * strip_y_length, data.current_cam, display);
        trajectory_path_point(point, DynamicVert->world[2], DynamicVert->world[3], data.current_cam, display);
        DynamicVert->vert[(*num) * 3 + 0] = pointlast[0];
        DynamicVert->vert[(*num) * 3 + 1] = pointlast[1];
        DynamicVert->vert[(*num) * 3 + 2] = pointlast[2];// + (lineWidth * director);
        DynamicVert->vert[(*num) * 3 + 3] = point_left[0];
        DynamicVert->vert[(*num) * 3 + 4] = point_left[1];
        DynamicVert->vert[(*num) * 3 + 5] = point_left[2];
        *num += 2;
        trajectory_line_horizon_width(DynamicVert->vert, 0, point, (*num) * 3, data, display);
        *num += 2;
    }

}

void ProcessForOrigLeftTableRight(line_vert_t* DynamicVert, DYNAMIC_PACKAGE data, int div, float strip_x_length, float strip_y_length, int* num, DISPLAY_PGL* display)
{
    float point[3];
    float pointlast[3];
    float point_left[3];
    float point_right[3];
    for (int i = 0; i < div - 1; i++)
    {
        trajectory_path_point(pointlast, DynamicVert->world[0] + (float)(i) * strip_x_length, DynamicVert->world[1] + (float)(i) * strip_y_length, data.current_cam, display);
        trajectory_path_point(point, DynamicVert->world[0] + (float)(i + 1) * strip_x_length, DynamicVert->world[1] + (float)(i + 1) * strip_y_length, data.current_cam, display);
        trajectory_line_horizon_width(DynamicVert->vert, 0, pointlast, (*num) * 3, data, display);
        *num += 2;
        trajectory_line_horizon_width(DynamicVert->vert, 0, point, (*num) * 3, data, display);
        *num += 2;
    }

    trajectory_path_point(point_right, DynamicVert->world[4 + 6], DynamicVert->world[4 + 7], data.current_cam, display);
    trajectory_path_point(point, DynamicVert->world[4 + 4], DynamicVert->world[4 + 5], data.current_cam, display);

    trajectory_path_point(pointlast, DynamicVert->world[0] + (float)(div - 1) * strip_x_length, DynamicVert->world[1] + (float)(div - 1) * strip_y_length, data.current_cam, display);
    trajectory_line_horizon_width(DynamicVert->vert, 0, pointlast, (*num) * 3, data, display);
    *num += 2;
    DynamicVert->vert[(*num) * 3 + 0] = point[0];
    DynamicVert->vert[(*num) * 3 + 1] = point[1];
    DynamicVert->vert[(*num) * 3 + 2] = point[2];// + (lineWidth * director);
    DynamicVert->vert[(*num) * 3 + 3] = point_right[0];
    DynamicVert->vert[(*num) * 3 + 4] = point_right[1];
    DynamicVert->vert[(*num) * 3 + 5] = point_right[2];
    *num += 2;
}

void ProcessForTableLeftTableRight(line_vert_t* DynamicVert, DYNAMIC_PACKAGE data, int div, float strip_x_length, float strip_y_length, int* num, DISPLAY_PGL* display)
{
    float point[3];
    float pointlast[3];
    float point_left[3];
    float point_right[3];
    if (display->view.type == FISHEYE_DISTORTION)
    {
        strip_x_length = (display->view.type == FISHEYE_DISTORTION) ? CalculateStripLength(DynamicVert->world[4 + 4], DynamicVert->world[4 + 0], div) : 0;
        strip_y_length = (display->view.type == FISHEYE_DISTORTION) ? CalculateStripLength(DynamicVert->world[4 + 5], DynamicVert->world[4 + 1], div) : 0;

        trajectory_path_point(pointlast, DynamicVert->world[4 + 0], DynamicVert->world[4 + 1] , data.current_cam, display);
        trajectory_path_point(point_left, DynamicVert->world[4 + 2], DynamicVert->world[4 + 3] , data.current_cam, display);
        trajectory_path_point(point, DynamicVert->world[4 + 0] + strip_x_length, DynamicVert->world[4 + 1] + strip_y_length, data.current_cam, display);
        DynamicVert->vert[(*num) * 3 + 0] = pointlast[0];
        DynamicVert->vert[(*num) * 3 + 1] = pointlast[1];
        DynamicVert->vert[(*num) * 3 + 2] = pointlast[2];// + (lineWidth * director);
        DynamicVert->vert[(*num) * 3 + 3] = point_left[0];
        DynamicVert->vert[(*num) * 3 + 4] = point_left[1];
        DynamicVert->vert[(*num) * 3 + 5] = point_left[2];
        *num += 2;
        trajectory_line_horizon_width(DynamicVert->vert, 0, point,  (*num) * 3, data, display);
        *num += 2;
        for (int i = 1; i < div; i++)
        {
            trajectory_path_point(pointlast, DynamicVert->world[4 + 0] + (float)(i) * strip_x_length, DynamicVert->world[4 + 1] + (float)(i) * strip_y_length,  data.current_cam, display);
            trajectory_path_point(point, DynamicVert->world[4 + 0] + (float)(i + 1) * strip_x_length, DynamicVert->world[4 + 1] + (float)(i + 1) * strip_y_length,  data.current_cam, display);
            trajectory_line_horizon_width(DynamicVert->vert, 0, pointlast,  (*num) * 3, data, display);
            *num += 2;
            trajectory_line_horizon_width(DynamicVert->vert, 0, point,  (*num) * 3, data, display);
            *num += 2;
        }

        trajectory_path_point(point, DynamicVert->world[4 + 4], DynamicVert->world[4 + 5] ,  data.current_cam, display);
        trajectory_path_point(point_right, DynamicVert->world[4 + 6], DynamicVert->world[4 + 7],  data.current_cam, display);
        trajectory_path_point(point_left, DynamicVert->world[4 + 0] + (float)(div) * strip_x_length, DynamicVert->world[4 + 1] + (float)(div) * strip_y_length,  data.current_cam, display);

        trajectory_line_horizon_width(DynamicVert->vert, 0, point_left,  (*num) * 3, data, display);
        *num += 2;
        DynamicVert->vert[(*num) * 3 + 0] = point[0];
        DynamicVert->vert[(*num) * 3 + 1] = point[1];
        DynamicVert->vert[(*num) * 3 + 2] = point[2];
        DynamicVert->vert[(*num) * 3 + 3] = point_right[0];
        DynamicVert->vert[(*num) * 3 + 4] = point_right[1];
        DynamicVert->vert[(*num) * 3 + 5] = point_right[2];
        *num += 2;
    }
    else
    {
        trajectory_path_point(pointlast, DynamicVert->world[4 + 0] + (float)(div - 1) * strip_x_length, DynamicVert->world[4 + 1] + (float)(div - 1) * strip_y_length,  data.current_cam, display);
        trajectory_path_point(point_left, DynamicVert->world[4 + 2] + (float)(div - 1) * strip_x_length, DynamicVert->world[4 + 3] + (float)(div - 1) * strip_y_length, data.current_cam, display);
        trajectory_path_point(point, DynamicVert->world[4 + 4], DynamicVert->world[4 + 5], data.current_cam, display);
        trajectory_path_point(point_right, DynamicVert->world[4 + 6], DynamicVert->world[4 + 7], data.current_cam, display);
        DynamicVert->vert[(*num) * 3 + 0] = pointlast[0];
        DynamicVert->vert[(*num) * 3 + 1] = pointlast[1];
        DynamicVert->vert[(*num) * 3 + 2] = pointlast[2];// + (lineWidth * director);
        DynamicVert->vert[(*num) * 3 + 3] = point_left[0];
        DynamicVert->vert[(*num) * 3 + 4] = point_left[1];
        DynamicVert->vert[(*num) * 3 + 5] = point_left[2];
        *num += 2;
        DynamicVert->vert[(*num) * 3 + 0] = point[0];
        DynamicVert->vert[(*num) * 3 + 1] = point[1];
        DynamicVert->vert[(*num) * 3 + 2] = point[2];// + (lineWidth * director);
        DynamicVert->vert[(*num) * 3 + 3] = point_right[0];
        DynamicVert->vert[(*num) * 3 + 4] = point_right[1];
        DynamicVert->vert[(*num) * 3 + 5] = point_right[2];
        *num += 2;
    }
}
        

void GenerateOtherLineVert(line_vert_t* DynamicVert, int total, DYNAMIC_PACKAGE data, DISPLAY_PGL* display)
{
    int num = DynamicVert->indexNum;
    int div = PARAM_DYNAMIC_LINE_MAX_NUM;
    float strip_x_length = (display->view.type == FISHEYE_DISTORTION || display->view.type == FISHEYE_STATIC_DISTORTION) ? CalculateStripLength(DynamicVert->world[2], DynamicVert->world[0], div):0;
    float strip_y_length = (display->view.type == FISHEYE_DISTORTION || display->view.type == FISHEYE_STATIC_DISTORTION) ? CalculateStripLength(DynamicVert->world[3], DynamicVert->world[1], div):0;

    //line_buffer_allocate(&display->buffer.DynamicLineBuffer, (num * (PARAM_DYNAMIC_LINE_MAX_NUM + 20) * 3 * 4), 1);
    {
        float point[3];
        trajectory_path_point(point, DynamicVert->world[0], DynamicVert->world[1], data.current_cam, display);
        {
            display->circle_patch.Point[0].X = point[0];
            display->circle_patch.Point[0].Y = point[1];
            if(display->view.type == AVM_3D_VIEW || display->view.type == AVM_2D_VIEW ) display->circle_patch.Point[0].Y = point[2];
        }
        trajectory_path_point(point, DynamicVert->world[2], DynamicVert->world[3], data.current_cam, display);
        {
            display->circle_patch.Point[1].X = point[0];
            display->circle_patch.Point[1].Y = point[1];
            if (display->view.type == AVM_3D_VIEW|| display->view.type == AVM_2D_VIEW) display->circle_patch.Point[1].Y = point[2];
        }
    }

    switch (total)
    {
        case ORIG_binnary:
            ProcessForOrigBinary(DynamicVert, data, div, strip_x_length, strip_y_length, &num, display);
            break;
        case TableLeft_ORIGRight:
            ProcessForTableLeftOrigRight(DynamicVert,  data, div, strip_x_length, strip_y_length, &num, display);
            break;
        case ORIGLeft_TABLERight:
            ProcessForOrigLeftTableRight(DynamicVert,  data, div, strip_x_length, strip_y_length, &num, display);
            break;
        case TABLELeft_TABLERight:
            ProcessForTableLeftTableRight(DynamicVert,  data, div, strip_x_length, strip_y_length, &num, display);
            break;
    }
    DynamicVert->indexNum = num;
}

void patch_circle_to_edge_point(DYNAMIC_PACKAGE data, DISPLAY_PGL* display)
{
    int style = data.current_style;
    int current_line = data.current_line;
    float lineWidth = LINEWIDTH_NOR * (float)data.linewidth[style][current_line] / 2.0f;
    float radius = lineWidth / 2.0f;

    line_vert_t* active = &(display->buffer.StylePatch);


    int numSegments = 72;
    float centerX = display->circle_patch.Point[0].X;
    float centerY = display->circle_patch.Point[0].Y;
    float z_axis = 0.1f;
    float ratio = 0.7f;
    int swap_position[3];

    if(Display_pgl.alignment.ON == 0 || autosys.zoom.cur_level > 0)
    {
        switch(display->view.type)
        {
            case AVM_2D_VIEW:
            case AVM_3D_VIEW:
                swap_position[0] = 0;
                swap_position[1] = 1;
                swap_position[2] = -1;
                radius = LINEWIDTH_NOR * (float)data.linewidth[style][current_line] /2.0f ;
                //z_axis = (display->view.director == vertical) ? 0.01f : 0.01f;
                z_axis = (display->view.director == vertical) ? 0.009f : 0.008f;
                ratio = 1.0f;
                break;
            case FISHEYE_DISTORTION:
            case FISHEYE_STATIC_DISTORTION:
                swap_position[0] = 0;
                swap_position[1] = 0;
                swap_position[2] = 0;
                radius = LINEWIDTH_NOR * (float)data.linewidth[style][current_line] / 4.0f;
                //z_axis = (display->view.director == vertical) ? 0.0f : 0.0f;
                z_axis = (display->view.director == vertical) ? 0.2f : 0.3f;
                if(display->view.director == slop)
                {
                    z_axis = (float)data.current_line * 0.01f * 2.0f + 0.01f;
                }
                ratio = (1.0f * (float)g_PANEL_WIDTH / (float)g_PANEL_HEIGHT);
                break;
            case FISHEYE_STATIC_SCENE:
                swap_position[0] = 0;
                swap_position[1] = 0;
                swap_position[2] = 0;
                float gl_width = (float)m_Aligment_data->viewport.W ;
                float LINE_BASIC = 1.0f / gl_width * 10.0f;
                radius = LINE_BASIC * (float)data.linewidth[style][current_line] / 4.0f;
                // if(m_sceneview_CustomlineWidth > 1.0f) radius *= m_sceneview_CustomlineWidth;
                //z_axis = (display->view.director == vertical) ? 0.0f : 0.0f;
                z_axis = (display->view.director == vertical) ? 0.2f : 0.3f;
                if(display->view.director == slop)
                {
                    z_axis = 0.01f;
                }
                ratio = (1.0f * (float)m_Aligment_data->viewport.W / (float)m_Aligment_data->viewport.H);
                break;
            }
    }
    else
    {
        switch(display->view.type)
        {
            case AVM_2D_VIEW:
                swap_position[0] = 0;
                swap_position[1] = 0;
                swap_position[2] = 0;
                radius = (float)display->alignment.Width  / 2.0f / (float)m_Aligment_data->viewport.W;
                z_axis = (display->view.director == vertical) ? 0.2f : 0.3f;
                ratio = (1.0f * (float)m_Aligment_data->viewport.W / (float)m_Aligment_data->viewport.H);
                break;
        }
    }
    float inverse = 0.0f;
    switch (data.current_cam)
    {
        case 0:
            if (display->g_curTireAngle < 0.0f) inverse = 1.0f;
            break;
        case 1:
            if (display->g_curTireAngle >= 0.0f) inverse = 0.0f;
            else inverse = 1.0f;
            break;
        case 2:
        case 3:
            inverse = 0.0f;
            break;
    }
    float pi = 3.141592653589793238f;
    //line_buffer_allocate(active, (numSegments + 5) * 3 * 3 * 2, 1);
    for (int i = 0; i <= numSegments; i++)
    {
        int ix = active->indexNum;
        
        active->vert[ix * 3 + 0] = centerX;
        active->vert[ix * 3 + 1 + swap_position[1]] = centerY;
        active->vert[ix * 3 + 2 + swap_position[2]] = z_axis;
        active->texture[ix * 2 + 0] = fabs(inverse - 1.00f);
        active->texture[ix * 2 + 1] = fabs(inverse - 0.99f);
        float angle1 = (float)i * (pi* 2.0f / (float)numSegments);
        float x1 = centerX + radius * cosf(angle1);
        float y1 = centerY + ratio *radius * sinf(angle1);
       
        float angle2 = (float)(i + 1) * (pi * 2.0f / (float)numSegments);
        float x2 = centerX + radius * cosf(angle2);
        float y2 = centerY + ratio *radius * sinf(angle2);
       
        active->vert[ix * 3 + 3] = x1;
        active->vert[ix * 3 + 4 + swap_position[1]] = y1;
        active->vert[ix * 3 + 5 + swap_position[2]] = z_axis;
        active->vert[ix * 3 + 6] = x2;
        active->vert[ix * 3 + 7 + swap_position[1]] = y2;
        active->vert[ix * 3 + 8 + swap_position[2]] = z_axis;
        active->texture[ix * 2 + 2] = fabs(inverse - 0.0f);
        active->texture[ix * 2 + 3] = fabs(inverse - 0.99f);
        active->texture[ix * 2 + 4] = fabs(inverse - 0.00f);
        active->texture[ix * 2 + 5] = fabs(inverse - 0.99f);
        active->indexNum += 3;
    }

    centerX = display->circle_patch.Point[1].X;
    centerY = display->circle_patch.Point[1].Y;

    for (int i = 0; i <= numSegments; i++)
    {
        int ix = active->indexNum;
       
        active->vert[ix * 3 + 0] = centerX;
        active->vert[ix * 3 + 1 + swap_position[1]] = centerY;
        active->vert[ix * 3 + 2 + swap_position[2]] = z_axis;
        active->texture[ix * 2 + 0] = fabs(inverse - 1.0f);
        active->texture[ix * 2 + 1] = fabs(inverse - 0.01f);
        float angle1 = (float)i * (pi * 2.0f / (float)numSegments);
        float x1 = centerX + radius * cosf(angle1);
        float y1 = centerY + ratio * radius * sinf(angle1);
        float angle2 = (float)(i + 1) * (pi * 2.0f / (float)numSegments);
        float x2 = centerX + radius * cosf(angle2);
        float y2 = centerY + ratio * radius * sinf(angle2);
        
        active->vert[ix * 3 + 3] = x1;
        active->vert[ix * 3 + 4 + swap_position[1]] = y1;
        active->vert[ix * 3 + 5 + swap_position[2]] = z_axis;
        active->vert[ix * 3 + 6] = x2;
        active->vert[ix * 3 + 7 + swap_position[1]] = y2;
        active->vert[ix * 3 + 8 + swap_position[2]] = z_axis;
        active->texture[ix * 2 + 2] = fabs(inverse - 0.00f);
        active->texture[ix * 2 + 3] = fabs(inverse - 0.02f);
        active->texture[ix * 2 + 4] = fabs(inverse - 0.00f);
        active->texture[ix * 2 + 5] = fabs(inverse - 0.02f);
        active->indexNum += 3;
    }

    display->buffer.StylePatch.indexNum = active->indexNum;
}

void Generate_circle_style_line_vert(line_vert_t* DynamicVert, DYNAMIC_PACKAGE data, int* indexNum, DISPLAY_PGL* display)
{
    float Angle = 90.0f;
    int total = DynamicVert->indexNum;
    DynamicVert->indexNum = *indexNum;
    int ix;
    if (display->view.director == vertical)
    {
        // line_buffer_allocate(DynamicVert, (DynamicVert->indexNum * 3 * 4) + (total * (3 * 4)), 1);
        for (ix = 0; ix < total; ix++)
        {
            int index = ix * 2;
            if (ix >= total - 1)
            {
                display->rect_patch.g_finaly_draw_point = 1;
            }
            set_trajectory_path_texture_line(DynamicVert->texture, DynamicVert->indexNum, DynamicVert->textmap, display->g_curTireAngle, index, data, display);
            set_trajectory_path_point_line(DynamicVert->vert, &DynamicVert->indexNum, DynamicVert->world, &Angle, index, data, display);
            display->rect_patch.g_finaly_draw_point = 0 ;
        }
        patch_circle_to_edge_point(data, display);
        *indexNum = DynamicVert->indexNum;
    }
    else if(display->view.director == slop)
    {
        // line_buffer_allocate(DynamicVert, (DynamicVert->indexNum * 3 * 4) + ( (total + 2) * (3 * 4)), 1);
        for (ix = 0; ix < total; ix++)
        {
            int index = ix * 2;
            if (ix >= total - 1)
            {
                display->rect_patch.g_finaly_draw_point = 1;
            }
            set_trajectory_path_point_line(DynamicVert->vert, &DynamicVert->indexNum, DynamicVert->world, &Angle, index, data, display);
            display->rect_patch.g_finaly_draw_point = 0 ;
        }
        patch_circle_to_edge_point(data, display);
        *indexNum = DynamicVert->indexNum;
    }
    else
    {
        GenerateOtherLineVert(DynamicVert, total, data, display);
        patch_circle_to_edge_point(data, display);
        *indexNum = DynamicVert->indexNum;
    }
}

void Generate_rectangle_style_line_vert(line_vert_t* DynamicVert, DYNAMIC_PACKAGE data, int* indexNum, DISPLAY_PGL* display)
{
    float Angle = 90.0f;
    int total = DynamicVert->indexNum;
    DynamicVert->indexNum = *indexNum;
    int ix;
    if (display->view.director == vertical)
    {
        for (ix = 0; ix < total; ix++)
        {
            int index = ix * 2;
            set_trajectory_path_texture_line(DynamicVert->texture, &DynamicVert->indexNum, DynamicVert->textmap, Angle, index, data, display);
            set_trajectory_path_point_line(DynamicVert->vert, &DynamicVert->indexNum, DynamicVert->world, &Angle, index, data, display);
            display->rect_patch.g_finaly_draw_point = 0;
        }
        *indexNum = DynamicVert->indexNum;
    }
    else
    {
        GenerateOtherLineVert(DynamicVert, total, data, display);
        *indexNum = DynamicVert->indexNum;
    }
}

void set_trajectory_triangle_line(line_vert_t*  DynamicVert, int* indexNum, DYNAMIC_PACKAGE data, int type, DISPLAY_PGL* display)
{
    switch (display->view.drawstyle)
    {
        case rectangle:
            Generate_rectangle_style_line_vert(DynamicVert, data, indexNum, display);
            break;
        case circle:
            Generate_circle_style_line_vert(DynamicVert, data, indexNum, display);
            break;
    }
}

/*==============================================================
*                            TOP2D aligment Operate
===============================================================*/
void setLineAlignment(int enable, SET_CUR_VIEWDATA* data)
{
    if(enable == 1 && data != NULL)
    {
        m_Aligment_data = data;
    }
}
/*==============================================================
*                            TOP2D aligment Operate
===============================================================*/
void setSceneViewCustomLineWidth(float width)
{
    m_sceneview_CustomlineWidth = width;
}

// VECF4 VECF4_Transform(VECF4 A, float* B)
// {
//     VECF4 opt;
//     opt.X = B[0] * A.X + B[4] * A.Y + B[8] * A.Z + B[12] * A.W;
//     opt.Y = B[1] * A.X + B[5] * A.Y + B[9] * A.Z + B[13] * A.W;
//     //opt.Z = ;
//     //opt.W = ;
//     return opt;
// }

void TOP2D_Init_buffer(float ***PreVert, Point***IntRecord, PointF*** GLVert)
{
    if((*PreVert) == NULL)
    {
        (*PreVert) = (float**)calloc(30, sizeof(float*));
        for(int ix = 0; ix < 30; ix++)
        {
            (*PreVert)[ix] = (float*)calloc(20, sizeof(float));
        }
    }
    if((*IntRecord) == NULL)
    {
        (*IntRecord) = (Point**)calloc(PARAM_AVM_DRAW_LINE_MAX, sizeof(Point*));
        for(int ix = 0; ix < PARAM_AVM_DRAW_LINE_MAX; ix++)
        {
            (*IntRecord)[ix] = (Point*)calloc(20, sizeof(Point));
        }
    }
    if((*GLVert) == NULL)
    {
        (*GLVert) = (PointF**)malloc(PARAM_AVM_DRAW_LINE_MAX * sizeof(PointF*));
        for (int i = 0; i < PARAM_AVM_DRAW_LINE_MAX; i++) 
        {
            (*GLVert)[i] = (PointF*)malloc(4 * sizeof(PointF));
        }
    }
}

void TOP2D_Free_buffer(float ***PreVert, Point***IntRecord)
{
    for (int i = 0; i < 30; i++) 
    {
        free((*PreVert)[i]);
    }
    free((*PreVert));

    for (int i = 0; i < PARAM_AVM_DRAW_LINE_MAX; i++) 
    {
        free((*IntRecord)[i]);
    }
    free((*IntRecord));
}

void NPixelPoint2GLPoint(Point src, PointF *dst, int imgH, int imgW)
{
    float pixelX = (float)src.X;
    float pixelY = (float)src.Y;

    float norx = (float)pixelX / (float)imgW * 2.0f - 1.0f;
    float nory = 1.0f - (float)pixelY / (float)imgH * 2.0f;

    dst->X = norx;
    dst->Y = nory;
}

void GLPoint2PixelPoint(PointF src, Point *dst, int imgH, int imgW)
{
    float GLX = src.X;
    float GLY = src.Y;

    float norx = (GLX + 1.0f) * (float)imgW * 0.5f;
    float nory = (GLY * -1.0f + 1.0f) * (float)imgH * 0.5f;

    dst->X = (int)(norx + 0.5f);
    dst->Y = (int)(nory + 0.5f);
}

void TOP2D_Pre_Generate_3D_glvert(float Top2Ddata[PARAM_AVM_DRAW_LINE_MAX][4], int Linenum, float ***output, DYNAMIC_PACKAGE data, DISPLAY_PGL *display, DIRETOR *linetype)
{
    int indexnum = 0;
    float angle = 90.0f;
    for (int ix = 0; ix < Linenum; ix++)
    {
        display->view.director = linetype[ix];
        data.current_line = ix;
        int index =0;
        float point[3];
        if(linetype[ix] == vertical)
        {
            trajectory_path_point(point, Top2Ddata[ix][index + 0], Top2Ddata[ix][index + 1], data.current_cam, display);
            trajectory_line_width((*output)[ix], angle, point, (indexnum) * 3, data, display);
            indexnum += 2;
            trajectory_path_point(point, Top2Ddata[ix][index + 2], Top2Ddata[ix][index + 3], data.current_cam, display);
            trajectory_line_width((*output)[ix], angle, point, (indexnum) * 3, data, display);
            indexnum = 0;
        }
        else
        {
            trajectory_path_point(point, Top2Ddata[ix][index + 0], Top2Ddata[ix][index + 1], data.current_cam, display);
            trajectory_line_horizon_width((*output)[ix], 0.0f, point, (indexnum) * 3, data, display);
            indexnum += 2;
            trajectory_path_point(point, Top2Ddata[ix][index + 2], Top2Ddata[ix][index + 3], data.current_cam, display);
            trajectory_line_horizon_width((*output)[ix], 0.0f, point, (indexnum) * 3, data, display);
            indexnum = 0;
        }
    }
}

void TOP2D_3D_glvert_TO_2Dplane(int Linenum, float ***Glvert, PointF ***output)
{
    /*VERT TO PORJECTION*/
    PointF PixelRecord[PARAM_AVM_DRAW_LINE_MAX][20];
    for (int ix = 0; ix < Linenum; ix++)
    {
        float* cur_line = (*Glvert)[ix];
        int line_count = 0;
        for(int index = 0; index < 4; index++)
        {
            VECF4 glvert = 
            {
                .X = cur_line[line_count++],
                .Y = cur_line[line_count++],
                .Z = cur_line[line_count++],
                .W = 1.0f,
            };
            VECF4 ProjectPlane = VECF4_Transform(glvert, autosys_get_zoomin2Dprojectionmatrix());
            //VECF4 ProjectPlane = VECF4_Transform(glvert, para2Davm.Rotate);
            PixelRecord[ix][index].X = ProjectPlane.X;
            PixelRecord[ix][index].Y = ProjectPlane.Y;
        }
    }

    // for(int ix = 0; ix < 4; ix++)
    // {
    //     for(int j = 0; j < 4; j++)
    //     {
    //         printf("%f, ", para2Davm.Rotate[ix*4 + j]);
    //     }
    //     printf("\n");
    // }

    //printf("PixelRecord[2][0].X = %f, PixelRecord[2][0].Y = %f\n", PixelRecord[2][0].X , PixelRecord[2][0].Y);

    /*PROJECTION TO PIXEL DOMAIN*/
    //Point IntRecord[PARAM_AVM_DRAW_LINE_MAX][20];
    for (int ix = 0; ix < Linenum; ix++)
    {
        int width  = m_Aligment_data->viewport.W;
        int height = m_Aligment_data->viewport.H;
        for (int index = 0; index < 4; index++)
        {
            PointF glpoint =
            {
                PixelRecord[ix][index].X, 
                PixelRecord[ix][index].Y,
            };
            GLPoint2PixelPoint(glpoint, &(*output)[ix][index], height, width);
        }
    }
}

void TOP2D_Alignment_kernel(int Linenum, Point*** IntRecord, DYNAMIC_PACKAGE data, DISPLAY_PGL *display, DIRETOR *linetype, int style) 
{
    #if 1
    AVMQUENE quene[PARAM_AVM_DRAW_LINE_MAX];
    for (int ix = 0; ix < 10; ix++)
    {
        quene[ix].data = (int*)calloc(Linenum, sizeof(int));
        quene[ix].used = 0;
    }

    for(int ix = 0; ix < Linenum; ix++)
    {
        int locat = (int)data.linewidth[style][ix];
        int curIdx = quene[locat].used;
        quene[locat].data[curIdx] = ix;
        quene[locat].used++;
    }

    for (int ix = 0; ix < 10; ix++)
    {
        int aligmentbuffer[10] = {0};
        if(quene[ix].used != 0)
        {
            for(int sel_index = 0; sel_index < quene[ix].used; sel_index++)
            {
                int candi = 0;
                int vote = 0;
                int sel_num = quene[ix].data[sel_index];
                switch (linetype[sel_num])
                {
                    case horizontal:
                        vote = abs((*IntRecord)[sel_num][0].Y - (*IntRecord)[sel_num][1].Y);
                        candi = (vote > 10) ? 9 : vote;
                        break;
                    case vertical:
                        vote = abs((*IntRecord)[sel_num][1].X - (*IntRecord)[sel_num][0].X);
                        candi = (vote > 10) ? 9 : vote;
                        break;
                }
                aligmentbuffer[candi]++;
            }

            int aligmentLength = 0;
            int max_vote = 0;
            int mode = 0;
            for(int ix = 0; ix < 10; ix++)
            {
                if(max_vote < aligmentbuffer[ix])
                {
                    aligmentLength = mode;
                }
                mode++;
            }
            /*Aligment to width*/
        if (aligmentLength != 0)
        {
            display->alignment.Width = aligmentLength;
            for (int sel_index = 0; sel_index < quene[ix].used; sel_index++)
            {
                int sel_num = quene[ix].data[sel_index];
                int error = 0;
                int abs_hoandi = 0;
                int abs_veandi = 0;
                switch (linetype[sel_num])
                {
                    case horizontal:
                        abs_hoandi = abs((*IntRecord)[sel_num][0].Y - (*IntRecord)[sel_num][1].Y);
                        if(abs_hoandi < aligmentLength)
                        {
                            error = aligmentLength - abs_hoandi;
                            (*IntRecord)[sel_num][1].Y += error;
                            (*IntRecord)[sel_num][3].Y += error;
                        }
                        else if(abs_hoandi > aligmentLength)
                        {
                            error = aligmentLength - abs_hoandi;
                            (*IntRecord)[sel_num][1].Y += error;
                            (*IntRecord)[sel_num][3].Y += error;
                        }
                        break;
                    case vertical:
                        abs_veandi = abs((*IntRecord)[sel_num][1].X - (*IntRecord)[sel_num][0].X);
                        if (abs_veandi < aligmentLength)
                        {
                            error = aligmentLength - abs_veandi;
                            (*IntRecord)[sel_num][1].X += error;
                            (*IntRecord)[sel_num][3].X += error;
                        }
                        else if (abs_veandi > aligmentLength)
                        {
                            error = aligmentLength - abs_veandi;
                            (*IntRecord)[sel_num][1].X += error;
                            (*IntRecord)[sel_num][3].X += error;
                        }
                        break;
                    }
                }
            }
        }
    }
    for (int ix = 0; ix < 10; ix++)
    {
        free(quene[ix].data);
    }
    #endif
    #if 0
    int aligmentbuffer[10] = {0};
    for (int ix = 0; ix < Linenum; ix++)
    {
        int candi = 0;
        int vote = 0;
        switch (linetype[ix])
        {
            case horizontal:
                vote = abs((*IntRecord)[ix][0].Y - (*IntRecord)[ix][1].Y);
                candi = (vote > 10) ? 9 : vote;
                break;
            case vertical:
                vote = abs((*IntRecord)[ix][1].X - (*IntRecord)[ix][0].X);
                candi = (vote > 10) ? 9 : vote;
                break;
        }
        aligmentbuffer[candi]++;
    }
    int aligmentLength = 0;
    int max_vote = 0;
    int mode = 0;
    for(int ix = 0; ix < 10; ix++)
    {
        if(max_vote < aligmentbuffer[ix])
        {
            aligmentLength = mode;
        }
        mode++;
    }
    /*Aligment to width*/
    if (aligmentLength != 0)
    {
        display->alignment.Width = aligmentLength;
        for (int ix = 0; ix < Linenum; ix++)
        {
            int error = 0;
            int abs_hoandi = 0;
            int abs_veandi = 0;
            switch (linetype[ix])
            {
                case horizontal:
                    abs_hoandi = abs((*IntRecord)[ix][0].Y - (*IntRecord)[ix][1].Y);
                    if(abs_hoandi < aligmentLength)
                    {
                        error = aligmentLength - abs_hoandi;
                        (*IntRecord)[ix][1].Y += error;
                        (*IntRecord)[ix][3].Y += error;
                    }
                    else if(abs_hoandi > aligmentLength)
                    {
                        error = aligmentLength - abs_hoandi;
                        (*IntRecord)[ix][1].Y += error;
                        (*IntRecord)[ix][3].Y += error;
                    }
                    break;
                case vertical:
                    abs_veandi = abs((*IntRecord)[ix][1].X - (*IntRecord)[ix][0].X);
                    if (abs_veandi < aligmentLength)
                    {
                        error = aligmentLength - abs_veandi;
                        (*IntRecord)[ix][1].X += error;
                        (*IntRecord)[ix][3].X += error;
                    }
                    else if (abs_veandi > aligmentLength)
                    {
                        error = aligmentLength - abs_veandi;
                        (*IntRecord)[ix][1].X += error;
                        (*IntRecord)[ix][3].X += error;
                    }
                    break;
            }
        }
    }
    #endif
    
}

void TOP2D_2Dplane_TO_2D_glvert(int Linenum, Point ***Int2DPlane, PointF ***GLVert)
{
    for (int ix = 0; ix < Linenum; ix++)
    {
        int width  = m_Aligment_data->viewport.W;
        int height = m_Aligment_data->viewport.H;
        for (int index = 0; index < 4; index++)
        {
            NPixelPoint2GLPoint((*Int2DPlane)[ix][index], &(*GLVert)[ix][index], height, width);
        }
    }
}

PointF** TOP2D_Transfer_To_Pixeldomain(float Top2Ddata[PARAM_AVM_DRAW_LINE_MAX][4], int Linenum, DYNAMIC_PACKAGE data, DISPLAY_PGL *display, DIRETOR *linetype, int style)
{
    float** output = NULL;
    Point** IntRecord = NULL;
    PointF** GLVert = NULL;
    TOP2D_Init_buffer(&output, &IntRecord, &GLVert);
    TOP2D_Pre_Generate_3D_glvert(Top2Ddata, Linenum, &output, data, display, linetype);
    TOP2D_3D_glvert_TO_2Dplane(Linenum, &output, &IntRecord);
    TOP2D_Alignment_kernel(Linenum, &IntRecord, data, display, linetype, style);
    TOP2D_2Dplane_TO_2D_glvert(Linenum, &IntRecord, &GLVert);
    TOP2D_Free_buffer(&output, &IntRecord);
    return GLVert;
}

void TOP2D_Locate_to_active_buffer(PointF** Top2d, line_vert_t*  DynamicVert, int *indexNum, DYNAMIC_PACKAGE data, DISPLAY_PGL* display)
{
    int cur_line_style = data.current_style;
    int line_num = data.lineNumber[cur_line_style];
    // line_buffer_allocate(&display->buffer.DynamicLineBuffer, (line_num * 3 * 4), 1);
    DynamicVert->indexNum = 0;
    for (int ix = 0; ix < line_num; ix++)
    {
        int start_count = (DynamicVert->indexNum) * 3;
        //for (int index = 0; index < 4; index++)
        {
            DynamicVert->vert[start_count + 0] = Top2d[ix][0].X;
            DynamicVert->vert[start_count + 1] = Top2d[ix][0].Y;
            DynamicVert->vert[start_count + 2] = 0.01f;

            DynamicVert->vert[start_count + 3] = Top2d[ix][1].X;
            DynamicVert->vert[start_count + 4] = Top2d[ix][1].Y;
            DynamicVert->vert[start_count + 5] = 0.01f;

            DynamicVert->vert[start_count + 6] = Top2d[ix][2].X;
            DynamicVert->vert[start_count + 7] = Top2d[ix][2].Y;
            DynamicVert->vert[start_count + 8] = 0.01f;

            DynamicVert->vert[start_count + 9]  = Top2d[ix][3].X;
            DynamicVert->vert[start_count + 10] = Top2d[ix][3].Y;
            DynamicVert->vert[start_count + 11] = 0.01f;
        }
        DynamicVert->indexNum += 4;
        data.line_strip[cur_line_style][ix] = DynamicVert->indexNum;

        display->circle_patch.Point[0].X = (Top2d[ix][1].X + Top2d[ix][0].X) / 2.0f;
        display->circle_patch.Point[0].Y = (Top2d[ix][1].Y + Top2d[ix][0].Y) / 2.0f;
        display->circle_patch.Point[1].X = (Top2d[ix][2].X + Top2d[ix][3].X) / 2.0f;
        display->circle_patch.Point[1].Y = (Top2d[ix][2].Y + Top2d[ix][3].Y) / 2.0f;
        patch_circle_to_edge_point(data, display);
    }
    

    for (int i = 0; i < PARAM_AVM_DRAW_LINE_MAX; i++) 
    {
        free(Top2d[i]);
    }
    free(Top2d);

}


void cal_avm_domain_pgl_mode(Cluster_calculater_Paramter *parameter)
{
    int cam               = parameter->cam;
    int style             = parameter->style;
    int* linetype         = parameter->linetype;
    float car_angle       = parameter->car_angle;
    double* yaxis         = parameter->y_axis;
    double* xaxis         = parameter->x_axis;
    DISPLAY_PGL* display  = parameter->display;
    line_vert_t* active   = parameter->active;

    int para = (Display_pgl.view.type == AVM_3D_VIEW || Display_pgl.view.type == AVM_2D_VIEW)?1:0;
    DYNAMIC_PACKAGE* pdata = NULL;
    pdata  = (Display_pgl.view.type != FISHEYE_STATIC_DISTORTION)?&dynamicData[para][cam]:&staticData[0][cam];

    int start_linestrip = 0;
    pdata->reference[style].cur_id = 0;
    cal_junctionPoint(cam, pdata->reference[style].map,pdata->map_size[style], &pdata->reference[style], car_angle, yaxis, xaxis, display);
    for (int i = 0; i < pdata->lineNumber[style]; i++)
    {
        int totalPoint = 0;
        if(cam >= 2)
        {
            {
                active->world[0] = (float)xaxis[i * 2 + 0];
                active->world[1] = (float)yaxis[i * 2 + 0];
                active->world[2] = (float)xaxis[i * 2 + 1];
                active->world[3] = (float)yaxis[i * 2 + 1];
                // AVM_LOGI("PixelMode id[%d] line p1 [%f, %f]\n",i,  xaxis[i * 2 + 0], yaxis[i * 2 + 0]);
                // AVM_LOGI("PixelMode id[%d] line p2 [%f, %f]\n",i,  xaxis[i * 2 + 1], yaxis[i * 2 + 1]);
                float div_x = (float)(active->world[2] - active->world[0]) / (float)PARAM_DYNAMIC_LINE_MAX_NUM;
                float div_y = (float)(active->world[3] - active->world[1]) / (float)PARAM_DYNAMIC_LINE_MAX_NUM;
                int index = 0;
                float color_gap = 1.0f / (float)PARAM_DYNAMIC_LINE_MAX_NUM;
                for (int ix = 0; ix < PARAM_DYNAMIC_LINE_MAX_NUM; ix++)
                {
                    active->world[index++] = active->world[0] + (div_x * (float)ix);
                    active->world[index++] = active->world[1] + (div_y * (float)ix);
                    active->textmap[ix]    = 1.0f - color_gap * (float)ix;
                }
                totalPoint = PARAM_DYNAMIC_LINE_MAX_NUM;
            }
        }
        else
        {
            if (linetype[i] == vertical)
            {
                totalPoint = cal_vertical_line(cam, active, car_angle, yaxis, xaxis, i, &pdata->reference[style]);
            }
            else if (linetype[i] == horizontal)
            {
                //AVM_LOGI("PixelMode id[%d] line p1 [%f, %f]\n",i,  xaxis[i * 2 + 0], yaxis[i * 2 + 0]);
                //AVM_LOGI("PixelMode id[%d] line p2 [%f, %f]\n",i,  xaxis[i * 2 + 1], yaxis[i * 2 + 1]);

                // line_buffer_allocate(active, 4, 0);
                cal_horizontal_line(cam, active, car_angle, yaxis, xaxis, i);

                if (Display_pgl.view.drawstyle == rectangle)
                {
                    if (pdata->table[style].ID[i] >= 0)
                    {
                        totalPoint = 0;
                        {
                            int director = 1;
                            for (int side = 0; side < 2; side++)
                            {
                                totalPoint++;
                                int boxindex = pdata->table[style].junctionP[i][side];
                                if (boxindex >= 0)
                                {
                                    float width = pdata->reference[style].ver_width[boxindex];
                                    active->world[4 + (4 * (side) + 0)] = pdata->reference[style].junP1[boxindex].X + (width * director);
                                    active->world[4 + (4 * (side) + 1)] = pdata->reference[style].junP1[boxindex].Y;
                                    active->world[4 + (4 * (side) + 2)] = pdata->reference[style].junP2[boxindex].X + (width * director);
                                    active->world[4 + (4 * (side) + 3)] = pdata->reference[style].junP2[boxindex].Y;
                                    totalPoint += (side + 1);
                                }
                                director *= -1;
                            }
                        }
                    }
                }
                else
                {
                    totalPoint = 2;
                }
            }
        }

        active->indexNum = totalPoint;

        pdata->current_cam = cam;
        pdata->current_style = style;
        pdata->current_line = i;
        if(display->view.type == AVM_2D_VIEW && autosys.zoom.cur_level <= 0 && Display_pgl.alignment.ON == 1)
        {
            float start_x = active->world[0];
            float start_y = active->world[1];
            float end_x   = active->world[totalPoint * 2 - 2];
            float end_y   = active->world[totalPoint * 2 - 1];
            parameter->Top2Ddata[i][0] = start_x;
            parameter->Top2Ddata[i][1] = start_y;
            parameter->Top2Ddata[i][2] = end_x;
            parameter->Top2Ddata[i][3] = end_y;
        }
        else
        {
            if(cam >= 2) 
            {
                display->view.director = vertical;
            }
            else 
            {
                display->view.director = linetype[i];
            }
            set_trajectory_triangle_line(active, &start_linestrip, *pdata, linetype[i], display);
            pdata->line_strip[style][i] = start_linestrip;
        }
    }

}

void cal_pixel_domain_pgl_mode(Cluster_calculater_Paramter *parameter)
{
    int cam              = parameter->cam;
    int style            = parameter->style;
    int* linetype        = parameter->linetype;
    float car_angle      = parameter->car_angle;
    double* yaxis         = parameter->y_axis;
    double* xaxis         = parameter->x_axis;
    DISPLAY_PGL* display = parameter->display;
    line_vert_t* active  = parameter->active;
    int para = (Display_pgl.view.type == AVM_3D_VIEW || Display_pgl.view.type == AVM_2D_VIEW)?1:0;
    DYNAMIC_PACKAGE* pdata = NULL;
    pdata  = (Display_pgl.view.type != FISHEYE_STATIC_SCENE) ? &staticData[0][cam]:&staticData[1][cam];
    int start_linestrip = 0;
    // AVM_LOGI("PixelMode line number [%d]\n", pdata->lineNumber[style]);
    // AVM_LOGI("PixelMode line p1 [%f, %f]\n", xaxis[0], yaxis[0]);
    // AVM_LOGI("PixelMode line p2 [%f, %f]\n", xaxis[1], yaxis[1]);
    // AVM_LOGI("PixelMode line w  [%f]\n", pdata->linewidth[style][0]);
    // AVM_LOGI("PixelMode line c  [%f,%f,%f,%f]\n", pdata->gl_colormap[style][0],pdata->gl_colormap[style][1],
    //                                               pdata->gl_colormap[style][2],pdata->gl_colormap[style][3]);


    // line_buffer_allocate(active, (PARAM_DYNAMIC_LINE_MAX_NUM + 2) * 2, 0);

    for (int i = 0; i < pdata->lineNumber[style]; i++)
    {
        int totalPoint = 0;
        if(Display_pgl.view.type == FISHEYE_STATIC_DISTORTION ||
           Display_pgl.view.type == FISHEYE_STATIC_SCENE)
        {
            {
                active->world[0] = xaxis[i*2 + 0];
                active->world[1] = yaxis[i*2 + 0];
                active->world[2] = xaxis[i*2 + 1];
                active->world[3] = yaxis[i*2 + 1];
                float div_x = (float)(active->world[2] - active->world[0]) / (float)PARAM_DYNAMIC_LINE_MAX_NUM;
                float div_y = (float)(active->world[3] - active->world[1]) / (float)PARAM_DYNAMIC_LINE_MAX_NUM;
                int index = 0;
                for(int ix = 0; ix < PARAM_DYNAMIC_LINE_MAX_NUM; ix++)
                {
                    active->world[index++] = active->world[0] + (div_x * (float)ix);
                    active->world[index++] = active->world[1] + (div_y * (float)ix);
                }
                totalPoint = PARAM_DYNAMIC_LINE_MAX_NUM;
            }
        }

        active->indexNum = totalPoint;

        pdata->current_cam = cam;
        pdata->current_style = style;
        pdata->current_line = i;

        if(display->view.type == FISHEYE_STATIC_DISTORTION ||
           display->view.type == FISHEYE_STATIC_SCENE)
        {
            display->view.director = slop;
            set_trajectory_triangle_line(active, &start_linestrip, *pdata, linetype[i], display);
            pdata->line_strip[style][i] = start_linestrip;
        }
    }
}

void assign_worldpoint_to_buffer(double* xbuffer,double* ybuffer, int cam, int view, int style, float car_long_half, ViewType type, DYNAMIC_PACKAGE* pdata)
{
    int* world;
    float car_width_half = autosys.system_info.CarSize.X / 2.0f;
    switch (cam + (int)type)
    {
        case (int)CAM_NUM_FRONT + (int)FISHEYE_DISTORTION:
        case (int)CAM_NUM_FRONT + (int)AVM_3D_VIEW:
        case (int)CAM_NUM_FRONT + (int)AVM_2D_VIEW:
            {
                world = pdata->worldcoordinate[style];
                for (int ix = 0; ix < pdata->lineNumber[style]; ix++)
                {
                    int index = ix * 4;
                    int count = ix * 2;
    
                    xbuffer[count + 0] = world[index + 0];
                    xbuffer[count + 1] = world[index + 2];
                    ybuffer[count + 0] = world[index + 1] + car_long_half;
                    ybuffer[count + 1] = world[index + 3] + car_long_half;
                }
            }
            break;
        case (int)CAM_NUM_BACK + (int)FISHEYE_DISTORTION:
        case (int)CAM_NUM_BACK + (int)AVM_3D_VIEW:
        case (int)CAM_NUM_BACK + (int)AVM_2D_VIEW:
        {
            world = pdata->worldcoordinate[style];
            for (int ix = 0; ix < pdata->lineNumber[style]; ix++)
            {
                int index = ix * 4;
                int count = ix * 2;
                xbuffer[count + 0] = world[index + 0];
                xbuffer[count + 1] = world[index + 2];
                ybuffer[count + 0] = world[index + 1] - car_long_half;
                ybuffer[count + 1] = world[index + 3] - car_long_half;
                //AVM_LOGI("worldcoordinate idx %d: [%f, %f, %f, %f]\n", ix, world[index + 0], world[index + 1], world[index + 2], world[index + 3]);
            }
        }
            break;
        case (int)CAM_NUM_LEFT + (int)FISHEYE_DISTORTION:
        case (int)CAM_NUM_LEFT + (int)AVM_3D_VIEW:
        case (int)CAM_NUM_LEFT + (int)AVM_2D_VIEW:
            {
            world = pdata->worldcoordinate[style];
            for (int ix = 0; ix < pdata->lineNumber[style]; ix++)
            {
                int index = ix * 4;
                int count = ix * 2;
                xbuffer[count + 0] = world[index + 0] - car_width_half;
                xbuffer[count + 1] = world[index + 2] - car_width_half;
                ybuffer[count + 0] = world[index + 1];
                ybuffer[count + 1] = world[index + 3];
            }
            }
            break;
        case (int)CAM_NUM_RIGHT + (int)FISHEYE_DISTORTION:
        case (int)CAM_NUM_RIGHT + (int)AVM_3D_VIEW:
        case (int)CAM_NUM_RIGHT + (int)AVM_2D_VIEW:
            {
            world = pdata->worldcoordinate[style];
            for (int ix = 0; ix < pdata->lineNumber[style]; ix++)
            {
                int index = ix * 4;
                int count = ix * 2;
                xbuffer[count + 0] = world[index + 0] + car_width_half;
                xbuffer[count + 1] = world[index + 2] + car_width_half;
                ybuffer[count + 0] = world[index + 1];
                ybuffer[count + 1] = world[index + 3];
            }
            }
            break;
        case (int)CAM_NUM_FRONT + (int)FISHEYE_STATIC_DISTORTION:
        case (int)CAM_NUM_BACK  + (int)FISHEYE_STATIC_DISTORTION:
        case (int)CAM_NUM_LEFT  + (int)FISHEYE_STATIC_DISTORTION:
        case (int)CAM_NUM_RIGHT  + (int)FISHEYE_STATIC_DISTORTION:
            {
                world = pdata->worldcoordinate[style];
                for (int ix = 0; ix < pdata->lineNumber[style]; ix++)
                {

                    int index = ix * 4;
                    int count = ix * 2;
                    xbuffer[count + 0] = (double)world[index + 0];
                    xbuffer[count + 1] = (double)world[index + 2];
                    ybuffer[count + 0] = (double)world[index + 1];
                    ybuffer[count + 1] = (double)world[index + 3];
                }
            }
            break;
        case (int)CAM_NUM_FRONT + (int)FISHEYE_STATIC_SCENE:
        case (int)CAM_NUM_BACK  + (int)FISHEYE_STATIC_SCENE:
        case (int)CAM_NUM_LEFT  + (int)FISHEYE_STATIC_SCENE:
        case (int)CAM_NUM_RIGHT  + (int)FISHEYE_STATIC_SCENE:
            {
                //m_sceneview_projection_matrix
                //world = pdata->worldcoordinate[style];
                world = pdata->pixelcoordinate[style];
                for (int ix = 0; ix < pdata->lineNumber[style]; ix++)
                {
                    int index = ix * 4;
                    int count = ix * 2;
#if ENABLE_REPORJUSTION_SCENEVIEW_PGL
                    double startX = (double)world[index + 0];
                    double startY = (double)world[index + 1];
                    double endX = (double)world[index + 2];
                    double endY = (double)world[index + 3];
                    double gl_sX, gl_sY, gl_eX, gl_eY;
                    GL_Normal_Pixel2glpt(startX, startY, &gl_sX, &gl_sY, PARAM_SCENEVIEW_WIDTH, PARAM_SCENEVIEW_HEIGHT);
                    GL_Normal_Pixel2glpt(endX, endY, &gl_eX, &gl_eY, PARAM_SCENEVIEW_WIDTH, PARAM_SCENEVIEW_HEIGHT);
                    VECF4 orPt1 = {gl_sX, gl_sY, 0.1f, 1.0};
                    VECF4 orPt2 = {gl_eX, gl_eY, 0.1f, 1.0};
                    VECF4 trPt1 = {startX, startY, 0.1f, 1.0};
                    VECF4 trPt2 = {endX, endY, 0.1f, 1.0};
                    GL_transform_vec4_by_arr16(orPt1, m_sceneview_projection_matrix, &trPt1, true);
                    GL_transform_vec4_by_arr16(orPt2, m_sceneview_projection_matrix, &trPt2, true);

                    // if (cam == 3)
                    // {
                    //     AVM_LOGI("Matrix : %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
                    //              m_sceneview_projection_matrix[0], m_sceneview_projection_matrix[1], m_sceneview_projection_matrix[2], m_sceneview_projection_matrix[3],
                    //              m_sceneview_projection_matrix[4], m_sceneview_projection_matrix[5], m_sceneview_projection_matrix[6], m_sceneview_projection_matrix[7],
                    //              m_sceneview_projection_matrix[8], m_sceneview_projection_matrix[9], m_sceneview_projection_matrix[10], m_sceneview_projection_matrix[11],
                    //              m_sceneview_projection_matrix[12], m_sceneview_projection_matrix[13], m_sceneview_projection_matrix[14], m_sceneview_projection_matrix[15]);
                    //     AVM_LOGI("P1[%f,%f,%f,%f], P2 [%f,%f,%f,%f]\n",
                    //              orPt1.X, orPt1.Y, orPt2.X, orPt2.Y,
                    //              trPt1.X, trPt1.Y, trPt2.X, trPt2.Y);
                    // }
                    GL_Normal_glpt2Pixel(trPt1.X, trPt1.Y, &xbuffer[count + 0], &ybuffer[count + 0], PARAM_SCENEVIEW_WIDTH, PARAM_SCENEVIEW_HEIGHT);
                    GL_Normal_glpt2Pixel(trPt2.X, trPt2.Y, &xbuffer[count + 1], &ybuffer[count + 1], PARAM_SCENEVIEW_WIDTH, PARAM_SCENEVIEW_HEIGHT);
// if(cam == 3)
// {
//     AVM_LOGI("OrigP1[%f,%f,%f,%f], transfer [%f,%f,%f,%f]\n",
//     startX, startY, endX, endY,
//     xbuffer[count + 0], ybuffer[count + 0], xbuffer[count + 1], ybuffer[count + 1]);
// }
#else
                    xbuffer[count + 0] = (double)world[index + 0];
                    xbuffer[count + 1] = (double)world[index + 2];
                    ybuffer[count + 0] = (double)world[index + 1];
                    ybuffer[count + 1] = (double)world[index + 3];
#endif
        }
    }
    break;
    }
}

void CalcFisheyeMatrices(int cam, float* rotateReslut, cropped_view* cropped, 
float* transferM,float* scaleM, float* transM, int *switchPos, ViewType type)
{
    float start_x = (float)cropped->start_x /(float)imageWidth;
    float start_y = (float)cropped->start_y /(float)imageHeight;
    float end_x = (float)cropped->end_x /(float)imageWidth;
    float end_y = (float)cropped->end_y /(float)imageHeight;
    end_x = (end_x == start_x)?start_x + 0.1f:end_x;
    end_y = (end_y == start_y)?start_y + 0.1f:end_y;
    float sacle = (float)(cropped->shift_z <= 0.0)?1.0f:(float)cropped->shift_z;


	float scaleX = 1.0f / (end_x - start_x);
	float scaleY = 1.0f / (end_y - start_y);
	float normalizedCenterX = (start_x+ end_x)/2.0f;  
	float normalizedCenterY = (start_y+ end_y)/2.0f;  
	float centerX = (0.5f - normalizedCenterX) *2.0f * scaleX;
	float centerY = (0.5f - normalizedCenterY) *2.0f * scaleY;
    float mirror = 1.0f;
    switch(cam)
    {
        case CAM_NUM_BACK:
        centerX *= -1.0f; 
        mirror *= -1.0f;
        case CAM_NUM_FRONT:
        {
            float translationMatrix[16] =
	        {
	        	1, 0, 0, 0,
	        	0, 1, 0, 0,
	        	0, 0, 1, 0,
	        	centerX, -centerY, 0, 1
	        };
	        float scaleMatrix[16] =
	        {
	        	scaleX, 0, 0, 0,
	        	0, scaleY, 0, 0,
	        	0, 0, 1, 0,
	        	0, 0, 0, 1
	        };
            float transfermatrix[16] = 
	        {
	        	1.0f * sacle * mirror, 0.0f, 0.0f, 0.0f,
	        	0.0f, 1.0f * sacle, 0.0f, 0.0f,
	        	0.0f, 0.0f, 1.0f, 0.0f,
	        	0.0f + cropped->shift_x, 0.0f + cropped->shift_y, 0.0f, 1.0f,
	        };
            memcpy(transferM,    transfermatrix, sizeof(float) * 16);
            memcpy(scaleM,       scaleMatrix, sizeof(float) * 16);
            memcpy(transM, translationMatrix, sizeof(float) * 16);
        }
        break;
        case CAM_NUM_LEFT:
        case CAM_NUM_RIGHT:
        {
            float Idenetity[16] =
	        {
	        	1, 0, 0, 0,
	        	0, 1, 0, 0,
	        	0, 0, 1, 0,
	        	0, 0, 0, 1
	        };

            float translationMatrix[16] =
	        {
	        	1, 0, 0, 0,
	        	0, 1, 0, 0,
	        	0, 0, 1, 0,
	        	centerX, -centerY, 0, 1
	        };

            float scaleMatrix[16] =
            {
                scaleX, 0, 0, 0,
                0, scaleY, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
            float transfermatrix[16] = 
            {
                1.0f * sacle, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f * sacle, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f + cropped->shift_x, 0.0f + cropped->shift_y, 0.0f, 1.0f,
            };

            if(type == FISHEYE_STATIC_DISTORTION)
            {
                memcpy(transferM,    transfermatrix, sizeof(float) * 16);
                memcpy(scaleM,       scaleMatrix, sizeof(float) * 16);
                memcpy(transM,       translationMatrix, sizeof(float) * 16);
            }
            else
            {
                *switchPos = 3;
                memcpy(transferM,    Idenetity, sizeof(float) * 16);
                memcpy(scaleM,    Idenetity, sizeof(float) * 16);
                memcpy(transM,    Idenetity, sizeof(float) * 16);
            }
            
        }
        break;
    }
}
int cal_dynamic_location_line(float car_angle, int cam, int style, DISPLAY_PGL* display)
{
    DYNAMIC_PACKAGE* pdata = NULL;
    switch(Display_pgl.view.type)
    {
        case AVM_3D_VIEW:
        case AVM_2D_VIEW:
            pdata = &dynamicData[1][cam];
            break;
        case FISHEYE_DISTORTION:
            pdata = &dynamicData[0][cam];
            break;
        case FISHEYE_STATIC_DISTORTION:
            pdata = &staticData[0][cam];
            break;
        case FISHEYE_STATIC_SCENE:
            pdata = &staticData[1][cam];
            break;
    }
    
    float car_LONG_HALF = (pdata->car_WHEELBASE / 2.0f);
    if (cam == CAM_NUM_FRONT) 
    {
        car_LONG_HALF *= -1.0f;
        car_angle *= -1.0f;
    }

    double car_WHEELTOREAR = pdata->car_WHEELTOREAR;
    double xaxis[100];
    double yaxis[100];
    int linetype[PARAM_AVM_DRAW_LINE_MAX];
    if(resort_dynamic_buffer(pdata, linetype, cam, style) == 0) return 0;
    memcpy(display->circle_patch.linetype[cam], linetype, sizeof(int) * PARAM_AVM_DRAW_LINE_MAX);
    assign_worldpoint_to_buffer(xaxis, yaxis, cam, display->view.type, style, car_LONG_HALF, display->view.type, pdata);
    
    line_vert_t* active = &(display->buffer.DynamicLineBuffer);
    line_vert_t* patch  = &(display->buffer.StylePatch);
    patch->indexNum = 0;
    pdata->line_strip[style][0] = 0;
    display->g_curTireAngle = car_angle;
    Cluster_calculater_Paramter parameter = 
    {
        .cam       = cam,
        .style     = style,
        .linetype  = linetype,
        .car_angle = car_angle,
        .y_axis    = yaxis,
        .x_axis    = xaxis,
        .display   = display,
        .active    = active,
    };

    switch(display->view.type)
    {
        case AVM_3D_VIEW:
        case AVM_2D_VIEW:
        case FISHEYE_DISTORTION:
            cal_avm_domain_pgl_mode(&parameter);
            break;
        case FISHEYE_STATIC_DISTORTION:
        case FISHEYE_STATIC_SCENE:
            cal_pixel_domain_pgl_mode(&parameter);
            break;
    }

    int start_linestrip = 0;
    if(display->view.type == AVM_2D_VIEW && autosys.zoom.cur_level <= 0 && Display_pgl.alignment.ON == 1)
    {
        //if(pdata->lineNumber[style] == 0) return;
        if(pdata->lineNumber[style] == 0) return -1;
        PointF** TOP2D_Vert = TOP2D_Transfer_To_Pixeldomain(parameter.Top2Ddata, pdata->lineNumber[style], *pdata, display, linetype, style);
        TOP2D_Locate_to_active_buffer(TOP2D_Vert, active, &start_linestrip, *pdata, display);
    }
    return 1;
}

void Aligment_PGL_function(float *carAngle, ViewType type)
{
#if PARAM_AVM_TOP2D_ALIGMENT_FUNCTION
    if( (*carAngle <= 0.5f && *carAngle >= -0.5f ) && type == AVM_2D_VIEW)
    {
        *carAngle = 0.01;
        Display_pgl.alignment.ON = 1;
    }
    else{Display_pgl.alignment.ON = 0;}
#else
    Display_pgl.alignment.ON = 0;
#endif
}

unsigned char upload_line_texture_buffer_to_gpu(int cam, int style, ViewType type)
{

    static int avm_3d_gpu_line_buffer_size_bytexture[MAX_CAM_NUM][5] = 
    {
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2}
    };
    static int avm_2d_gpu_line_buffer_size_bytexture[MAX_CAM_NUM][5] = 
    {
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2}
    };
    static int avm_fisheye_gpu_line_buffer_size_bytexture[MAX_CAM_NUM][5] = 
    {
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2},
        {DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2, DRAW_DISTORTION_LINE_MAX * 2}
    };


    static int (*line_buffer_table_bytexture[3])[MAX_CAM_NUM][5] = 
    {
        &avm_3d_gpu_line_buffer_size_bytexture,
        &avm_2d_gpu_line_buffer_size_bytexture,
        &avm_fisheye_gpu_line_buffer_size_bytexture
    };

    char *typename = {"3D line", "2D line", "Fisheye line"};

    int* line_buffer_size_ptr = NULL;

    int data_size = Display_pgl.buffer.DynamicLineBuffer.indexNum * 2;
    int index = 0;
    switch(type)
    {
        case AVM_3D_VIEW:
            line_buffer_size_ptr = &(*line_buffer_table_bytexture[0])[cam][style];
            break;
        case AVM_2D_VIEW:
            line_buffer_size_ptr = &(*line_buffer_table_bytexture[1])[cam][style];
            break;
        break;
        case FISHEYE_DISTORTION:
        case FISHEYE_STATIC_DISTORTION:
        case FISHEYE_STATIC_SCENE:
            line_buffer_size_ptr = &(*line_buffer_table_bytexture[2])[cam][style];
            break;
    }

    //if(line_buffer_size_ptr == NULL) return;
    if(line_buffer_size_ptr == NULL) return -1;

    if(*line_buffer_size_ptr >= data_size)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, data_size * sizeof(float), Display_pgl.buffer.DynamicLineBuffer.texture);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return 0;
    }
    else
    {
        *line_buffer_size_ptr = data_size;
        glBufferData(GL_ARRAY_BUFFER, data_size * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, data_size * sizeof(float), Display_pgl.buffer.DynamicLineBuffer.texture);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        AVM_LOGI("Resize %s Texturebuffer size from %d to %d\n", typename, *line_buffer_size_ptr, data_size);
        return 1;
    }
}

int onccccc = 0;
unsigned char upload_line_buffer_to_gpu(int cam, int style, ViewType type)
{

    static int avm_3d_gpu_line_buffer_size[MAX_CAM_NUM][5] = 
    {
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3}
    };
    static int avm_2d_gpu_line_buffer_size[MAX_CAM_NUM][5] = 
    {
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3}
    };
    static int avm_fisheye_gpu_line_buffer_size[MAX_CAM_NUM][5] = 
    {
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3},
        {DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3, DRAW_DISTORTION_LINE_MAX * 3}
    };


    static int (*line_buffer_table[3])[MAX_CAM_NUM][5] = 
    {
        &avm_3d_gpu_line_buffer_size,
        &avm_2d_gpu_line_buffer_size,
        &avm_fisheye_gpu_line_buffer_size
    };

    char *typename = {"3D line", "2D line", "Fisheye line"};

    int* line_buffer_size_ptr = NULL;

    int data_size = Display_pgl.buffer.DynamicLineBuffer.indexNum * 3;
    int index = 0;
    switch(type)
    {
        case AVM_3D_VIEW:
            line_buffer_size_ptr = &(*line_buffer_table[0])[cam][style];
            break;
        case AVM_2D_VIEW:
            line_buffer_size_ptr = &(*line_buffer_table[1])[cam][style];
            break;
        break;
        case FISHEYE_DISTORTION:
        case FISHEYE_STATIC_DISTORTION:
        case FISHEYE_STATIC_SCENE:
            line_buffer_size_ptr = &(*line_buffer_table[2])[cam][style];
            break;
    }

    //if(line_buffer_size_ptr == NULL) return;
    if(line_buffer_size_ptr == NULL) return -1;

    if(*line_buffer_size_ptr >= data_size)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, data_size * sizeof(float), Display_pgl.buffer.DynamicLineBuffer.vert);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // if(style == 0)
        // {
        //     if (Display_pgl.buffer.DynamicLineBuffer.indexNum >= 1000 && onccccc == 0)
        //     {
        //         onccccc = 1;
        //         FILE *fp = fopen("./line.txt", "w");
        //         for (int ix = 0; ix < Display_pgl.buffer.DynamicLineBuffer.indexNum; ix++)
        //         {
        //             float x = Display_pgl.buffer.DynamicLineBuffer.vert[ix * 3 + 0];
        //             float y = Display_pgl.buffer.DynamicLineBuffer.vert[ix * 3 + 1];
        //             float z = Display_pgl.buffer.DynamicLineBuffer.vert[ix * 3 + 2];

        //             fprintf(fp, "%d, %.6f, %.6f, %.6f\n", ix, x, y, z);
        //         }
        //         fclose(fp);
        //         // for (int ix = 0; ix < Display_pgl.buffer.DynamicLineBuffer.indexNum; ix++)
        //         // {
        //         //     AVM_LOGI("id [%d] line p [%f, %f, %f]\n",
        //         //              ix,
        //         //              Display_pgl.buffer.DynamicLineBuffer.vert[ix * 3 + 0],
        //         //              Display_pgl.buffer.DynamicLineBuffer.vert[ix * 3 + 1],
        //         //              Display_pgl.buffer.DynamicLineBuffer.vert[ix * 3 + 2]);
        //         // }
        //     }
        // }
        return 0;
    }
    else
    {
        *line_buffer_size_ptr = data_size;
        glBufferData(GL_ARRAY_BUFFER, data_size * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, data_size * sizeof(float), Display_pgl.buffer.DynamicLineBuffer.vert);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        AVM_LOGI("Resize %s buffer size from %d to %d\n", typename, *line_buffer_size_ptr, data_size);
        return 1;
    }
}

void draw_line_circle_style(int cam, int style, float* transfor, float* scaleMatrix, float* translationMatrix, PGL_SUPPLY* supply)
{
    int program = glContext_ReverseLine.programID;
    int ix = 0;
    float ptr_color[4];
    int index = -1;
    int strip_start;
    int strip_length;

    int flag_draw_patch = 1;
    float y_limit = -1.0f;
    if (supply != NULL)
    {
        flag_draw_patch = supply->flag_enable_patch[style];
        y_limit         = supply->y_limit[style] ;
        if (Display_pgl.view.type == AVM_3D_VIEW || Display_pgl.view.type == AVM_2D_VIEW)
        {
            flag_draw_patch = supply->flag_enable_patch[style/2];
        }
        //AVM_LOGI("draw_line_circle_style: style = %d, flag_draw_patch = %d, y_limit = %f\n",style, flag_draw_patch, y_limit);
    }

    // int para = Display_pgl.view.type;
    int para = (Display_pgl.view.type == AVM_3D_VIEW || Display_pgl.view.type == AVM_2D_VIEW)?1:0;
    float transforMatrixtmp[16], transforMatrix[16];
    myMultiplyMat4_opengl(transforMatrixtmp, transfor, translationMatrix);
    myMultiplyMat4_opengl(transforMatrix, transforMatrixtmp, scaleMatrix);
    glUniformMatrix4fv(glContext_ReverseLine.attrTransformatiomMatrixLocation, 1, GL_FALSE, transforMatrix);
    glUniformMatrix4fv(glGetUniformLocation(program, "ScopeArea.scaleMatrix"), 1, GL_FALSE, scaleMatrix);
    glUniformMatrix4fv(glGetUniformLocation(program, "ScopeArea.translationMatrix"), 1, GL_FALSE, translationMatrix);

    glUniform1f(glGetUniformLocation(program, "y_limit"), y_limit);
    DYNAMIC_PACKAGE *activeLineInfo;
    if(Display_pgl.view.type == FISHEYE_STATIC_DISTORTION)
    {
        flag_draw_patch = 1;
        activeLineInfo = &staticData[0][cam];
    }
    else if(Display_pgl.view.type == FISHEYE_STATIC_SCENE)
    {
        flag_draw_patch = 1;
        activeLineInfo = &staticData[1][cam];
    }
    else
    {
        activeLineInfo = &dynamicData[para][cam];
    }


    for (ix = 0; ix < activeLineInfo->lineNumber[style]; ix++)
    {

        if (Display_pgl.view.type == AVM_3D_VIEW )
        {
            glBindVertexArray(VAO_3Davm_Line[cam + 0 * MAX_CAM_NUM]);
        }
        else if(Display_pgl.view.type == AVM_2D_VIEW)
        {
            glBindVertexArray(VAO_2Davm_Line[cam  + 0 * MAX_CAM_NUM]);
        }
        else
        {
            glBindVertexArray(VAO_fisheye_Line[cam + 0 * MAX_CAM_NUM]);
        }

        ptr_color[0] = activeLineInfo->gl_colormap[style][++index];
        ptr_color[1] = activeLineInfo->gl_colormap[style][++index];
        ptr_color[2] = activeLineInfo->gl_colormap[style][++index];
        ptr_color[3] = activeLineInfo->gl_colormap[style][++index];
        //AVM_LOGI("color is id[%d], %f %f %f %f\n", ix, ptr_color[0], ptr_color[1], ptr_color[2], ptr_color[3]);
        glUniform4fv(glGetUniformLocation(program, "color"), 1, ptr_color);


        if (ix == 0)
        {
            strip_start = 0;
            strip_length = activeLineInfo->line_strip[style][ix];
        }
        else
        {
            strip_start = activeLineInfo->line_strip[style][ix - 1];
            strip_length = activeLineInfo->line_strip[style][ix] - activeLineInfo->line_strip[style][ix - 1];
        }

        glEnable(GL_POLYGON_OFFSET_FILL);
        if (Display_pgl.circle_patch.linetype[cam][ix] == 1)
        {
            glPolygonOffset(-1.0f, -1.0f);
        }
        else if(Display_pgl.circle_patch.linetype[cam][ix] == 0)
        {
            glUniform1i(glGetUniformLocation(program, "windows"), 0);
            glPolygonOffset(1.0f, 1.0f);
        }
        else if(Display_pgl.circle_patch.linetype[cam][ix] == 2)
        {
            glUniform1i(glGetUniformLocation(program, "windows"), 0);
        }

        glDrawArrays(GL_TRIANGLE_STRIP, strip_start, strip_length);

        if(Display_pgl.circle_patch.linetype[cam][ix] == 2)
        {
            //glPolygonOffset(-1.0f, -1.0f);
        }
        if(flag_draw_patch == 1)
        {
            if (Display_pgl.view.type == AVM_3D_VIEW )
            {
                glBindVertexArray(VAO_StylePatch_line[cam + 0 * MAX_CAM_NUM]);
            }
            else if(Display_pgl.view.type == AVM_2D_VIEW)
            {
                glBindVertexArray(VAO_2Davm_StylePatch_line[cam + 0 * MAX_CAM_NUM]);
            }
            else
            {
                glBindVertexArray(VAO_fisheye_StylePatch_line[cam + 0 * MAX_CAM_NUM]);
            }

            //glBindVertexArray(VAO_StylePatch_line[cam + 2 * style]);
            glDrawArrays(GL_TRIANGLES, ix * (219 * 2), (219 * 2));
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void draw_line_rect_style(int cam, int style, float* transfor, float* scaleMatrix, float* translationMatrix)
{
    int program = programHandleReverseLine[0];
    int ix = 0;
    float ptr_color[4];
    int index = -1;
    int strip_start;
    int strip_length;

    int para = (Display_pgl.view.type == AVM_3D_VIEW || Display_pgl.view.type == AVM_2D_VIEW)?1:0;

    glUniformMatrix4fv(glGetUniformLocation(programHandleReverseLine[0], "transformatiomMatrix"), 1, GL_FALSE, transfor);
    glUniformMatrix4fv(glGetUniformLocation(program, "ScopeArea.scaleMatrix"), 1, GL_FALSE, scaleMatrix);
    glUniformMatrix4fv(glGetUniformLocation(program, "ScopeArea.translationMatrix"), 1, GL_FALSE, translationMatrix);

    for (ix = 0; ix < dynamicData[para][cam].lineNumber[style]; ix++)
    {

        if (Display_pgl.view.type == AVM_3D_VIEW|| Display_pgl.view.type == AVM_2D_VIEW)
        {
            glBindVertexArray(VAO_3Davm_Line[cam + 2 * style]);
        }
        else
        {
            glBindVertexArray(VAO_fisheye_Line[cam + 2 * style]);
        }

        ptr_color[0] = dynamicData[para][cam].gl_colormap[style][++index];
        ptr_color[1] = dynamicData[para][cam].gl_colormap[style][++index];
        ptr_color[2] = dynamicData[para][cam].gl_colormap[style][++index];
        ptr_color[3] = dynamicData[para][cam].gl_colormap[style][++index];
        glUniform4fv(glGetUniformLocation(program, "color"), 1, ptr_color);


        if (ix == 0)
        {
            strip_start = 0;
            strip_length = dynamicData[para][cam].line_strip[style][ix];
        }
        else
        {
            strip_start = dynamicData[para][cam].line_strip[style][ix - 1];
            strip_length = dynamicData[para][cam].line_strip[style][ix] - dynamicData[para][cam].line_strip[style][ix - 1];
        }

        glDrawArrays(GL_TRIANGLE_STRIP, strip_start, strip_length);

        if(0)
        {
            glBindVertexArray(VAO_StylePatch_line[cam + 2 * style]);
            glDrawArrays(GL_TRIANGLES, ix * (219 * 2), (219 * 2));
        }

    }
}

/*======================================
*                                      *            
*        opengl draw setting           *    
*                                      *
=======================================*/
int avm_2d_display_loading_flag[2][5] = {0};
int avm_3d_display_loading_flag[2][5] = {0};

void draw_3D_avm_dynamicline(float carAngle, float *rotateReslut, int camera, int style, ViewType type, PGL_SUPPLY *sup_pgl)
{
    if(g_dynamicLine_Turn == 0) return;
    style = (style >= PARAM_PGL_MAX_STYLE_NUM)?(PARAM_PGL_MAX_STYLE_NUM - 1):style;    
    int program = glContext_ReverseLine.programID;
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glEnable(GL_DEPTH_TEST);
    int ix;
    Aligment_PGL_function(&carAngle, type);
    glUseProgram(program);

    int flag_weight_p0_0 = (int)autosys.current_info.line_info.flag_line_gpu_refresh;
    int flag_weight_p0_1 = (fabs(carAngle) > 1.0f)? 1:0;
    int flag_weight_p1_0 = (avm_2d_display_loading_flag[camera][style] == 0 && type == AVM_2D_VIEW);
    int flag_weight_p1_1 = (avm_3d_display_loading_flag[camera][style] == 0 && type == AVM_3D_VIEW);

    //if( flag_weight_p0_0 || flag_weight_p0_1 || (flag_weight_p1_0 || flag_weight_p1_1) )
    {
        Display_pgl.view.type = type;
        cal_dynamic_location_line(carAngle, camera, style, &Display_pgl);
        if(type == AVM_3D_VIEW)
        {
            int VAO_index = camera + MAX_CAM_NUM * 0;
            glBindVertexArray(VAO_3Davm_Line[VAO_index]);

            glBindBuffer(GL_ARRAY_BUFFER, VBO_3Davm_line[VAO_index][0]);
            if(upload_line_buffer_to_gpu(camera, 0, type) == 1) return;
            glBindBuffer(GL_ARRAY_BUFFER, VBO_3Davm_line[VAO_index][1]);
            if(upload_line_texture_buffer_to_gpu(camera, 0, type) == 1) return;

            glBindVertexArray(VAO_StylePatch_line[VAO_index]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_StylePatch_line[VAO_index][0]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, Display_pgl.buffer.StylePatch.indexNum * 3  * sizeof(float), Display_pgl.buffer.StylePatch.vert);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_StylePatch_line[VAO_index][1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, Display_pgl.buffer.StylePatch.indexNum * 2  * sizeof(float), Display_pgl.buffer.StylePatch.texture);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            avm_3d_display_loading_flag[camera][0]++;
        }
        else
        {
            int VAO_index = camera  + MAX_CAM_NUM * 0;
            glBindVertexArray(VAO_2Davm_Line[VAO_index]);

            glBindBuffer(GL_ARRAY_BUFFER, VBO_2Davm_line[VAO_index][0]);
            if(upload_line_buffer_to_gpu(camera, 0, type) == 1) return;
            glBindBuffer(GL_ARRAY_BUFFER, VBO_2Davm_line[VAO_index][1]);
            if(upload_line_texture_buffer_to_gpu(camera, 0, type) == 1) return;

            glBindVertexArray(VAO_2Davm_StylePatch_line[VAO_index]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_2Davm_StylePatch_line[VAO_index][0]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, Display_pgl.buffer.StylePatch.indexNum * 3  * sizeof(float), Display_pgl.buffer.StylePatch.vert);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_2Davm_StylePatch_line[VAO_index][1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, Display_pgl.buffer.StylePatch.indexNum * 2  * sizeof(float), Display_pgl.buffer.StylePatch.texture);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            avm_2d_display_loading_flag[camera][0]++;
        }
        carAnglePrev[camera] = carAngle;
        //autosys.current_info.line_info.flag_line_gpu_refresh = 0;
    }



    glUniform1i(glGetUniformLocation(program, "switchPosition"), 1);
    if(style % 2 == 0)
    {
        glUniform1i(glGetUniformLocation(program, "windows"), 1);
    }
    else
    {
        glUniform1i(glGetUniformLocation(program, "windows"), 2);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, glContext_ReverseLine.textureID[style/2]);
        glUniform1i(glGetUniformLocation(program, "colorbar"), 1);
    }
    glUniform1f(glGetUniformLocation(program, "yRervsed"), yReverse);

    //glUniformMatrix4fv(glGetUniformLocation(programHandleReverseLine[0], "transformatiomMatrix"), 1, GL_FALSE, rotateReslut); // Rotate3

    float transfermatrix[16] = 
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

    float *TransForMatrix = ((Display_pgl.view.type == AVM_3D_VIEW||autosys.zoom.cur_level>0 || Display_pgl.alignment.ON == 0))? rotateReslut : &transfermatrix;

    switch(Display_pgl.view.drawstyle)
    {
        case circle:
        draw_line_circle_style(camera, style, TransForMatrix, transfermatrix, transfermatrix, sup_pgl );
        break;
        case rectangle:
        draw_line_rect_style(camera, style, TransForMatrix, transfermatrix, transfermatrix);
        break;
    }
    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

int onlyineee = 0;

void draw_fish_line(float carAngle, float *rotateReslut, int camera, int style, cropped_view* cropped, ViewType type, PGL_SUPPLY *sup_pgl, float pglRotateAngle)
{
    if(type == FISHEYE_STATIC_DISTORTION && (!g_staticLine_Turn || !g_2ddynamicLine_Turn)) return;
    if(type == FISHEYE_DISTORTION && (!g_dynamicLine_Turn || !g_2ddynamicLine_Turn)) return;
    //if(type == FISHEYE_STATIC_SCENE && (!g_staticSceneLine_Turn)) return;
    //if(g_dynamicLine_Turn == 0 || g_2ddynamicLine_Turn == 0 || g_staticLine_Turn == 0) return;
    int program = glContext_ReverseLine.programID;
    carAngle = (carAngle == 0.0f)?0.05f:carAngle;
    Display_pgl.view.type = type;


    bool needUpdate = (autosys.current_info.line_info.flag_line_gpu_refresh ||
                       fabs(carAngle) > 1.0f ||
                       fisheye_avm_display_loading_flag[camera][style] == 0);

    needUpdate = true;
    m_sceneview_projection_matrix = rotateReslut;
    //* 2. update
    //if (needUpdate)
    {
        glUseProgram(program);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        Display_pgl.alignment.ON = 0;
        if(cal_dynamic_location_line(carAngle, camera, style, &Display_pgl) == 0) return;
        {
            int VAO_index = camera  + MAX_CAM_NUM * 0;
            glBindVertexArray(VAO_fisheye_Line[VAO_index]);

            glBindBuffer(GL_ARRAY_BUFFER, VBO_fisheye_line[VAO_index][0]);
            if(upload_line_buffer_to_gpu(camera, 0, type) == 1) return;
            glBindBuffer(GL_ARRAY_BUFFER, VBO_fisheye_line[VAO_index][1]);
            if(upload_line_texture_buffer_to_gpu(camera, 0, type) == 1) return;

            glBindVertexArray(VAO_fisheye_StylePatch_line[VAO_index]);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_fisheye_StylePatch_line[VAO_index][0]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, Display_pgl.buffer.StylePatch.indexNum * 3  * sizeof(float), Display_pgl.buffer.StylePatch.vert);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_fisheye_StylePatch_line[VAO_index][1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, Display_pgl.buffer.StylePatch.indexNum * 2  * sizeof(float), Display_pgl.buffer.StylePatch.texture);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        fisheye_avm_display_loading_flag[camera][style]++;
        carAnglePrev[camera] = carAngle;
    }
    int switchPosition = 0;
    float transferM[16], scaleM[16], transM[16];
    CalcFisheyeMatrices(camera, rotateReslut, cropped, transferM, scaleM, transM, &switchPosition, type);
    //pglRotate
	{
        if(camera == 1) pglRotateAngle *= -1.0f;
		float rotateMatrix[16], resulttransfermatrix[16];
		myCreateRotate4(2, pglRotateAngle, rotateMatrix);
		myMatrixMultiplyby4(resulttransfermatrix, rotateMatrix, transferM);
		memcpy(transferM, resulttransfermatrix, sizeof(float) * 16);
	}


    glUniform1i(glGetUniformLocation(program, "switchPosition"), switchPosition);
    glUniform1i(glGetUniformLocation(program, "windows"), 0);
    glUniform1f(glGetUniformLocation(program, "yRervsed"), yReverse);
    if(type == FISHEYE_STATIC_SCENE || type == FISHEYE_STATIC_DISTORTION)
    {
        glUniform1i(glGetUniformLocation(program, "windows"), 1);
    }
    else
    {
        glUniform1i(glGetUniformLocation(program, "windows"), 3);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, glContext_ReverseLine.textureID[style]);
        glUniform1i(glGetUniformLocation(program, "colorbar"), 1);
    }
    switch (Display_pgl.view.drawstyle)
    {
        case circle:
            draw_line_circle_style(camera, style, transferM, scaleM, transM, sup_pgl);
        break;
        case rectangle:
            draw_line_rect_style(camera, style, transferM, scaleM, transM);
        break;
    }

    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

}


