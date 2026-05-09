#include "autosys.h"
#include "autosys_page.h"
#include "autosys_IVI.h"
#include "../avm_module/include/avm/maths.h"
#include "../avm_module/include/avm/fp_source.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "../avm_module/include/avm/avm_support_funcs.h"

autosys_avm_package autosys;

float GL_IndeMatrix[16] =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};

autosys_current_info m_current_info;

PointF* ZoomLayerPara[PARAM_ZOOM_BIRDVIEW_MAX];
PointF table_layer1[12] = 
{
    {2.0f, -4.0f}, {0.0f, -4.0f}, {-2.0f, -4.0f},
    {0.0f,  0.0f}, {0.0f,  0.0f}, {0.0f,  0.0f},
    {0.0f,  0.0f}, {0.0f,  0.0f}, {0.0f,  0.0f},
    {2.0f,  4.0f}, {0.0f,  4.0f}, {-2.0f,  4.0f}
};

PointF table_layer2[12] = 
{
    {3.0f, -4.0f}, {0.0f, -6.0f}, {-3.0f, -4.0f},
    {0.0f,  0.0f}, {0.0f,  0.0f}, {0.0f,  0.0f},
    {0.0f,  0.0f}, {0.0f,  0.0f}, {0.0f,  0.0f},
    {3.0f,  6.0f}, {0.0f,  6.0f}, {-3.0f,  6.0f}
};

PointF table_layer3[12] = 
{
    {3.0f, -4.0f}, {0.0f, -6.0f}, {-3.0f, -4.0f},
    {0.0f,  0.0f}, {0.0f,  0.0f}, {0.0f,  0.0f},
    {0.0f,  0.0f}, {0.0f,  0.0f}, {0.0f,  0.0f},
    {3.0f,  6.0f}, {0.0f,  6.0f}, {-3.0f,  6.0f}
};

int m_layerNum = 0;;
int m_box_col = 0;
int m_box_row = 0;
int *magiValCountArray;
float **diffratioArray;
int** layoutArray;
void autosys_set_bowl_shadowArea();
void autosys_set_bowl_seethArea();
void autosys_set_zoomIn_para();

/*==================================================================

               autosys_package INIT

*==================================================================*/

void init_info(char* EVSpath)
{
    Point screen                     = {.X = 1280, .Y = 720};
    Point TOP2DRANGE                 = {.X = 472, .Y = 720};
    autosys.system_info.SCREEN       = screen;
    autosys.current_info.TOP2D_RANGE = TOP2DRANGE;
    autosys.current_info.notify_show_cur_page_context = 1;
    camera_para_t *cur_2dcar_matrix = &autosys.current_info.projection.Projection_2DCAR;
    camera_para_t *m_2dcar_matrix   = &m_current_info.projection.Projection_2DCAR;
    memcpy(cur_2dcar_matrix->Rotate, &GL_IndeMatrix, sizeof(float) * 16);
    memcpy(m_2dcar_matrix->Rotate, &GL_IndeMatrix, sizeof(float) * 16);
    /*read bowlsetting config*/
    FILE *fp = fp_source_app("EVSTable/config/Bowl_setting/bowlsetting.config", "rb");
    if(fp == NULL)
    {
        AVM_LOGE(LOG_RED("[ERROR]")" init_info no read the bowlsetting.config");
        SET_STATUS_CALIBRATION_IVI(car_model_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
    }
    else
    {
        char line[100];
        int carSize[2];
        int offset[4];
        int blendAngle[4];
        while (fgets(line, sizeof(line), fp))
        {
            if (strncmp(line, "carSize", 7) == 0)
            {
                sscanf(line, "carSize:%d,%d", &carSize[0], &carSize[1]);
            }
            else if (strncmp(line, "offset", 6) == 0)
            {
                sscanf(line, "offset:%d,%d,%d,%d\n", &offset[0], &offset[1], &offset[2], &offset[3]);
            }
            else if (strncmp(line, "knife", 5) == 0)
            {
                sscanf(line, "knife:%d,%d,%d,%d\n", &blendAngle[0], &blendAngle[1], &blendAngle[2], &blendAngle[3]);
            }
        }
        fclose(fp);
        autosys.system_info.CarSize.X = carSize[0];
        autosys.system_info.CarSize.Y = carSize[1];
        memcpy(&autosys.system_info.BOWL_offset, offset, sizeof(int) * 4);
        memcpy(&autosys.system_info.BOWL_angle, blendAngle, sizeof(int) * 4);
        AVM_LOGI("[system] carsize w[%d], h[%d]\n", autosys.system_info.CarSize.X, autosys.system_info.CarSize.Y);
        AVM_LOGI("[system] bowloffset : f[%d] b[%d] l[%d] r[%d]\n",
                 autosys.system_info.BOWL_offset[0],
                 autosys.system_info.BOWL_offset[1],
                 autosys.system_info.BOWL_offset[2],
                 autosys.system_info.BOWL_offset[3]);
        AVM_LOGI("[system] bowlangle : f[%d] b[%d] l[%d] r[%d]\n",
                 autosys.system_info.BOWL_angle[0],
                 autosys.system_info.BOWL_angle[1],
                 autosys.system_info.BOWL_angle[2],
                 autosys.system_info.BOWL_angle[3]);
    }
    /*read carsetting config*/
    fp = fp_source_app("/EVSTable/car_model/car_obj_map.bin", "rb");
    if(fp == NULL)
    {
        AVM_LOGE(LOG_RED("[ERROR]")" init_info no read the car_obj.bin");
        SET_STATUS_CALIBRATION_IVI(config, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
    }
    else
    {
        int *p = (int *)autosys.system_info.carModel_obj_map;
        for (int i = 0; i < 50; i++) {
            p[i] = -1;
        }
        int totalbox, innbox_num;
        int Txbuffer[10];
        int isopendoorcar = 0;
        fread(&totalbox, sizeof(int), 1, fp);
        for (int ix = 0; ix < totalbox; ix++)
        {
            fread(&innbox_num, sizeof(int), 1, fp);
            fread(Txbuffer, sizeof(int), innbox_num, fp);
            for (int iy = 0; iy < innbox_num; iy++)
            {
                autosys.system_info.carModel_obj_map[ix][iy] = Txbuffer[iy];
                if (Txbuffer[iy] >= 0)
                    isopendoorcar++;
            }
        }

        char opendoor[] = "opendoor\n";
        char closedoor[] = "normal\n";
        autosys.system_info.carModel_item = (isopendoorcar > 14) ? 1 : 0;
        AVM_LOGI("[system] carModel_item num %d, type is %s\n", isopendoorcar, (isopendoorcar > 14) ? opendoor : closedoor);

        AVM_LOGI("[system] carModel_obj_map box[0] [%d][%d][%d][%d]\n",
                 autosys.system_info.carModel_obj_map[0][0],
                 autosys.system_info.carModel_obj_map[0][1],
                 autosys.system_info.carModel_obj_map[0][2],
                 autosys.system_info.carModel_obj_map[0][3]);
        AVM_LOGI("[system] carModel_obj_map box[1] [%d][%d][%d][%d]\n",
                 autosys.system_info.carModel_obj_map[1][0],
                 autosys.system_info.carModel_obj_map[1][1],
                 autosys.system_info.carModel_obj_map[1][2],
                 autosys.system_info.carModel_obj_map[1][3]);
        fclose(fp);
    }

    fp = fp_source_app("/EVSTable/config/Car_setting/seethroughFrame.config", "rb");
    if(fp == NULL)
    {
        AVM_LOGE(LOG_RED("[ERROR]")" no reading the seethroughFrame.config");
        SET_STATUS_CALIBRATION_IVI(config, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
        autosys.system_info.seetrough_frame_lineColor[0] = 1.0f;
        autosys.system_info.seetrough_frame_lineColor[1] = 0.0f;
        autosys.system_info.seetrough_frame_lineColor[2] = 0.0f;
    }
    else
    {
        char line[100];
        int seeFrameColor[4] = {0};
        while (fgets(line, sizeof(line), fp))
        {
            if (strncmp(line, "BaseColor", 9) == 0)
            {
                sscanf(line, "BaseColor:%d,%d,%d,%d\n", 
                &seeFrameColor[0],
                &seeFrameColor[1],
                &seeFrameColor[2],
                &seeFrameColor[3]
                );
            }
        }
        fclose(fp);
        autosys.system_info.seetrough_frame_lineColor[0] = (float)seeFrameColor[0] / 255.0f;
        autosys.system_info.seetrough_frame_lineColor[1] = (float)seeFrameColor[1] / 255.0f;
        autosys.system_info.seetrough_frame_lineColor[2] = (float)seeFrameColor[2] / 255.0f;
    }

    /* readCarSetting.config */
    fp = fp_source_app("/EVSTable/config/Car_setting/CarModel_setting.config", "rb");
    if(fp == NULL)
    {
        AVM_LOGE(LOG_RED("[ERROR]")" no reading the carmodel_setting.config");
        SET_STATUS_CALIBRATION_IVI(config, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
        return;
    }
    else
    {
        char line[100];
        int carLightMode = 0;
        int skyboxmode = 0;
        while (fgets(line, sizeof(line), fp))
        {
            if (strncmp(line, "Lightmode", 8) == 0)
            {
                sscanf(line, "Lightmode = [%d]", &carLightMode);
            }
            else if (strncmp(line, "skyboxmode", 9) == 0)
            {
                sscanf(line, "skyboxmode = [%d]", &skyboxmode);
            }
        }
        fclose(fp);
        autosys.system_info.light_mode = carLightMode;
        autosys.system_info.sky_mode   = skyboxmode;
        AVM_LOGI("[system] carModel_lightMode =  [%d]\n", carLightMode);
    }

    /* readZoomIn config */
    fp = fp_source_app("/EVSTable/config/zoomin.config", "rb");
    if(fp == NULL)
    {
        AVM_LOGE(LOG_RED("[ERROR]")" no reading the zoomin_setting.config");
        SET_STATUS_CALIBRATION_IVI(config, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
    }
    else
    {

        char line[100];

        int magiValCount = 0;
        int diffratioCount = 0;
        int layoutCount = 0;

        while (fgets(line, sizeof(line), fp))
        {
            if (strncmp(line, "layerNum", 7) == 0)
            {
                sscanf(line, "layerNum = [%d]", &m_layerNum);

                magiValCountArray = (int *)calloc(m_layerNum, sizeof(int));

                diffratioArray = (float **)calloc(m_layerNum, sizeof(float *));
                for (int ix = 0; ix < m_layerNum; ix++)
                {
                    diffratioArray[ix] = (float *)calloc(2, sizeof(float));
                }
            }
            else if (strncmp(line, "box_col", 6) == 0)
            {
                sscanf(line, "box_col = [%d]", &m_box_col);
            }
            else if (strncmp(line, "box_row", 6) == 0)
            {
                sscanf(line, "box_row = [%d]", &m_box_row);
                layoutArray = (int **)calloc(m_box_row, sizeof(int *));
                for (int ix = 0; ix < m_box_row; ix++)
                {
                    layoutArray[ix] = (int *)calloc(m_box_col, sizeof(int));
                }
            }
            else if (strncmp(line, "magiVal_", 7) == 0)
            {
                char goal[128];
                sprintf(goal, "magiVal_%d = [%%d]", magiValCount);
                sscanf(line, goal, &magiValCountArray[magiValCount]);
                magiValCount++;
            }
            else if (strncmp(line, "diffratio_", 9) == 0)
            {
                char goal[128];
                sprintf(goal, "diffratio_%d = [%%f, %%f]", diffratioCount);
                sscanf(line, goal, &diffratioArray[diffratioCount][0], &diffratioArray[diffratioCount][1]);
                diffratioCount++;
            }
            else if (strncmp(line, "layout_", 6) == 0)
            {
                char goal[128];
                switch (m_box_col)
                {
                case 1:
                    sprintf(goal, "layout_%d = [%%d]", layoutCount);
                    sscanf(line, goal, &layoutArray[layoutCount][0]);
                    break;
                case 2:
                    sprintf(goal, "layout_%d = [%%d, %%d]", layoutCount);
                    sscanf(line, goal, &layoutArray[layoutCount][0], &layoutArray[layoutCount][1]);
                    break;
                case 3:
                    sprintf(goal, "layout_%d = [%%d, %%d, %%d]", layoutCount);
                    sscanf(line, goal, &layoutArray[layoutCount][0], &layoutArray[layoutCount][1],
                           &layoutArray[layoutCount][2]);
                    break;
                case 4:
                    sprintf(goal, "layout_%d = [%%d, %%d, %%d, %%d]", layoutCount);
                    sscanf(line, goal, &layoutArray[layoutCount][0], &layoutArray[layoutCount][1],
                           &layoutArray[layoutCount][2], &layoutArray[layoutCount][3]);
                    break;
                }
                layoutCount++;
            }
        }
        fclose(fp);
    }

    /*debug_use*/
    AVM_LOGI("[system] layerNum: %d\n", m_layerNum);
    AVM_LOGI("[system] box_col: %d\n", m_box_col);
    AVM_LOGI("[system] box_row: %d\n", m_box_row);


    AVM_LOGI("[system] magiValCountArray:\n");
    for (int i = 0; i < m_layerNum; i++) {
        AVM_LOGI("[system] magiVal_%d = %d\n", i, magiValCountArray[i]);
    }


    AVM_LOGI("[system] diffratioArray:\n");
    for (int i = 0; i < m_layerNum; i++) {
        AVM_LOGI("[system] diffratio_%d = [%f, %f]\n", i, diffratioArray[i][0], diffratioArray[i][1]);
    }

    AVM_LOGI("[system] layoutArray:\n");
    for (int i = 0; i < m_box_row; i++) {
        AVM_LOGI("[system] layout_%d = [", i);
        for (int j = 0; j < m_box_col; j++) {
            printf("%d", layoutArray[i][j]);
            if (j < m_box_col - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }

    /*seethrough initValue */
    autosys.system_info.seethrough_sigmoid_alpha      = 5.0f;
    autosys.system_info.seethrough_top2d_alpha_center = 70.0f / 130.0f;
    autosys.system_info.seethrough_top2d_alpha_range  = 40.0f /130.0f;
}

int is_version_compatible(int libMaj, int libMin,
                          int toolMaj, int toolMin)
{
    for (size_t i = 0; i < NUM_RULES; i++) 
    {
        const CompatRule *r = &compat_table[i];
        if (r->lib_maj == libMaj && r->lib_min == libMin) 
        {
            return (toolMaj >= r->tmaj_min && toolMaj <= r->tmaj_max)
                && (toolMin >= r->tmin_min && toolMin <= r->tmin_max);
        }
    }
    return 0;
}

void __AUTOSYS_CHECK_VERSION_STATUS__()
{
    char libautosys_version[128] = {0};
    char AVM_TOOLPC_version[128] = {0};
    char Page_name[128] = {0};
    char RxBuffer[128] = {0};
    int PC_VERSION_YEAR  = 0;
    int PC_VERSION_MAJOR = 0;
    int PC_VERSION_MINOR = 0;
    sprintf(libautosys_version, "%d-%d", VERSION_MAJOR, VERSION_MINOR);
    sprintf(Page_name, "EVSTable/config/Page_setting/ePage%d.bin", 0);
    FILE* fp = fp_source_app(Page_name, "rb");
    if (fp == NULL)
    {
        sprintf(AVM_TOOLPC_version, "%d-%d", 0, 0);
    }
    else
    {
        size_t n = fread(RxBuffer, sizeof(char), 11, fp);
        RxBuffer[n] = '\0';
        fclose(fp);
        sscanf(RxBuffer, "%d-v-%d-%d", &PC_VERSION_YEAR, &PC_VERSION_MAJOR, &PC_VERSION_MINOR);
        sprintf(AVM_TOOLPC_version, "%d-%d", PC_VERSION_MAJOR, PC_VERSION_MINOR);
    }
    AVM_LOGI("[system] libautosys version: %s\n", libautosys_version);
    AVM_LOGI("[system] AVM_TOOLPC version: %s\n", AVM_TOOLPC_version);

    if (is_version_compatible(VERSION_MAJOR, VERSION_MINOR, PC_VERSION_MAJOR, PC_VERSION_MINOR)) 
    {
        AVM_LOGI("[system] version match\n");
    } 
    else 
    {
        AVM_LOGI(LOG_RED("[WARNING]")"[system] version not match\n");
    }
}
struct timeval autosys_start, autosys_end;
void Initial_autosys_avm_package(char* EVSpath)
{
    gettimeofday(&autosys_start, NULL);
    /*reset*/
    memset(&autosys.current_info.carmodel_info,0,sizeof(CAR_MODEL_INFO));
    /*flag*/
    autosys.g_flag.fuc_zoom_birdView = 1;
    autosys.g_flag.fuc_user_control  = 1;
    autosys.g_flag.fuc_seethrough    = 1;
    autosys.g_flag.fuc_touch         = 1;
    autosys.g_flag.fuc_fxaa          = 1;
    autosys.g_flag.fuc_smod          = 1;
    /*info*/
    init_info(EVSpath);
    /*zoom*/
    autosys_set_zoomIn_para();
    /*touch*/
    autosys.system_info.click_during_Count = PRRAM_TOUCH_CLICK_RESPONSECOUNT;
    autosys.system_info.click_process_Time = PARAM_TOUCH_CLICK_PROCESSTIME;
    /*shadow */
    autosys_set_bowl_shadowArea();
    /*seethrough vertex */
    autosys_set_bowl_seethArea();
    /*readConfig*/
    avm_ePage_readConfig(EVSpath);
    //avm_ePage_singleConfig(EVSpath, 0);
    gettimeofday(&autosys_end, NULL);
    printf("gen_texture_buf = %f\n", calculate_time(autosys_start, autosys_end));
    autosys.current_info.flag_system_init = 1;
}

/*==================================================================

                push data to autosys_package

*==================================================================*/
void autosys_assgin_2Dprojection2core(camera_para_t *para)
{
    opengl_projection_matrix *autosys_sys_matrix = &autosys.system_info.projection;
    opengl_projection_matrix *autosys_cui_matrix = &autosys.current_info.projection;
    memcpy(&autosys_sys_matrix->Projection_2Davm, para, sizeof(camera_para_t));
    memcpy(&autosys_cui_matrix->Projection_2Davm, para, sizeof(camera_para_t));
    memcpy(&m_current_info.projection.Projection_2Davm, para, sizeof(camera_para_t));
}
void autosys_assgin_3Dprojection2core(camera_para_t *para)
{
    opengl_projection_matrix *autosys_matrix = &autosys.system_info.projection;
    autosys_matrix->Projection_3Davm = para;
    //memcpy(&autosys_matrix->Projection_3Davm, para, sizeof(camera_para_t));
}
/*==================================================================

                set data to autosys_package

*==================================================================*/
void autosys_set_bowl_shadowArea()
{
    autosys_system_info *system = &autosys.system_info;
    tmp_shadow* shadow = &autosys.shadow;
    shadow->TOP2D_realWorldlocation[0].X = -(system->CarSize.X / 2.0f)  - system->BOWL_offset[2];
    shadow->TOP2D_realWorldlocation[0].Y = -(system->CarSize.Y / 2.0f)  - system->BOWL_offset[0];
    shadow->TOP2D_realWorldlocation[1].X =  (system->CarSize.X / 2.0f)  + system->BOWL_offset[3];
    shadow->TOP2D_realWorldlocation[1].Y =  (system->CarSize.Y / 2.0f)  + system->BOWL_offset[1];
}

void autosys_set_bowl_seethArea()
{
    Point* shadow_real        = &autosys.shadow.TOP2D_realWorldlocation;
    VECF3 input_start         = {.X = shadow_real[0].X/130.0f, .Z = shadow_real[0].Y/130.0f, .Y = 0.01f};
    VECF3 input_end           = {.X = shadow_real[1].X/130.0f, .Z = shadow_real[1].Y/130.0f, .Y = 0.01f};

    VECF3 topLeft = input_start;
    VECF3 bottomRight = input_end;
    VECF3 topRight = {.X = bottomRight.X, .Y = topLeft.Y, .Z = topLeft.Z};
    VECF3 bottomLeft = {.X = topLeft.X, .Y = bottomRight.Y, .Z = bottomRight.Z};

    autosys.program_context[SEETHROUGH].vertexCoord[0] = (float*)calloc(18 , sizeof(float));
    autosys.program_context[SEETHROUGH].alphaValue[0]  = (float*)calloc(6 , sizeof(float));
    float* vertexCoord = autosys.program_context[SEETHROUGH].vertexCoord[0];
    float* alphavalue  = autosys.program_context[SEETHROUGH].alphaValue[0];
    float bufferRange = 1.05;
    //v1
    alphavalue[0] = 1.0f;
    alphavalue[1] = 1.0f;
    alphavalue[2] = 1.0f;
    alphavalue[3] = 1.0f;
    alphavalue[4] = 1.0f;
    alphavalue[5] = 1.0f;
    /* 1.02 (Zoom) in slightly to avoid black edges */
    vertexCoord[0] = topLeft.X * bufferRange;
    vertexCoord[1] = 0.0f;
    vertexCoord[2] = topLeft.Z* bufferRange;

    vertexCoord[3] = bottomLeft.X* bufferRange;
    vertexCoord[4] = 0.0f;
    vertexCoord[5] = bottomLeft.Z* bufferRange;

    vertexCoord[6] = topRight.X* bufferRange;
    vertexCoord[7] = 0.0f;
    vertexCoord[8] = topRight.Z* bufferRange;

    // v2
    vertexCoord[9] = bottomLeft.X* bufferRange;
    vertexCoord[10] = 0.0f;
    vertexCoord[11] = bottomLeft.Z* bufferRange;

    vertexCoord[12] = topRight.X* bufferRange;
    vertexCoord[13] = 0.0f;
    vertexCoord[14] = topRight.Z* bufferRange;

    vertexCoord[15] = bottomRight.X* bufferRange;
    vertexCoord[16] = 0.0f;
    vertexCoord[17] = bottomRight.Z* bufferRange;
        
    AVM_LOGI("Seethrough vertices (x, y, z):\n");
    for (int i = 0; i < 18; i += 3)
    {
        AVM_LOGI("Vertex %d: (%f, %f, %f)\n", i / 3 + 1, vertexCoord[i], vertexCoord[i + 1], vertexCoord[i + 2]);
    }
}

void autosys_cal_bowl_shadow_range()
{
    camera_para_t *cur_matrix = &autosys.system_info.projection.Projection_2Davm;
    //camera_para_t *cur_matrix = &autosys.current_info.projection.Projection_2Davm;
    Point* shadow_real        = &autosys.shadow.TOP2D_realWorldlocation;
    tmp_shadow* shadow        = &autosys.shadow;
    VECF4 input_start         = {.X = shadow_real[0].X/130.0f, .Y = shadow_real[0].Y/130.0f, .Z = 0.1f, .W = 1.0f};
    VECF4 input_end           = {.X = shadow_real[1].X/130.0f, .Y = shadow_real[1].Y/130.0f, .Z = 0.1f, .W = 1.0f};
    VECF4 result_start        =  VECF4_Mutiple_4by1(input_start, cur_matrix->Rotate);
    VECF4 result_end          =  VECF4_Mutiple_4by1(input_end,   cur_matrix->Rotate);


    shadow->TOP2D_Normlocation[0].X = (result_start.X + 1.0f) / 2.0f;
    shadow->TOP2D_Normlocation[0].Y = ((result_start.Z * -1.0f + 1.0f)/2.0f);
    shadow->TOP2D_Normlocation[1].X = (result_end.X + 1.0f) / 2.0f;
    shadow->TOP2D_Normlocation[1].Y = ((result_end.Z * -1.0f + 1.0f)/2.0f);
}

void autosys_set_zoomin2DprojectionmartixReset()
{
    autosys_touch *touch_info = &autosys.touch;
    camera_para_t *sys_matrix = &autosys.system_info.projection.Projection_2Davm;
    camera_para_t *cur_matrix = &autosys.current_info.projection.Projection_2Davm;
    camera_para_t *cur_2dcar_matrix = &autosys.current_info.projection.Projection_2DCAR;
    memcpy(cur_matrix, sys_matrix, sizeof(camera_para_t));
    //memcpy(cur_matrix->Rotate, sys_matrix->Rotate, sizeof(float) * 16);

    memcpy(cur_2dcar_matrix->Rotate, &GL_IndeMatrix, sizeof(float) * 16);
    memcpy(m_current_info.projection.Projection_2DCAR.Rotate, &GL_IndeMatrix, sizeof(float) * 16);
    autosys.zoom.cur_level = 0;
    autosys.zoom.firs_select_block = 255;
    //touch_info->preious_status = TOUCH_LEAVE;
}

void autosys_set_zoomIn_para()
{
    #if 0 //old version
    autosys.zoom.Each_level_num[0] = 1;
    autosys.zoom.Each_level_num[1] = 2;
    autosys.zoom.Each_level_num[2] = 4;
    autosys.zoom.Each_level_num[3] = 8;
    autosys.zoom.linetable[0][0] = 4;
    autosys.zoom.linetable[0][1] = 5;
    autosys.zoom.linetable[1][0] = 2;
    autosys.zoom.linetable[1][1] = 2;
    autosys.zoom.linetable[2][0] = 2;
    autosys.zoom.linetable[2][1] = 2;
    autosys.zoom.linetable[3][0] = 2;
    autosys.zoom.linetable[3][1] = 2;


    ZoomLayerPara[1] = table_layer1;
    ZoomLayerPara[2] = table_layer2;
    ZoomLayerPara[3] = table_layer3;
    #endif
    
    autosys.zoom.cur_level = 0;
    autosys.zoom.block_size_col = m_box_col;
    autosys.zoom.block_size_row = m_box_row;
    autosys.zoom.enable_layer_num = m_layerNum;
    for(int enablelayer = 0; enablelayer < m_layerNum; enablelayer++)
    {
        autosys.zoom.Each_level_num[enablelayer] = magiValCountArray[enablelayer];
        memcpy(autosys.zoom.diff_ratio[enablelayer], diffratioArray[enablelayer], sizeof(float) * 2);
    }

    for(int row = 0; row < m_box_row; row++)
    {
        memcpy(autosys.zoom.enableblock[row], layoutArray[row], sizeof(int) * m_box_col);
    }


    free(magiValCountArray);
    for (int i = 0; i < m_layerNum; i++) {
        free(diffratioArray[i]);
    }
    free(diffratioArray);
    for (int i = 0; i < m_box_row; i++) {
        free(layoutArray[i]);
    }
    free(layoutArray);
}

PointF autosys_get_realP_fromTouchPoint(float src_x, float src_y)
{
    zoom_birdView *zoom = &autosys.zoom;
    float tmpx = src_x;
    float tmpy = src_y;
    if(zoom->cur_level <= 1)
    {

    }
    else
    {
        autosys_current_info *info = &autosys.current_info;
        tmpx += (-zoom->preXshift * 0.5f * (float)info->TOP2D_RANGE.X) +  ( (zoom->preZoom-1.0f) * (float)info->TOP2D_RANGE.X / 2.0f);
        tmpy += (zoom->preYshift * 0.5f * (float)info->TOP2D_RANGE.Y) +  ( (zoom->preZoom-1.0f) * (float)info->TOP2D_RANGE.Y / 2.0f);
        tmpx /= zoom->preZoom;
        tmpy /= zoom->preZoom;
    }
    PointF dst = {tmpx, tmpy};
    AVM_LOGI(" preshift{%f,%f},zoom{%f}, src{%f,%f}, dst{%f,%f}\n",
    zoom->preXshift,zoom->preYshift,
    zoom->preZoom,
    src_x,src_y,
    dst.X,
    dst.Y);
    
    return dst;
}

void autosys_set_zoomin2DprojectionmatrixByVector(float xdiff,float ydiff, int zoom_value)
{
    zoom_birdView *zoom = &autosys.zoom;
    camera_para_t *sys_matrix = &autosys.system_info.projection.Projection_2Davm;
    camera_para_t *cur_matrix = &m_current_info.projection.Projection_2Davm;
    camera_para_t *cur_2dcar_matrix = &m_current_info.projection.Projection_2DCAR;
    int level = zoom->cur_level;
    if (level > 0)
    {
        int first_num = zoom->firs_select_block;
        float x_position_shift = 0.0f;
        float y_position_shift = 0.0f;

        float y_axis_ratio = zoom->diff_ratio[level][0];
        float x_axis_ratio = zoom->diff_ratio[level][1];

        // AVM_LOGI("level %d, firste_num %d, zoomlater [%f, %f]\n", 
        // level,
        // first_num,
        // xdiff,
        // ydiff);
        x_position_shift = (float)zoom_value * xdiff * x_axis_ratio;
        y_position_shift = (float)zoom_value * ydiff * y_axis_ratio;

        // switch (first_num)
        // {
        //     case 3:
        //     case 4:
        //     case 5:
        //     case 6:
        //     case 7:
        //     case 8:
        //     zoom->cur_level = zoom->cur_level -1;
        //     AVM_LOGI("No ACCTION\n");
        //     return;
        //     break;
        // }
        zoom->preXshift = x_position_shift;
        zoom->preYshift = y_position_shift;
        zoom->preZoom   = (float)zoom_value;
        // AVM_LOGI("shift %f, %f\n",x_position_shift, y_position_shift);
        cur_matrix->Model[0]  = sys_matrix->Model[0] * (float)zoom_value;
        cur_matrix->Model[5]  = sys_matrix->Model[5] * (float)zoom_value;
        cur_matrix->Model[12] = (float)x_position_shift;
        cur_matrix->Model[13] = (float)y_position_shift;
        rotate_callculate_2D(&m_current_info.projection.Projection_2Davm);

        // cur_matrix->Rotate[0] = sys_matrix->Rotate[0] * (float)zoom_value;
        // cur_matrix->Rotate[5] = sys_matrix->Rotate[5] * (float)zoom_value;
        // cur_matrix->Rotate[12] = sys_matrix->Rotate[12] + (float)x_position_shift;
        // cur_matrix->Rotate[13] = sys_matrix->Rotate[13] + (float)y_position_shift;
        cur_2dcar_matrix->Rotate[0] = (float)zoom_value;
        cur_2dcar_matrix->Rotate[5] = (float)zoom_value;
        cur_2dcar_matrix->Rotate[12] = (float)x_position_shift;
        cur_2dcar_matrix->Rotate[13] = (float)y_position_shift;
    }
    else
    {
        //autosys_set_zoomin2DprojectionmartixReset();
        zoom->preXshift = 0.0f;
        zoom->preYshift = 0.0f;
        zoom->preZoom = 1.0f;
        zoom->firs_select_block = 255;
        memcpy(cur_matrix, sys_matrix, sizeof(camera_para_t));
        memcpy(cur_2dcar_matrix->Rotate, &GL_IndeMatrix, sizeof(float) * 16);
    }
}

void autosys_set_zoomin2Dprojectionmatrix(int zoom_value, int sele_num)
{
    zoom_birdView *zoom = &autosys.zoom;
    camera_para_t *sys_matrix = &autosys.system_info.projection.Projection_2Davm;
    camera_para_t *cur_matrix = &m_current_info.projection.Projection_2Davm;
    camera_para_t *cur_2dcar_matrix = &m_current_info.projection.Projection_2DCAR;
    int level = zoom->cur_level;
    if (level > 0)
    {
        int first_num = zoom->firs_select_block;
        float x_position_shift_unit = 1.0f / (float)(zoom->linetable[0][0] - 1) * 0.5f;
        float y_position_shift_unit = 1.0f / (float)(zoom->linetable[0][1] - 1) * 0.5f;
        float x_position_shift = 0.0f;
        float y_position_shift = 0.0f;
        #if 0
        switch (first_num)
        {
        case 0:
            x_position_shift =  x_position_shift_unit * (float)zoom_value * 2.0f;
            y_position_shift = -y_position_shift_unit * (float)zoom_value * 2.0f;
            break;
        case 1:
            x_position_shift = -x_position_shift_unit * (float)zoom_value * 2.0f;
            y_position_shift = -y_position_shift_unit * (float)zoom_value * 2.0f;
            break;
        case 2:
            x_position_shift = x_position_shift_unit * (float)zoom_value;
            y_position_shift = -y_position_shift_unit * (float)zoom_value;
            break;
        case 3:
            x_position_shift = -x_position_shift_unit * (float)zoom_value;
            y_position_shift = -y_position_shift_unit * (float)zoom_value;
            break;
        case 4:
            x_position_shift = x_position_shift_unit * (float)zoom_value;
            y_position_shift = y_position_shift_unit * (float)zoom_value;
            break;
        case 5:
            x_position_shift = -x_position_shift_unit * (float)zoom_value * 2.0f;
            y_position_shift = y_position_shift_unit  * (float)zoom_value * 2.0f;
            break;
        case 6:
            x_position_shift = x_position_shift_unit * (float)zoom_value * 2.0f;
            y_position_shift = y_position_shift_unit * (float)zoom_value * 2.0f;
            break;
        case 7:
            x_position_shift = -x_position_shift_unit * (float)zoom_value * 2.0f;
            y_position_shift = y_position_shift_unit  * (float)zoom_value * 2.0f;
            break;
        }
        #else
        AVM_LOGI("level %d, firste_num %d, zoomlater [%f, %f]\n", 
        level,
        first_num,
        ZoomLayerPara[level][first_num].X,
        ZoomLayerPara[level][first_num].Y);
        x_position_shift = x_position_shift_unit * (float)zoom_value * ZoomLayerPara[level][first_num].X;
        y_position_shift = y_position_shift_unit * (float)zoom_value * ZoomLayerPara[level][first_num].Y;

        switch (first_num)
        {
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            zoom->cur_level = zoom->cur_level -1;
            AVM_LOGI("No ACCTION\n");
            return;
            break;
        }

        // switch (first_num)
        // {
        //case 0:
            // x_position_shift =  x_position_shift_unit * (float)zoom_value * 2.0f;
            // y_position_shift = -y_position_shift_unit * (float)zoom_value * 2.0f;
           // break;
        // case 1:
        // AVM_LOGI("XD11[%f]\n", (float)zoom_value);
        //     x_position_shift = 0.0f ;
        //     y_position_shift = -y_position_shift_unit * (float)zoom_value * 6.0f;
        //     break;
        // case 10:
        //     x_position_shift = 0.0f ;
        //     y_position_shift = y_position_shift_unit * (float)zoom_value * 6.0f;
        //     break;
        // case 3:
        //     x_position_shift = -x_position_shift_unit * (float)zoom_value;
        //     y_position_shift = -y_position_shift_unit * (float)zoom_value;
        //     break;
        // case 4:
        //     x_position_shift = x_position_shift_unit * (float)zoom_value;
        //     y_position_shift = y_position_shift_unit * (float)zoom_value;
        //     break;
        // case 5:
        //     x_position_shift = -x_position_shift_unit * (float)zoom_value * 2.0f;
        //     y_position_shift = y_position_shift_unit  * (float)zoom_value * 2.0f;
        //     break;
        // case 6:
        //     x_position_shift = x_position_shift_unit * (float)zoom_value * 2.0f;
        //     y_position_shift = y_position_shift_unit * (float)zoom_value * 2.0f;
        //     break;
        // case 7:
        //     x_position_shift = -x_position_shift_unit * (float)zoom_value * 2.0f;
        //     y_position_shift = y_position_shift_unit  * (float)zoom_value * 2.0f;
            //break;
        // }
        #endif
        // AVM_LOGI("shift %f, %f\n",x_position_shift, y_position_shift);
        cur_matrix->Model[0]  = sys_matrix->Model[0] * (float)zoom_value;
        cur_matrix->Model[5]  = sys_matrix->Model[5] * (float)zoom_value;
        cur_matrix->Model[12] = sys_matrix->Model[12] + (float)x_position_shift;
        cur_matrix->Model[13] = sys_matrix->Model[13] + (float)y_position_shift;
        rotate_callculate_2D(&m_current_info.projection.Projection_2Davm);

        // cur_matrix->Rotate[0] = sys_matrix->Rotate[0] * (float)zoom_value;
        // cur_matrix->Rotate[5] = sys_matrix->Rotate[5] * (float)zoom_value;
        // cur_matrix->Rotate[12] = sys_matrix->Rotate[12] + (float)x_position_shift;
        // cur_matrix->Rotate[13] = sys_matrix->Rotate[13] + (float)y_position_shift;
        cur_2dcar_matrix->Rotate[0] = (float)zoom_value;
        cur_2dcar_matrix->Rotate[5] = (float)zoom_value;
        cur_2dcar_matrix->Rotate[12] = 0.0f + (float)x_position_shift;
        cur_2dcar_matrix->Rotate[13] = 0.0f + (float)y_position_shift;
    }
    else
    {
        //autosys_set_zoomin2DprojectionmartixReset();
        memcpy(cur_matrix, sys_matrix, sizeof(camera_para_t));
        memcpy(cur_2dcar_matrix->Rotate, &GL_IndeMatrix, sizeof(float) * 16);
        autosys.zoom.firs_select_block = 255;
    }
}

void autosys_updata_zoominprojectionmatrix()
{
    camera_para_t *tmp_matrix_2dAVM         = &m_current_info.projection.Projection_2Davm;
    camera_para_t *tmp_matrix_2dCAR         = &m_current_info.projection.Projection_2DCAR;
    camera_para_t *autosys_cur_matrix_2dAVM = &autosys.current_info.projection.Projection_2Davm;
    camera_para_t *autosys_cur_2dcar_2dCAR  = &autosys.current_info.projection.Projection_2DCAR;

    memcpy(autosys_cur_matrix_2dAVM->Rotate, tmp_matrix_2dAVM->Rotate, sizeof(float) * 16);
    memcpy(autosys_cur_2dcar_2dCAR->Rotate,  tmp_matrix_2dCAR->Rotate, sizeof(float) * 16);
}
/*==================================================================

                get data to autosys_package

*==================================================================*/
float *autosys_get_zoomin2Dprojectionmatrix()
{
    return autosys.current_info.projection.Projection_2Davm.Rotate;
}

float *autosys_get_zoomin2DCARprojectionmatrix()
{
    return autosys.current_info.projection.Projection_2DCAR.Rotate;
}

camera_para_t *autosys_get_AVM3DProjectionmatrix()
{
    return autosys.system_info.projection.Projection_3Davm;
}


Scope autosys_get_bowl_shadow_range()
{
    tmp_shadow* shadow          = &autosys.shadow;
    autosys_system_info *system = &autosys.system_info;
    tmp_seethrough *seethroughbuffer = &autosys.seethrough;
    float shader_w = shadow->TOP2D_Normlocation[1].X - shadow->TOP2D_Normlocation[0].X;
    float shader_h = shadow->TOP2D_Normlocation[1].Y - shadow->TOP2D_Normlocation[0].Y;

    float shift_x = seethroughbuffer->tune_shift;
    float shift_y = 0.1f;
    // float scale     = (1.0f + 0.2f);
    // float scale_inv = (1.0f - 0.2f);
    float scale, scale_inv;
    if(seethroughbuffer->tune_scale >= 1.0f) 
    {
        scale     = seethroughbuffer->tune_scale;
        scale_inv = 1.0f - (seethroughbuffer->tune_scale - 1.0f);
    }
    else if(seethroughbuffer->tune_scale < 1.0f)
    {
        scale     = seethroughbuffer->tune_scale;
        scale_inv = 1.0f + (1.0f - seethroughbuffer->tune_scale);
    }


    Scope scope =
	{
		{ (shadow->TOP2D_Normlocation[1].X  + shift_x - (shader_w)) * scale, (1.0f - shadow->TOP2D_Normlocation[1].Y) * (scale)},
		{ (shadow->TOP2D_Normlocation[0].X  + shift_x + (shader_w)) * scale_inv, (1.0f - shadow->TOP2D_Normlocation[0].Y) * (scale_inv)},
	};

    //shift, rotate, scale



    return scope;
}
