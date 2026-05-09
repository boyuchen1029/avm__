/**
 *******************************************************************************
 * @file : canbus_control.cpp
 * @describe :
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20220104 0.1.0 Linda.
 *******************************************************************************
 */
//
// Created by AI611 on 2021/11/4.
//

// #define DEBUG

#include "../../system.h"
#include "user_control/ui.h"
#include "user_control/user_command.h"
#include "user_control/keypad.h"
#include "avm/fp_source.h"
#include "avm/avm_support_funcs.h"
#include "set_memory.h"
#include "control.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

extern int cleanCommendFlag;
extern int change_4in1_view_flag;
static int seeThrough = 0;

static void kaypad_control_view(user_command_mode_t *user_command, avm_ui_info_t *avmuiInfo)
{
    avmuiInfo->flag2davm = 0;
    switch (*user_command)
    {
    case CMD_Q:
        avmuiInfo->view = VIEW_START;
        break;
    case CMD_W:
        avmuiInfo->view = VIEW_FRON_PAGE;
        break;
    case CMD_T:
        avmuiInfo->view = VIEW_ALLEY_2D_AVM;
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_E:
        avmuiInfo->view = VIEW_2D_LEFT_TIRE_2D_AVM;
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_R:
        avmuiInfo->view = VIEW_2D_RIGHT_TIRE_2D_AVM;
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_Y:
        avmuiInfo->view = VIEW_2D_DUAL_SV_2D_AVM;
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_U:
        avmuiInfo->view = VIEW_3D_LEFT_SV_2D_AVM; // 1
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_I:
        avmuiInfo->view = VIEW_3D_RIGHT_SV_2D_AVM; // 1
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_O:
        avmuiInfo->view = VIEW_3D_RV_2D_AVM;
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_P:
        avmuiInfo->view = VIEW_R_GEAR_2D_RV_2D_AVM; // 1
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_A:
        avmuiInfo->view = VIEW_ALLEY_VIEW_2D_RV;
        break;
    case CMD_S:
        avmuiInfo->view = VIEW_2D_LEFT_TIRE_2D_RV;
        break;
    case CMD_D:
        avmuiInfo->view = VIEW_2D_RIGHT_TIRE_2D_RV;
        break;
    case CMD_F:
        avmuiInfo->view = VIEW_3D_LEFT_SV_2D_RV;
        break;
    case CMD_G:
        avmuiInfo->view = VIEW_3D_RIGHT_SV_2D_RV;
        break;
    case CMD_H:
        avmuiInfo->view = VIEW_3D_RV;
        break;
    case CMD_J:
        avmuiInfo->view = VIEW_2D_FV_2D_AVM;
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_K:
        avmuiInfo->view = VIEW_2D_FV_2D_RV;
        break;
    case CMD_L:
        avmuiInfo->view = VIEW_2D_LEFT_SV_2D_RV;
        break;
    case CMD_Z:
        avmuiInfo->view = VIEW_2D_RIGHT_SV_2D_RV;
        break;
    case CMD_X:
        avmuiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM; // 1
        avmuiInfo->flag2davm = 1;
        break;
    case CMD_C:
        avmuiInfo->view = VIEW_ODA_2D_DUAL_SV;
        break;
    case CMD_V:
        avmuiInfo->view = VIEW_LEFT_LIGHT_3D_SV_2D_SV;
        break;
    case CMD_B:
        avmuiInfo->view = VIEW_RIGHT_LIGHT_3D_SV_2D_SV;
        break;
    case CMD_N:
        avmuiInfo->view = VIEW_RECORD_4_IN_1;
        break;
    case CMD_M:
        if (seeThrough == 1)
        {
            seeThrough = 0;
        }
        else
        {
            seeThrough = 1;
        }
        cleanCommendFlag = 1;
        break;
    case CMD_COMMA:
        fwrite_readpixels(1920, 1080, "result.rgba");
        break;
    case CMD_NONE:
        break;
    default:
        break;
    }
}

static void control(avm_ui_info_t *uiInfo, can_bus_info_t *can, user_command_mode_t *user_command)
{
    // keypadOn = 0;
    /*    if(uiInfo->view == VIEW_RECORD_4_IN_1)
        {
            kaypad_control_view(user_command, uiInfo);
            // keypadOn = 1;
        }
        else
        {

            if(can[1].gear == 'F' || can[1].turningSignal == 'F')
            {
                uiInfo->view = VIEW_START;
                AVM_LOGI("Please Check gear or turningSignal\n");
                AVM_LOGI("gear:%c steer:%-7.3lf Turning: %c\n", can[1].gear, can[1].steer, can[1].turningSignal);
            }
            else if((can[1].gear == 'P' && can[0].gear != 'P') ||
                    (can[1].gear == 'P' && can[1].turningSignal == 'O' && can[0].turningSignal != 'O'))
            {
                uiInfo->view = VIEW_START;
                uiInfo->circleStart = 1;
                AVM_LOGI("Do gear P\n");
                AVM_LOGI("gear:%c steer:%-7.3lf Turning: %c\n", can[1].gear, can[1].steer, can[1].turningSignal);
            }
            else if((can[1].gear == 'N' && can[0].gear != 'N') ||
                    (can[1].gear == 'D' && can[0].gear != 'D') ||
                    (can[1].gear == 'N' && can[1].turningSignal == 'O' && can[0].turningSignal != 'O') ||
                    (can[1].gear == 'D' && can[1].turningSignal == 'O' && can[0].turningSignal != 'O')
                    )
            {
                uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
                AVM_LOGI("Do gear N , D\n");
                AVM_LOGI("gear:%c steer:%-7.3lf Turning: %c\n", can[1].gear, can[1].steer, can[1].turningSignal);
            }
            else if(can[1].gear == 'R')
            {
                uiInfo->view = VIEW_R_GEAR_2D_RV_2D_AVM;
                AVM_LOGI("Do gear R\n");
                AVM_LOGI("gear:%c steer:%-7.3lf Turning: %c\n", can[1].gear, can[1].steer, can[1].turningSignal);
            }
            else
            {
                if(can[1].turningSignal == 'R')
                {
                    uiInfo->view = VIEW_3D_RIGHT_SV_2D_AVM;
                    AVM_LOGI("Do turningSignal R\n");
                }
                else if(can[1].turningSignal == 'L' )
                {
                    uiInfo->view = VIEW_3D_LEFT_SV_2D_AVM;
                    AVM_LOGI("Do turningSignal L\n");
                }
                else
                {
                    kaypad_control_view(user_command, uiInfo);
                    // keypadOn = 1;
                }
            }
        }

        if(seeThrough == 1 && can[1].speed < 20.0)
        {
            uiInfo->seeThrough = 1;
        }
        else if((seeThrough == 1 && can[1].speed > 25.0) || seeThrough == 0)
        {
            uiInfo->seeThrough = 0;
        }
    */

    // if (can[1].gear == 16)
    // {
    //     uiInfo->view = VIEW_R_GEAR_2D_RV_2D_AVM;
    // }
    // else if (can[1].gear == 32 || can[1].gear == 8 || can[1].gear == 0)
    // {
    //     if (can[1].turningSignal == 1)
    //     {
    //         uiInfo->view = VIEW_3D_LEFT_SV_2D_AVM;
    //     }
    //     else if (can[1].turningSignal == 2)
    //     {
    //         uiInfo->view = VIEW_3D_RIGHT_SV_2D_AVM;
    //     }
    //     else
    //     {
    //         uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
    //     }
    // }

#if (SEETHROUGH == 1)
    uiInfo->seeThrough = 1;
#elif (SEETHROUGH == 0)
    //uiInfo->seeThrough = 0;
#endif

#if (MODsta == 1)
    //uiInfo->mod = 1;
#elif (MODsta == 0)
    uiInfo->mod = 0;
#endif

    // if (uiInfo->seeThrough == 1 || uiInfo->mod == 1)
    // {
    //     uiInfo->avm2dFBO = 1;
    // }
    // else
    // {
    //     uiInfo->avm2dFBO = 0;
    // }
}
int IR_MODE = 0;
int user_command_old = 0;
static void control_3d_avm_direction(avm_ui_info_t *uiInfo, user_command_mode_t *user_command, camera_para_t *camerapara)
{
    if (uiInfo->view == VIEW_AUTOSYS_3D_AVM_2D_AVM)
    {
        if (*user_command == 5)
        {
            if (IR_MODE == 0)
                camerapara->exPara.theta -= 2.2; // vertical degree //0.0
            else if (IR_MODE == 1)
                uiInfo->view = VIEW_3D_LEFT_SV_2D_AVM;
        }
        else if (*user_command == 7)
        {
            if (IR_MODE == 0)
                camerapara->exPara.theta += 2.2; // vertical degree //0.0
            else if (IR_MODE == 1)
                uiInfo->view = VIEW_3D_RIGHT_SV_2D_AVM;
        }
        else if (*user_command == 9)
        {
            if (IR_MODE == 0)
            {
                camerapara->exPara.phi += 2.2; // hor degree -70
                if (camerapara->exPara.phi > 52)
                {
                    camerapara->exPara.phi = 52;
                }
            }
        }
        else if (*user_command == 6)
        {
            if (IR_MODE == 0)
            {
                camerapara->exPara.phi = -52.0; //-38.0 -56.0
                camerapara->exPara.theta = 270.0;
            }
        }
        else if (*user_command == 3)
        {
            if (IR_MODE == 0)
            {
                camerapara->exPara.phi -= 2.2; // hor degree -70

                if (camerapara->exPara.phi < -52)
                {
                    camerapara->exPara.phi = -52;
                }
            }
        }
        else if (*user_command == 14 && user_command_old == -1)
        {
            if (IR_MODE == 0)
            {
                change_4in1_view_flag = 0;
                IR_MODE = 1;
                uiInfo->view = VIEW_R_GEAR_2D_RV_2D_AVM;
            }
            else if (IR_MODE == 1)
            {
                IR_MODE = 2;
                // uiInfo->view = VIEW_RECORD_4_IN_1;
                change_4in1_view_flag = 1;
            }
            else if (IR_MODE == 2)
            {
                change_4in1_view_flag = 0;
                IR_MODE = 0;
                uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
            }
        }
    }
    else if (uiInfo->view == VIEW_R_GEAR_2D_RV_2D_AVM || uiInfo->view == VIEW_3D_LEFT_SV_2D_AVM || uiInfo->view == VIEW_3D_RIGHT_SV_2D_AVM)
    // else if(uiInfo->view !=  uiInfo->viewold)
    {
        if (*user_command == 5)
        {
            if (IR_MODE == 0)
                camerapara->exPara.theta -= 2.2; // vertical degree //0.0
            else if (IR_MODE == 1)
                uiInfo->view = VIEW_3D_LEFT_SV_2D_AVM;
        }
        else if (*user_command == 7)
        {
            if (IR_MODE == 0)
                camerapara->exPara.theta += 2.2; // vertical degree //0.0
            else if (IR_MODE == 1)
                uiInfo->view = VIEW_3D_RIGHT_SV_2D_AVM;
        }
        else if (*user_command == 14 && user_command_old == -1)
        {
            if (IR_MODE == 0)
            {
                change_4in1_view_flag = 0;
                IR_MODE = 1;
                uiInfo->view = VIEW_R_GEAR_2D_RV_2D_AVM;
            }
            else if (IR_MODE == 1)
            {
                IR_MODE = 2;
                // uiInfo->view = VIEW_RECORD_4_IN_1;
                change_4in1_view_flag = 1;
            }
            else if (IR_MODE == 2)
            {
                change_4in1_view_flag = 0;
                IR_MODE = 0;
                uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
            }
        }
    }
    else if (change_4in1_view_flag == 1)
    // else if(uiInfo->view !=  uiInfo->viewold)
    {
        if (*user_command == 14 && user_command_old == -1)
        {
            if (IR_MODE == 0)
            {
                change_4in1_view_flag = 0;
                IR_MODE = 1;
                uiInfo->view = VIEW_R_GEAR_2D_RV_2D_AVM;
            }
            else if (IR_MODE == 1)
            {
                IR_MODE = 2;
                // uiInfo->view = VIEW_RECORD_4_IN_1;
                change_4in1_view_flag = 1;
            }
            else if (IR_MODE == 2)
            {
                change_4in1_view_flag = 0;
                IR_MODE = 0;
                uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
            }
        }
    }

    // if(uiInfo->view == VIEW_AUTOSYS_3D_AVM_2D_AVM)
    // {
    //     // if(*user_command == CMD_6)
    //     // {
    //     //     camerapara->exPara.theta -= 1.2;// vertical degree //0.0
    //     // }
    //     // else if(*user_command == CMD_4)
    //     // {
    //     //     camerapara->exPara.theta += 1.2;// vertical degree //0.0
    //     // }
    //     // else if(*user_command == CMD_8)
    //     // {
    //     //     camerapara->exPara.phi += 1.2;  // hor degree -70
    //     //     if(camerapara->exPara.phi > 56)
    //     //     {
    //     //         camerapara->exPara.phi = 56;
    //     //     }
    //     // }
    //     // else if(*user_command == CMD_2)
    //     // {
    //     //     camerapara->exPara.phi -= 1.2;  // hor degree -70

    //     //     if(camerapara->exPara.phi < -56)
    //     //     {
    //     //         camerapara->exPara.phi = -56;
    //     //     }
    //     // }
    //     // else if(*user_command == CMD_PLUS)
    //     // {
    //     //     camerapara->exPara.eyeRadius += 0.05;
    //     // }
    //     // else if(*user_command == CMD_MINUS)
    //     // {
    //     //     camerapara->exPara.eyeRadius -= 0.05;
    //     // }
    //     // else if(*user_command == CMD_DIVIDE)
    //     // {
    //     //     init_3d_avm_parameter(camerapara);
    //     // }

    //     if (*user_command == IR_LEFT)
    //     {
    //             if (IR_MODE == 0)
    //                 camerapara->exPara.theta -= 1.2; // vertical degree //0.0
    //             else if (IR_MODE == 1)
    //                 uiInfo->view = VIEW_3D_LEFT_SV_2D_AVM;
    //     }
    //     else if (*user_command == IR_RIGHT)
    //     {
    //         if(IR_MODE == 0)
    //                 camerapara->exPara.theta += 1.2; // vertical degree //0.0
    //         else if (IR_MODE == 1)
    //                 uiInfo->view = VIEW_3D_RIGHT_SV_2D_AVM;
    //     }
    //     else if (*user_command == IR_UP)
    //     {
    //         if (IR_MODE == 0)
    //         {
    //             camerapara->exPara.phi += 1.2; // hor degree -70
    //             // if (camerapara->exPara.phi > 56)
    //             // {
    //             //     camerapara->exPara.phi = 56;
    //             // }
    //         }
    //     }
    //     else if (*user_command == IR_CENTER)
    //     {
    //         if(IR_MODE == 0)
    //         {
    //             camerapara->exPara.phi = -56.0; //-38.0 -56.0
    //             camerapara->exPara.theta = 270.0;
    //         }
    //     }
    //     else if (*user_command == IR_DOWN)
    //     {
    //         if (IR_MODE == 0)
    //         {
    //             camerapara->exPara.phi -= 1.2; // hor degree -70

    //             // if (camerapara->exPara.phi < -56)
    //             // {
    //             //     camerapara->exPara.phi = -56;
    //             // }
    //         }
    //     }
    //     else if (*user_command == IR_POWER && user_command_old == -1)
    //     {
    //         if (IR_MODE == 0)
    //         {
    //             IR_MODE = 1;
    //            uiInfo->view =  VIEW_R_GEAR_2D_RV_2D_AVM;
    //         }
    //         else if (IR_MODE == 1)
    //         {
    //             IR_MODE = 2;
    //             uiInfo->view = VIEW_RECORD_4_TO_1;
    //         }
    //         else if (IR_MODE == 2)
    //         {
    //             IR_MODE = 0;
    //             uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
    //         }
    //     }
    //     else if (*user_command == CMD_PLUS)
    //     {
    //             camerapara->exPara.eyeRadius += 0.05;
    //     }
    //     else if (*user_command == CMD_MINUS)
    //     {
    //             camerapara->exPara.eyeRadius -= 0.05;
    //     }
    //     else if (*user_command == CMD_DIVIDE)
    //     {
    //             init_3d_avm_parameter(camerapara);
    //     }

    //     float eye_radius_tmp = fabs(camerapara->exPara.phi);
    //     // eye_radius = 8.4 - (eye_radius_tmp * 0.044);

    //     // if(*user_command == CMD_M || *user_command == CMD_COMMA)
    //     // {
    //     //     cleanCommendFlag = 1;// vertical degree //0.0
    //     // }
    //     // else
    //     // {
    //     //     cleanCommendFlag = 0;
    //     // }

    //     // *user_command = CMD_NONE;

    //     // AVM_LOGI("para3Davm.exPara.phi = %f \n",para3Davm.exPara.phi);
    //     // AVM_LOGI("para3Davm.exPara.theta = %f \n",para3Davm.exPara.theta);
    //     // AVM_LOGI("para3Davm.exPara.eyeRadius = %f \n",para3Davm.exPara.eyeRadius);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[0] = %f \n",para3Davm.exPara.tmpCenter[0]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[1] = %f \n",para3Davm.exPara.tmpCenter[1]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[2] = %f \n",para3Davm.exPara.tmpCenter[2]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[3] = %f \n",para3Davm.exPara.tmpCenter[3]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[4] = %f \n",para3Davm.exPara.tmpCenter[4]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[5] = %f \n",para3Davm.exPara.tmpCenter[5]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[6] = %f \n",para3Davm.exPara.tmpCenter[6]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[7] = %f \n",para3Davm.exPara.tmpCenter[7]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[8] = %f \n",para3Davm.exPara.tmpCenter[8]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[9] = %f \n",para3Davm.exPara.tmpCenter[9]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[10] = %f \n",para3Davm.exPara.tmpCenter[10]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[11] = %f \n",para3Davm.exPara.tmpCenter[11]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[12] = %f \n",para3Davm.exPara.tmpCenter[12]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[13] = %f \n",para3Davm.exPara.tmpCenter[13]);
    //     // AVM_LOGI("para3Davm.exPara.tmpCenter[14] = %f \n",para3Davm.exPara.tmpCenter[14]);
    // }
    // else if(uiInfo->view ==  VIEW_R_GEAR_2D_RV_2D_AVM || uiInfo->view ==  VIEW_3D_LEFT_SV_2D_AVM || uiInfo->view ==  VIEW_3D_RIGHT_SV_2D_AVM )
    // // else if(uiInfo->view !=  uiInfo->viewold)
    // {
    //     if (*user_command == IR_LEFT)
    //     {
    //             if (IR_MODE == 0)
    //             camerapara->exPara.theta -= 1.2; // vertical degree //0.0
    //             else if (IR_MODE == 1)
    //             uiInfo->view = VIEW_3D_LEFT_SV_2D_AVM;
    //     }
    //     else if (*user_command == IR_RIGHT)
    //     {
    //             if (IR_MODE == 0)
    //             camerapara->exPara.theta += 1.2; // vertical degree //0.0
    //             else if (IR_MODE == 1)
    //             uiInfo->view = VIEW_3D_RIGHT_SV_2D_AVM;
    //     }
    //     else if (*user_command == IR_POWER && user_command_old == -1)
    //     {
    //             if (IR_MODE == 0)
    //             {
    //             IR_MODE = 1;
    //             uiInfo->view = VIEW_R_GEAR_2D_RV_2D_AVM;
    //             }
    //             else if (IR_MODE == 1)
    //             {
    //             IR_MODE = 2;
    //             uiInfo->view = VIEW_RECORD_4_IN_1;
    //             }
    //             else if (IR_MODE == 2)
    //             {
    //             IR_MODE = 0;
    //             uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
    //             }
    //     }
    // }
    // else if(uiInfo->view ==  VIEW_RECORD_4_IN_1)
    // // else if(uiInfo->view !=  uiInfo->viewold)
    // {
    //    if (*user_command == IR_POWER && user_command_old == -1)
    //     {
    //             if (IR_MODE == 0)
    //             {
    //             IR_MODE = 1;
    //             uiInfo->view = VIEW_R_GEAR_2D_RV_2D_AVM;
    //             }
    //             else if (IR_MODE == 1)
    //             {
    //             IR_MODE = 2;
    //             uiInfo->view = VIEW_RECORD_4_IN_1;
    //             }
    //             else if (IR_MODE == 2)
    //             {
    //             IR_MODE = 0;
    //             uiInfo->view = VIEW_AUTOSYS_3D_AVM_2D_AVM;
    //             }
    //     }
    // }
    user_command_old = *user_command;
}
void avm_panel_move(avm_ui_info_t *uiInfo, camera_para_t *camerapara, int mode, float theta, float phi)
{

    switch (mode)
    {
    case 0:
        camerapara->exPara.phi = phi;
        break;
    case 1:
        camerapara->exPara.theta = theta;
        break;
    case 2:
        camerapara->exPara.phi = phi;
        camerapara->exPara.theta = theta;
    default:
        break;
    }
}
void avm_control(avm_ui_info_t *uiInfo, can_bus_info_t *can, user_command_mode_t *user_command, camera_para_t *camerapara)
{
    control(uiInfo, can, user_command);
    // if(keypadOn)
    // {
    //     kaypad_control_view(*user_command, uiInfo);
    // }
    control_3d_avm_direction(uiInfo, user_command, camerapara);

    // control_camara_parameter(*user_command, camerapara);
}