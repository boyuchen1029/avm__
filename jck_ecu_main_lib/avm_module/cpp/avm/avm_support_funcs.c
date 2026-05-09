/**
 *******************************************************************************
 * @file : avm_support_funcs.h
 * @describe :
 *
 * @author : Bennit Yang
 * @verstion : 0.1.0.
 * @date 20211104 0.1.0 Bennit Yang.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/
//
// Created by AI611 on 2021/11/4.
//

#include "../../system.h"
#include "user_control/ui.h"

#include "canbus.h"
#include "avm_ui_init.h"

#include "avm/avm_support_funcs.h"
#include "avm/fp_source.h"

#include <math.h>
#include <string.h>
#include <time.h>

#define KPH 0.2778f                 // 1000 / 60 / 60
#define PERIMETER_URX 2.017         // M
#define CIRCLE_DEGREE 360.0
#if (PLAFORM == NT98690)
#define M_PI		3.14159265358979323846	/* pi */
#endif

float tire_rotate_angle(float speed, float fps)
{
    if(fps < 0.1 || speed <= 0.0)
    {
        return 0.0f;
    }

    double mps = speed * KPH; // kph convert to mps
    double angle = fmod(mps / PERIMETER_URX * CIRCLE_DEGREE / fps, CIRCLE_DEGREE); // 2.017:,
    return (float)angle;
}

void cal_tire_roate_angle(float *angle, float *speed, float fps, int gear)
{
    if (*speed > 130.0)
    {
        *speed = 0.0;
    }
    float delta = tire_rotate_angle(*speed, fps);

    if (gear != 8)
    {
        *angle -= delta;
    }
    else if (gear == 8)
    {
        *angle += delta;
    }

    if (*angle >= 360.0f)
        *angle -= 360.0f;
    else if (*angle < 0.0f)
        *angle += 360.0f;
}

void cal_tire_rudder_rotation_matrix(float * matrix, float rudder, int tireNum)
{
    if(tireNum == 2 || tireNum == 3)
    {
        // FL and FR tire
        double cosinRu = cos( rudder * M_PI / 180.0 );
        double sinRu = sin( rudder * M_PI / 180.0 );
        *(matrix + 0) = (float)cosinRu;
        *(matrix + 2) = (float)sinRu;
        *(matrix + 4 * 1 + 1) = 1.0f;
        *(matrix + 4 * 2) = (float)sinRu * -1.0f;
        *(matrix + 4 * 2 + 2) = (float)cosinRu;
        *(matrix + 4 * 3 + 3) = 1.0f;

        /*
        matrix looks like below:
        ---
        float matrix[4][4] =
        {
                {cosinRu,   0,      sinRu,      0},
                {0,         1,      0,          0},
                {-sinRu,    0,      cosinRu,    0},
                {0,         0,      0,          1}
        };
        ---
        */
    }
    else
    {
        // 後輪：使用單位矩陣
        matrix[0]  = 1.0f; matrix[1]  = 0.0f; matrix[2]  = 0.0f; matrix[3]  = 0.0f;
        matrix[4]  = 0.0f; matrix[5]  = 1.0f; matrix[6]  = 0.0f; matrix[7]  = 0.0f;
        matrix[8]  = 0.0f; matrix[9]  = 0.0f; matrix[10] = 1.0f; matrix[11] = 0.0f;
        matrix[12] = 0.0f; matrix[13] = 0.0f; matrix[14] = 0.0f; matrix[15] = 1.0f;
    }

}

float calculate_time(struct timeval start, struct timeval end)//sec
{
    float dtime;
    dtime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)* 0.000001;
    return dtime;
}

static float angleTurned = 0.0f;
static unsigned char flagCountDown = 0;
static struct timeval start, end;
static int circleOld;
float frame360 = 0.0;
float count360 = 0.0;

void car_rotate_360(avm_ui_info_t * uiInfo, float * theta, float fps)
{
    if(uiInfo->circleStart == 0)
    {
        angleTurned = 0;
        flagCountDown = 0;
    }
    else if(uiInfo->circleStart == 1)
    {
        if(flagCountDown == 0)
        {
            #if(PLAFORM == IMAX8 || PLAFORM == SPHE8368_P || PLAFORM == NT98690)
                float degree = 3.5f;
            #elif(PLAFORM == MTK2712)
                frame360 += 1.0;
                count360 += fps;
                float dofps = count360 / frame360;
                float degree = 6.0;//360.0 / 6.0 / dofps;
            #endif
            
            angleTurned += degree;
            
            if(angleTurned >= 360.0f)
            {
                *theta = 270.0f;
                angleTurned = 0.0f;
                flagCountDown = 1; // start count down for 2s.
                frame360 = 0.0;
                count360 = 0.0;
                gettimeofday(&start, NULL);
            }
            else
            {
                *theta -= degree;
            }
        }
        else if(flagCountDown == 1)
        {
            gettimeofday(&end, NULL);
            float temp_time = calculate_time(start, end);
            if(temp_time > 2.0)
            {
                uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
                flagCountDown = 0;
                uiInfo->circleStart = 2;
            }
        }
    }
    
    if(circleOld != uiInfo->circleStart)
    {
        // if(uiInfo->circleStart == 1)
        // {
        //     int data[] = {1};
        //     write_can(0x06, 1, data);
        // }
        // else
        // {
        //     int data[] = {0};
        //     write_can(0x06, 1, data);
        // }
        circleOld = uiInfo->circleStart;
    }
    
}

void init_avmUiInfo(avm_ui_info_t * uiInfo)
{
    // uiInfo
    memset(uiInfo, 0, sizeof(avm_ui_info_t));
    //VIEW_RECORD_4_IN_1 VIEW_AUTOSYS_3D_AVM_2D_AVM VIEW_FULLSCREEN VIEW_START

    //uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM; //VIEW_ELEAD_LAYOUT;
    //uiInfo->viewold =VIEW_AUTOSYS_3D_AVM_2D_AVM;//VIEW_ELEAD_LAYOUT;

    uiInfo->view = VIEW_ELEAD_LAYOUT; //VIEW_ELEAD_LAYOUT;
    uiInfo->viewold = VIEW_ELEAD_LAYOUT;//VIEW_ELEAD_LAYOUT;

    ui_info(uiInfo);
}
