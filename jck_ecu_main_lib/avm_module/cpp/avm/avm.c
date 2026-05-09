/*
 *  Copyright 2017-2018 NXP
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or late.
 *
 */

/*
 * @file mx8_v4l2_cap_drm.c
 *
 * @brief MX8 Video For Linux 2 driver test application
 *
 */

/* Standard Include Files */
#include <time.h>
#if(PLAFORM != NT98690)
#include "avm_version.h"
#endif
#include "../../system.h"

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <sys/shm.h>

#include <linux/videodev2.h>
#include <linux/input.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
// #include <sys/time.h>

#include <pthread.h>
#include <stdbool.h>

#include "avm/GLHeader.h"
#include "avm/bb_func.h"
#include "avm_ui_init.h"
#include "canbus.h"
#include "../camera.h"

#include "avm/camera_matrix.h"
#include "user_control/keypad_control.h"
#include "user_control/ui.h"

#include "avm/fp_source.h"
#include "avm/avm_support_funcs.h"
#include "draw_windows_handle.h"
#include "program/set_framebuffer.h"

#include "set_memory.h"
#include "program/program_line.h"
#include "program/program_car.h"
#include "program/program_rgba.h"
#include "program/program_main.h"
#include "program/program_uyvy.h"
#include "program/program_y.h"
#include "program/program_mod.h"
#include "program/program_blur.h"
// #include "program/BSD_func.h"
#include "program/program_seeThrough.h"
#include "program/program_TSAA.h"
#include "mod/mod_main.h"

#include "control.h"
#include "display_location.h"
#include "avm_2d_3d_para.h"
#include "avmDefine.h"
#include "../../autosys/autosys.h"
#include "../../autosys/autosys_API.h"
#include "../../autosys/autosys_IVI.h"
#include "../../autosys/autosys_driver.h"
#include "avm/avm.h"
#include "avm/stb_image.h" 

char *EVSTable_path[4];
int MOD_ON_OFF_flag = 0;
int AutoSys_mod_front_flag = 0;
int AutoSys_mod_back_flag = 0;
int AutoSys_mod_left_flag = 0;
int AutoSys_mod_right_flag = 0;
int mod_recall_flag = 0;

int g_PANEL_WIDTH = 720;
int g_PANEL_HEIGHT = 1280;


#if (PLAFORM == SPHE8368_P || PLAFORM == SPHE6702)
float yReverse = -1.0; // for GL yReverse; 1.0 = positiv, -1.0 = negative;
#else
float yReverse = 1.0; // for GL yReverse; 1.0 = positiv, -1.0 = negative;
#endif

// AVM UI
#define LAST_CAN 0
#define NOW_CAN 1
struct timeval canbus_start, canbus_end;
struct timeval img_start, img_end;
can_bus_info_t canBusAvm[2];
int cleanCommendFlag = 0;

int recordstartFlag = 0;
int frame = 0;

static trans_camera_t transCamera[3] = {{1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0}, {960, 960, 960, 960, 960, 960}};
trans_camera_t avm_2d[2];
camera_para_t para2Davm, para3Davm, paracar, paraSeeThrough, paraCorrectEye;

float Rotateangle = 0;

avm_ui_info_t avmUiInfo;

static FBO_t *fbo_2davm;
static FBO_t fbo_y;
static FBO_t carmodel_fbo;
static FBO_t seethroug_crope_FBO;
// static FBO_t fisheye_blur_FBO1;
// static FBO_t fisheye_blur_FBO2;

/*alfred 20230203*/
//stTouchData touchdata;
// stAVMPGLdrawView MODE_VIEW[5];

int cam_ch = 12;
int pre_avm_page = -100;
static int m_usr_click = 0;
static int m_OnTouchflag = 0;
static int m_preTouchArea = 0;
int g_reset_360rotateflag = 0;
int g_onlyonetime_360rotateflag = 1;
int g_enable_360system = 1;
int g_reset_540sysyem = 0;
int g_carModel_color = 0;
int g_3D_carModel_alpha_value = 0;
int g_avm_camera_flag = 0;

IVI_WARNING ivi_warning;

static int frame_count = 0;
void GOTOTOUCHVIEW(stTouchData *TouchDt)
{
	touchdata.x = TouchDt->x;
	touchdata.y = TouchDt->y;
}

static GLuint bufferEVSPBO[VAO_NUM];
int carSizeW, carSizeH;
float carSizeWScale, carSizeHScale;
int last2Dflag = 0;
#ifdef MSAA
FBO_t fboMSAA;
#endif
int avm_fbo;
float fps360 = 0;
struct timeval start360, end360;
float current_carAngle = 0;

//#ifdef DEBUG_FPS
float temp_time;
struct timeval start, end;
struct timeval start1, end1;
int fpscount = 0;
//#endif


void init_parameterPanel();
void init_carmodelRotate();

static void RotateCallculate(can_bus_info_t canbus)
{
	rotate_callculate(&para3Davm);
	rotate_callculate_2D(&para2Davm);

	if (1)
	{
		seeThrough_rotate_callculate(&paraSeeThrough, &canbus);
	}
}
static void generate_FXAA_texture()
{
#if FXAA

	autosys_page *avm       = &autosys.avm_page;
	ePage      *ePage       = &avm->page[avm->current_page];

	if (check_FXAA_frame() == 1)
	{
		FBO_t* fbo_fxaa = &autosys.frame_fbo[FBO_FXAA];
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_fxaa->framebuffer);
		{
			glEnable(GL_DEPTH_TEST);
			glClearColor(0.0,0.0,0.0,0.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			FXAA_preDraw_VIEW(&avmUiInfo, &canBusAvm[NOW_CAN]);
#if save_FXAA_framebuffer
			if (frame == 10)
			{
				printf("------------------------------------------------------------\n");
				fwrite_readpixels(PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT, "FXAA_buffer_1.rgba");
				printf("current save FXAA_buffer_0 frame buffer\n");
			}
#endif
		}
#if 0
unsigned char* buffer = (unsigned char*)malloc(320 * 188 * 4);
if (buffer == NULL) {
    fprintf(stderr, "Failed to allocate memory for buffer.\n");
    return;
}
gettimeofday(&img_start, NULL);
glBindTexture(GL_TEXTURE_2D, FXAA_FBO.texture);
glReadPixels(0, 0, 320, 188, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
glBindTexture(GL_TEXTURE_2D, 0);
gettimeofday(&img_end, NULL);
float processtime = calculate_time(img_start, img_end); //(1/s)
AVM_LOGI("Get Image Time [%f]\n", processtime);
free(buffer);
#endif
		trans_FXAA_texture(fbo_fxaa->texture);
	}
#endif
}
static void generate_see_through_texture(can_bus_info_t canbus)
{
#if (PARAM_AVM_SEETHROUGH)
	camera_para_t *matrix = &autosys.system_info.projection.Projection_2Davm;
	set_seeThrough_fbo(fbo_2davm->texture, matrix->Rotate, paraSeeThrough.Rotate);
#endif
}
int through_process_count = 0;
int through_process_first = 0;
int init_see_alpha_value = 0;
static void generate_see_through_process_texture()
{
	if(autosys.seethrough.seethrough_finish == 1) return;
#if (PARAM_AVM_SEETHROUGH)

	glBindFramebuffer(GL_FRAMEBUFFER, seethroug_crope_FBO.framebuffer);
	{
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 64, 64);

		autosys_cal_bowl_shadow_range();
		draw_car_shadow_process_seethrough(NULL, autosys_get_bowl_shadow_range());
		glDisable(GL_BLEND);
	}

	glBindTexture(GL_TEXTURE_2D, seethroug_crope_FBO.texture);
	//glGenerateMipmap(GL_TEXTURE_2D);
	calculate_seethrough_prcess_value();
	glBindTexture(GL_TEXTURE_2D, 0);

	autosys.current_info.seethrough_process = get_seethrough_processValue();
	if (autosys.current_info.seethrough_process > 0.97f) 
	{
		autosys.seethrough.seethrough_finish = 1;
		autosys.current_info.seethrough_process = 1.0f;
	}


#if 0
	unsigned char textureData[4] = {0};

    glGetTexImage(GL_TEXTURE_2D, 6, GL_RGBA, GL_UNSIGNED_BYTE, &textureData);
	//glGetTexImage(GL_TEXTURE_2D, 6, GL_ALPHA, GL_UNSIGNED_BYTE, &textureData);
	#if 1
    if (textureData[0] > 0)
    {
        if (through_process_first == 0)
        {
            through_process_first = 1;
            init_see_alpha_value = textureData[0];
			autosys.current_info.seethrough_process = 0;
            AVM_LOGI("Current seethrough area %.1f%%\n", 0.0);
        }
        else
        {
            float delta = 255.0f - (float)textureData[0];
            float distance = 255.0f - (float)init_see_alpha_value;
            float process = (1.0f - (delta / distance)) * 100.0f;
            if (process > 95.0f) 
			{
				autosys.seethrough.seethrough_finish = 1;
				process = 100.0f;
			}
			autosys.current_info.seethrough_process = (int)(process * 10);
            AVM_LOGI("[%u] Current seethrough area %.1f%%\n", textureData[0], process);
        }
    }
#endif
    // 解除绑定
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

#endif
}
static void generate_2davm_texture()
{
#if (PARAM_AVM_SEETHROUGH || PARAM_AVM_SMOD)
	camera_para_t *matrix = &autosys.system_info.projection.Projection_2Davm;
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_2davm->framebuffer);
	{
		//glEnable(GL_DEPTH_TEST);
		glClearColor(autosys.shadow.color.X,autosys.shadow.color.Y,autosys.shadow.color.Z,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, AVM_2D_FBO_W, AVM_2D_FBO_H);
		draw_3d_avm(matrix->Rotate, AVM_2D, 0);
		//glDisable(GL_BLEND);
	}
	glClearColor(0.0f,0.0f,0.0f,0.0f);

#endif
}
static void generate_mod_texture(can_bus_info_t canbus)
{
#if PARAM_AVM_SMOD
    autosys_page *avm       = &autosys.avm_page;
	if(avm->current_page >= MAX_PAGE_NUM) return;
    ePage      *ePage       = &avm->page[avm->current_page];
	if (autosys.g_flag.fuc_smod == 1)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_y.framebuffer);
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, AVM_2D_FBO_W, AVM_2D_FBO_H);
			draw_rgba2y(fbo_2davm->texture, ePage->set.viewport[PAGE_2DAVM].W);
		}
		set_mod_start(&canbus);
		last2Dflag = 0;
	}
	else
	{
		reset_modflag();
	}
#endif
}
#if 1
/**
 * @author alfred
 * @date 2023/09/12
 * @brief ivi to contorl 360 rotate show
 * @param
 */
static void __IVI_SET_360ROTATE__(int _enable360system)
{
	g_enable_360system = _enable360system;
	if (g_reset_360rotateflag == 1)
	{
		g_reset_360rotateflag = 0;
		lock_page_roate = 0;
	}
}
void static_car_signal_process()
{
	CAR_MODEL_INFO *car_info = &autosys.current_info.carmodel_info;
	if (car_info->nextPage != car_info->currentPage)
	{
		bool isCurrentSetChosenPage = true;
		for (int ix = 0; ix < car_info->Pages_key_num; ix++)
		{
			if (car_info->nextPage == car_info->Pages_Chosen[ix])
			{
				car_info->freeze_frame_count = 1;
				car_info->enable_draw = 1;
				car_info->f_record = 1;
				car_info->delay_count = 0;
				isCurrentSetChosenPage = false;
				break;
			}
		}
		if (isCurrentSetChosenPage)
		{
			for (int ix = 0; ix < car_info->Pages_key_num; ix++)
			{
				if (car_info->currentPage == car_info->Pages_Chosen[ix])
				{
					car_info->freeze_frame_count = 0;
					car_info->f_record = 0;
					car_info->Triger_Regester = 0;
					car_info->Triger_Destory  = 1;
					car_info->enable_draw = 0;
					car_info->delay_count = 0;
					break;
				}
			}
		}
		car_info->currentPage = car_info->nextPage;
	}
}

void static_car_signal_colorChange()
{
	CAR_MODEL_INFO *car_info       = &autosys.current_info.carmodel_info;

	for (int ix = 0; ix < car_info->Pages_key_num; ix++)
	{
		if (car_info->nextPage == car_info->Pages_Chosen[ix])
		{
			car_info->enable_draw = 1;
			car_info->f_record = 1;
			break;
		}
	}
}
// upload_newMODline();
/**
 * @author alfred
 * @date 2023/09/15
 * @brief ivi to contorl 360 rotate show
 * @param
 */
static bool __IVI_ISCHANGEPAGE__()
{
	bool changPage_flag = false;
	if (pre_avm_page != autosys.avm_page.current_page)
	{
		CAR_MODEL_INFO *car_info       = &autosys.current_info.carmodel_info;
		car_info->nextPage = autosys.avm_page.current_page;
		static_car_signal_process();
		changPage_flag = true;
		//if (eight.moving == 1)
		{
			eight.interrupt = 1;
			if(eight.busy == 1)
			{
				eight.cur_id = eight.net_id;
			}
			eight.busy = 0;
			eight.moving = 0;
		}
		/*freetouch reset*/
		init_parameterPanel();
		/*rotate reset*/
		init_carmodelRotate(pre_avm_page);
		//avm_ePage_singleConfig(EVSTable_path[0], autosys.avm_page.current_page);
		pre_avm_page = autosys.avm_page.current_page;
	}
	return changPage_flag;
}
/**
 * @author alfred
 * @date 2023/09/26
 * @brief ivi to contorl carModel color
 * @param
 */
int m_currentColorIx = 0;
int m_isChange = 0;
static void __IVI_SET_CARMODELCOLOR__(int ix)
{
	CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;
	if (car_info->color_style != ix)
	{
		car_info->color_style = ix;
		m_isChange = 1;
	}

	if (m_isChange)
	{
		CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;
		car_info->color_style = ix;
		static_car_signal_colorChange();
		upload_selection_car_color(ix);
		m_isChange = 0;
	}
}
/**
 * @author alfred
 * @date 2023/10/04
 * @brief ivi to contorl carModel open/close door
 * @param
 */
int m_currentStatusIx = 0;
int m_isStatusChange = 0;
static void __IVI_SET_CARMODEL_STATUS(autosys_avm_info* info)
{
	int target = 0;

	target |= (info->car_model_door.left_front_car_door << 0);
	target |= (info->car_model_door.left_rear_car_door << 1);
	target |= (info->car_model_door.right_front_car_door << 2);
	target |= (info->car_model_door.right_rear_car_door << 3);
	target |= (info->car_model_door.trunk_car_door << 4);

	if (m_currentStatusIx != target)
	{
		m_currentStatusIx = target;
		m_isStatusChange = 1;
	}

	if (m_isStatusChange)
	{
		CAR_MODEL_INFO *carinfo = &autosys.current_info.carmodel_info;
		for(int ix = 0; ix < 5; ix++)
		{
			carinfo->car2d_status[ix] = (target & (0x01 << ix)) ? 1 : 0;
			carinfo->car3d_status[ix] = (target & (0x01 << ix)) ? 1 : 0;
		}
		upload_selection_car_color(carinfo->color_style);
		m_isStatusChange = 0;
	}
}

/**
 * @author alfred
 * @date 2023/10/24
 * @brief ivi to contorl Eight touch
 * @param
 */

int pre_set_eighttouch_index = -1;

static void __IVI_SET_EIGHTTOUCH__(int target)
{
	autosys_page *avm       = &autosys.avm_page;
    ePage      *ePage       = &avm->page[avm->current_page];
	if (ePage->set.touch.eight.enable != 1.0f)
		return;

	if (eight.cur_id != target && eight.level == 0)
	{
		pre_set_eighttouch_index = target;
		eight.level++;
		if (eight.busy != 1)
		{
			int clockwiseDistance = target - eight.cur_id;
			if (clockwiseDistance < 0)
				clockwiseDistance += 8;
			int counterClockwiseDistance = 8 - clockwiseDistance;

			eight.director = (clockwiseDistance < counterClockwiseDistance) ? 1 : 0;
			eight.busy = 1;
			eight.net_id = target;
			int step = abs(eight.cur_id - target);
			eight.block = (step > 4) ? (8 - step) : step;
			eight.processTime = (ePage->set.touch.eight.processTime / 8) * eight.block;
			// debug
			AVM_LOGI("----------------------------------------\n");
			AVM_LOGI("current EightTouch position [%d]\n", eight.cur_id);
			AVM_LOGI("next EightTouch position [%d]\n", eight.net_id);
			AVM_LOGI("director is [%d]\n", eight.director);
			AVM_LOGI("ORIG Process Time [%f]\n", ePage->set.touch.eight.processTime);
			AVM_LOGI("real Process Time [%f]\n", eight.processTime);
			AVM_LOGI("-----------------------------------------\n");
		}
	}
	else if (eight.level >= 1 && eight.busy == 1)
	{
		if(pre_set_eighttouch_index != target)
		{
			pre_set_eighttouch_index = target;
			eight.net_id             = target;
			eight.level = 2;
			eight.signal = 1;
		}
	}
	else
	{
		eight.busy = 0;
	}
}

/**
 * @author alfred
 * @date 2023/10/24
 * @brief ivi to contorl Eight touch
 * @param
 */


static void __IVI_SET_MUTITOUCH__(int target)
{
	autosys_page *avm       = &autosys.avm_page;
    ePage      *ePage       = &avm->page[avm->current_page];
	autosys.current_info.mutiple_touch_index = target;
	if(target < 8)
	{
		__IVI_SET_EIGHTTOUCH__(target);
	}
}

/**
 * @author alfred
 * @date 2024/05/16
 * @brief ivi to contorl BB PID Latency
 * @param
 */
static int BB_LOG_COUNT = 0;
static void __IVI_SET_AVM_BB_PID__(int Latency)
{
	if (Latency < 1)
	{
		if (BB_LOG_COUNT % 60 == 0)
		{
			// AVM_LOGI("\033[31m AVM BB PID is setting fault\033[0m\n");
		}
		return;
	}

	if (Latency > 40)
	{
		if (BB_LOG_COUNT++ % 60 == 0)
		{
			// AVM_LOGI("\033[31m AVM BB PID is setting fault\033[0m\n");
			// AVM_LOGI("\033[31m The PID range is 1 - 40 \033[0m\n");
		}
		return;
	}

	if (Latency != g_avm_BB_PID_Latency)
	{
		autosys.current_info.BB_PID_Latency = Latency;
		g_avm_BB_PID_Latency = Latency;
		AVM_PID_GROUP_DATA_Clear();
		AVM_LOGI(" AVM BB PID setting OK [PID is %d]\n", g_avm_BB_PID_Latency);
	}
}

void init_carmodelRotate(int page)
{
	flag_360_finsih = 1;
	g_CarModel_ever_do_rotate_ones[page] = 1;
}

/**
 * @author alfred
 * @date 2024/01/22
 * @brief ivi to reset 540 rotate show
 * @param
 */
int ivi_control_notify_reset540_OK = 0;
static void __IVI_RESET_540ROTATE__(int _reset)
{

	if (_reset == 1 && ivi_control_notify_reset540_OK == 0)
	{
		g_reset_540sysyem = _reset;
		ivi_control_notify_reset540_OK = 1;
		AVM_LOGI("IVI call avm 540 rotate reset\n");
	}
	else if (g_reset_540sysyem == 0 && ivi_control_notify_reset540_OK == 1)
	{
		ivi_control_notify_reset540_OK = 0;
		AVM_LOGI("AVM call avm 540 rotate reset OK\n");
		__AVM_CALL_CONTORL_IVI__(6, 0);
	}
}

void init_static_car_fbo()
{
	Point Resolution[MAX_STATIC_CAR_FBO_NUM] = {0};
	autosys_page *avm              = &autosys.avm_page;
	CAR_MODEL_INFO * car_info      = &autosys.current_info.carmodel_info;
	int create_fbo_id = 0;
	for(int ix = 0; ix < MAX_PAGE_NUM; ix++)
	{
		bool Serch_exist = false;
		ePage      *ePage          = &avm->page[ix];
		if(car_info->Pages_key[ix] == 1)
		{
			int curSize_W        = ePage->set.viewport[PAGE_3DAVM].W * PARAM_FXAA_LOSS_PANEL_VOLUME;
			int curSize_H        = ePage->set.viewport[PAGE_3DAVM].H * PARAM_FXAA_LOSS_PANEL_VOLUME;
			for(int id = 0; id < create_fbo_id; id++)
			{
				if(Resolution[id].X == curSize_W && Resolution[id].Y == curSize_H)
				{
					car_info->Pages_key[ix] = id + 1;
					Serch_exist = true;
					break;
				}
			}

			if(!Serch_exist)
			{
				if (create_fbo_id >= MAX_STATIC_CAR_FBO_NUM) 
				{
					AVM_LOGI("Exceed the maximum size!! in Static Car FBO\n");
					car_info->Pages_key[ix] = 0;
    				continue;
				}
				create_fbo_id++;
				car_info->Pages_key[ix] = create_fbo_id;
				Resolution[create_fbo_id - 1].X = curSize_W;
				Resolution[create_fbo_id - 1].Y = curSize_H;
				create_fbo(&autosys.frame_fbo[FBO_STATIC_CAR_MODEL_0 + create_fbo_id - 1], curSize_W, curSize_H, UI_TEST1_enable, GL_LINEAR);
			}
		}
	}
}


#endif

struct timeval start1, end1;

void *read_UI_line_func(void *ptr)
{
	//read_file_PGL_VIEW();
	Init_DISPLAY_PGL_PARAMETER();
	pthread_exit(NULL);
}

void *read_file_vec_func(void *ptr)
{
	int initialCount = 0;
	read_file_vec(initialCount);
	pthread_exit(NULL);
}

void *read_file_vec_2d_func(void *ptr)
{
	int initialCount = 0;
	read_file_vec_2d(initialCount);
	pthread_exit(NULL);
}

void *read_file_PGL_VIEW_func(void *ptr)
{
	pthread_exit(NULL);
}


void avm_check_file_tree_exist()
{
	/**
	 * check in any avm file
	 */

	AVM_LOGI("[DEBUG]-------------------------Check AVM File Tree--------------------------------- \n");

	int flash_key_grope[] =
	{
		7, // avm folder
		8, // golden folder
	};
	char flash_tree_grope[][128] = 
	{
		"EVSTable/avm/obj_0.bin",
		"EVSTable/avm/obj_1.bin",
		"EVSTable/avm/obj_2.bin",
		"EVSTable/avm/obj_3.bin",
		//"EVSTable/avm/colorOut1.png",
		"EVSTable/avm/avm3Dcolor.bin",
		"EVSTable/avm/avm2d_limit.bin",
		//"EVSTable/avm/alpha.png",
		"EVSTable/golden/goldensample_result.bin",
	};

	enum flash_key_grope
	{
		avm_folder        = 8,
		config_folder     = avm_folder + 25,
		input_img_folder  = config_folder + 9,
		sceneView_folder  = input_img_folder + 4,
		pgl_folder        = sceneView_folder + 14,
		carmodel_folder   = pgl_folder + 18,
		golden_folder     = carmodel_folder + 5,
	};
	char app_tree_grope[][128] = 
	{
		"EVSTable/avm/obj_0.bin",
		"EVSTable/avm/obj_1.bin",
		"EVSTable/avm/obj_2.bin",
		"EVSTable/avm/obj_3.bin",
		"EVSTable/avm/colorOut1.png",
		"EVSTable/avm/avm3Dcolor.bin",
		"EVSTable/avm/avm2d_limit.bin",
		"EVSTable/avm/alpha.png",

		"EVSTable/config/system.config",
		"EVSTable/config/Bowl_setting/bowlsetting.config",
		"EVSTable/config/Calbi_setting/SetPara.config",
		"EVSTable/config/Calbi_setting/parall/User_ROI.config",
		"EVSTable/config/Calbi_setting/rect/User_ROI.config",
		"EVSTable/config/Page_setting/ePage0.bin",
		"EVSTable/config/Page_setting/ePage1.bin",
		"EVSTable/config/Page_setting/ePage2.bin",
		"EVSTable/config/Page_setting/ePage3.bin",
		"EVSTable/config/Page_setting/ePage4.bin",
		"EVSTable/config/Page_setting/ePage5.bin",
		"EVSTable/config/Page_setting/ePage6.bin",
		"EVSTable/config/Page_setting/ePage7.bin",
		"EVSTable/config/Page_setting/ePage8.bin",
		"EVSTable/config/Page_setting/ePage9.bin",
		"EVSTable/config/Page_setting/ePage10.bin",
		"EVSTable/config/Page_setting/ePage11.bin",
		"EVSTable/config/Page_setting/ePage12.bin",
		"EVSTable/config/Page_setting/ePage13.bin",
		"EVSTable/config/Page_setting/ePage14.bin",
		"EVSTable/config/Page_setting/ePage15.bin",
		"EVSTable/config/Page_setting/ePage16.bin",
		"EVSTable/config/Page_setting/ePage17.bin",
		"EVSTable/config/Page_setting/ePage18.bin",
		"EVSTable/config/Page_setting/ePage19.bin",
		"EVSTable/config/Page_setting/ePage20.bin",
		"EVSTable/config/Page_setting/ePage21.bin",
		"EVSTable/config/Page_setting/ePage22.bin",
		"EVSTable/config/Page_setting/ePage23.bin",
		"EVSTable/config/Page_setting/ePage24.bin",
		"EVSTable/config/Page_setting/ePage25.bin",
		"EVSTable/config/Page_setting/ePage26.bin",
		"EVSTable/config/Page_setting/ePage27.bin",
		"EVSTable/config/Page_setting/ePage28.bin",
		"EVSTable/config/Page_setting/ePage29.bin",

		"EVSTable/input_img/fish.txt",
		"EVSTable/input_img/front.txt",
		"EVSTable/input_img/back.txt",
		"EVSTable/input_img/left.txt",
		"EVSTable/input_img/right.txt",
		"EVSTable/input_img/front.png",
		"EVSTable/input_img/back.png",
		"EVSTable/input_img/left.png",
		"EVSTable/input_img/right.png",

		"EVSTable/sceneView/obj_f.bin",
		"EVSTable/sceneView/obj_b.bin",
		"EVSTable/sceneView/obj_l.bin",
		"EVSTable/sceneView/obj_r.bin",

		"EVSTable/pgl/makeline_matrial/Front_homoMatrixCoorToPixel.bin",
		"EVSTable/pgl/makeline_matrial/Front_homoMatrixPosition.bin",
		"EVSTable/pgl/makeline_matrial/Left_homoMatrixCoorToPixel.bin",
		"EVSTable/pgl/makeline_matrial/Right_homoMatrixCoorToPixel.bin",
		"EVSTable/pgl/makeline_matrial/homoMatrixCoorToPixel.bin",
		"EVSTable/pgl/makeline_matrial/homoMatrixPosition.bin",

		"EVSTable/pgl/Dynamic/PARA1/carinformation.bin",
		"EVSTable/pgl/Dynamic/PARA1/front.bin",
		"EVSTable/pgl/Dynamic/PARA1/rear.bin",
		"EVSTable/pgl/Dynamic/PARA2/carinformation.bin",
		"EVSTable/pgl/Dynamic/PARA2/front.bin",
		"EVSTable/pgl/Dynamic/PARA2/rear.bin",
		"EVSTable/pgl/Dynamic/PARA3/fisheye_static_rawpixel.bin",
		"EVSTable/pgl/Dynamic/PARA3/fisheye_static_undistpixel.bin",


		"EVSTable/car_model/CarModel.bin",
		"EVSTable/car_model/car_2d.png",
		"EVSTable/car_model/vehicleSurface_Color.png",
		"EVSTable/car_model/glassSurface_Color.png",
		"EVSTable/car_model/wheelSurface_Color.png",
		"EVSTable/car_model/FrontLamp_off_01.png",
		"EVSTable/car_model/RearLamp_off_01.png",
		"EVSTable/car_model/SideMirrorLamp_off_01.png",
		"EVSTable/car_model/shadow/Car_shadow_area.obj",
		"EVSTable/car_model/shadow/shadow.bin",

		"EVSTable/car_model/skybox/back.png",
		"EVSTable/car_model/skybox/front.png",
		"EVSTable/car_model/skybox/left.png",
		"EVSTable/car_model/skybox/right.png",
		"EVSTable/car_model/skybox/bottom.png",
		"EVSTable/car_model/skybox/top.png",

		"EVSTable/car_model/color_style/1/car_2d.png",
		"EVSTable/car_model/color_style/1/vehicleSurface_Color.png",

		"EVSTable/golden/goldensample_result.bin",
		"EVSTable/golden/goldensample_Front_homoMatrixCoorToPixel.bin",
		"EVSTable/golden/goldensample_Front_homoMatrixPosition.bin",
		"EVSTable/golden/goldensample_Back_homoMatrixCoorToPixel.bin",
		"EVSTable/golden/goldensample_Back_homoMatrixPosition.bin",
	};

	char tmp_tree_grope[][128] = 
	{
		"EVSTable/input_img/fish.txt",
		"EVSTable/input_img/front.txt",
		"EVSTable/input_img/back.txt",
		"EVSTable/input_img/left.txt",
		"EVSTable/input_img/right.txt",
		"EVSTable/input_img/front.png",
		"EVSTable/input_img/back.png",
		"EVSTable/input_img/left.png",
		"EVSTable/input_img/right.png",
	};
	char path[512];
	int exist_cnt = 0;
	int abnormal_cnt= 0;
	int checkitemSize = sizeof(flash_tree_grope) / sizeof(flash_tree_grope[0]);
	for(int idx = 0; idx < checkitemSize; idx++)
	{
		sprintf(path, "%s%s",EVSTable_path[1], flash_tree_grope[idx]);

		// if(strstr(flash_tree_grope[idx], "png") != NULL)
		// {
		// 	flag_flash_data_exist++;
		// 	continue;
		// }

		if (access(path, F_OK) == 0)
		{
			exist_cnt++;
			struct stat statbuf;
			stat(path, &statbuf);
			if(statbuf.st_size <= 0)
			{
				abnormal_cnt++;
				g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_FLASH);
				AVM_LOGI("[DEBUG][Path_1] File exists but size is 0: %s\n", path);
				if(idx < 8)
				{
					g_calibration_info.flash_info.avm_file |= DATA_EMPTY;
				}

			}
			//g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, CALIBRATION);
		}
		else
		{
			AVM_LOGI("[DEBUG][Path_1] File does not exist: %s\n", path);
		}
	}

	if (exist_cnt == 0)
	{
		g_calibration_info.status =
			UPDATE_CALIB_STATUS(g_calibration_info.status, NON_CALIBRATION);
	}
	else if (exist_cnt == checkitemSize && abnormal_cnt == 0)
	{
		g_calibration_info.status =
			UPDATE_CALIB_STATUS(g_calibration_info.status, CALIBRATION);
	}
	else
	{
		g_calibration_info.status =
			UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_FLASH);
	}

	for(int idx = 0; idx < sizeof(app_tree_grope) / sizeof(app_tree_grope[0]); idx++)
	{
		sprintf(path, "%s%s",EVSTable_path[0], app_tree_grope[idx]);
		if (access(path, F_OK) == 0)
		{
			struct stat statbuf;
			stat(path, &statbuf);
			if(statbuf.st_size <= 0)
			{
				if(idx < avm_folder)
				{
					g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_APP);
					g_calibration_info.flag_emergency = true;
					g_calibration_info.app_info.avm_file |= DATA_EMPTY;
				}
				AVM_LOGI("[DEBUG][Path_0] File exists but size is 0: %s\n", path);
			}
		}
		else
		{
			if(idx < avm_folder)
			{
				g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_APP);
				g_calibration_info.flag_emergency = true;
				g_calibration_info.app_info.avm_file |= DATA_NO_FIND;
			}
			AVM_LOGI("[DEBUG][Path_0] File does not exist: %s\n", path);
		}
	}

	for(int idx = 0; idx < sizeof(tmp_tree_grope) / sizeof(tmp_tree_grope[0]); idx++)
	{
		sprintf(path, "%s%s",EVSTable_path[0], tmp_tree_grope[idx]);
		if (access(path, F_OK) == 0)
		{
			struct stat statbuf;
			stat(path, &statbuf);
			if(statbuf.st_size <= 0)
			{
				AVM_LOGI("[DEBUG][Path_2] File exists but size is 0: %s\n%s\n", path);
			}
		}
		else
		{
			AVM_LOGI("[DEBUG][Path_2] File does not exist: %s\n", path);
		}
	}
	AVM_LOGI("[DEBUG]-------------------------Check AVM File Tree--------------------------------- \n");
}

void read_avm_sysyem_file(const char dir[])
{
	__IVI_Calibration_Status_Init__();

	autosys_current_info * cur_info = &autosys.current_info;	
	CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;	
	PARA_BLUR * blur_info     = &autosys.system_info.blur_info;	
	int * mod_info = &autosys.system_info.mod_version;
	FILE *fp_EVSTable;
	size_t len = 0;
	ssize_t read;
	int read_EVStable_path_count = 0;
	fp_EVSTable = fopen(dir, "r");
	if (fp_EVSTable == NULL)
	{
		AVM_LOGI(LOG_RED("[ERROR]")"EVStable_path.config Load error!!!\n");
		AVM_LOGI("[WARNING] Using EVStable_path default path!!!\n");
		char * path0 = EVSTable_Path_0;
		char * path1 = EVSTable_Path_1;
		char * path2 = EVSTable_Path_2;
		EVSTable_path[0] = path0;
		EVSTable_path[1] = path1;
		EVSTable_path[2] = path2;
	}
	else
	{
		while ((getline(&EVSTable_path[read_EVStable_path_count], &len, fp_EVSTable)) != -1)
		{
			char *temp1 = EVSTable_path[read_EVStable_path_count];
			temp1[strcspn(EVSTable_path[read_EVStable_path_count], "\n\r")] = 0;
			printf("EVSTable_path[%d] = %s!!!!!!!!qt\n", read_EVStable_path_count, EVSTable_path[read_EVStable_path_count]);
			read_EVStable_path_count++;
			len = 0;
		}
		fclose(fp_EVSTable);
	}

	int screen_width = 0, screen_height = 0, max_page_number = 0;
    int one_action_key[4];
	int blur_minimap_enable = 0;
    int blur_enable_list[4];
    int blur_level_list[4];
    float blur_mask_list[4][4];
    int max_carmodel_number, max_carmodel_color, init_carmodel_Color;
    int max_static_line, max_static_distortion_line, max_pgl_style, init_pgl_style;
	int car_alpha_mode[4];
	int init_mod_version = 0;
	int BB_version = 0;
	Point IMGSize;
	char path[1024];
	FILE *file = fp_source_app("EVSTable/config/system.config", "r");
	if (file == NULL)
	{
		AVM_LOGI("[WARNING] Using System default value!!!\n");
		screen_width        = HDwidth;
		screen_height       = HDheight;
		max_page_number     = max_ViewPage;
		init_carmodel_Color = 0;
		g_calibration_info.status = UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_APP);
		g_calibration_info.flag_emergency = true;
		g_calibration_info.app_info.config |= DATA_NONFIND_OR_EMPTY;
		g_calibration_info.call_back_calibration_status(&g_calibration_info);
	}
	else
	{
		char line[256];
		while (fgets(line, sizeof(line), file))
		{

			if (line[0] == '/')
				continue;

			if (sscanf(line, "screen_width = [%d]", &screen_width) == 1)
				continue;
			if (sscanf(line, "screen_height = [%d]", &screen_height) == 1)
				continue;
			if (sscanf(line, "max_page_number = [%d]", &max_page_number) == 1)
				continue;

			if (strstr(line, "one_action_key") != NULL)
			{
				int count = 1; // Start from 1 as there's at least one number
				for (char *c = line; *c != '\0'; c++)
				{
					if (*c == ',')
						count++;
				}

				int *one_action_key = malloc(count * sizeof(int));
				if (one_action_key == NULL)
				{
					return;
				}

				char *ptr = strchr(line, '[') + 1;
				for (int i = 0; i < count; i++)
				{
					if (i < count - 1)
					{
						sscanf(ptr, "%d,", &one_action_key[i]);
					}
					else
					{
						sscanf(ptr, "%d]", &one_action_key[i]);
					}
					ptr = strchr(ptr, ',') + 1;
				}

				// Use the array as needed
				for (int i = 0; i < count; i++)
				{
					AVM_LOGI("one_action_key[%d] = %d\n", i, one_action_key[i]);
					autosys.avm_page.page[one_action_key[i]].set.animation.para2.only_one_key = 1;
					// MODE_VIEW[one_action_key[i]].animation.para.only_one_key = 1;
				}

				free(one_action_key);
			}
			// if (sscanf(line, "one_action_key = [%d,%d,%d,%d]", &one_action_key[0], &one_action_key[1]
			// , &one_action_key[2], &one_action_key[3]) == 4) continue;
			if (sscanf(line, "max_carmodel_number = [%d]", &max_carmodel_number) == 1)
				continue;
			if (sscanf(line, "max_carmodel_color = [%d]", &max_carmodel_color) == 1)
				continue;
			if (sscanf(line, "init_carmodel_Color = [%d]", &init_carmodel_Color) == 1)
				continue;
			if (sscanf(line, "max_static_line = [%d]", &max_static_line) == 1)
				continue;
			if (sscanf(line, "max_static_distortion_line = [%d]", &max_static_distortion_line) == 1)
				continue;
			if (sscanf(line, "max_pgl_style = [%d]", &max_pgl_style) == 1)
				continue;
			if (sscanf(line, "init_pgl_style = [%d]", &init_pgl_style) == 1)
				continue;
			if (sscanf(line, "mod_version_type = [%d]", &init_mod_version) == 1)
				continue;
			if (sscanf(line, "blur_minimap = [%d]", &blur_minimap_enable) == 1)
				continue;
			if (sscanf(line, "blur_enable = [%d,%d,%d,%d]", &blur_enable_list[0], &blur_enable_list[1], &blur_enable_list[2], &blur_enable_list[3]) == 1)
				continue;
			if (sscanf(line, "blur_gaussian_level = [%d,%d,%d,%d]", &blur_level_list[0], &blur_level_list[1], &blur_level_list[2], &blur_level_list[3]) == 1)
				continue;
			if (sscanf(line, "blur_mask_0 = [%f,%f,%f,%f]", &blur_mask_list[0][0], &blur_mask_list[0][1], &blur_mask_list[0][2], &blur_mask_list[0][3]) == 1)
				continue;
			if (sscanf(line, "blur_mask_1 = [%f,%f,%f,%f]", &blur_mask_list[1][0], &blur_mask_list[1][1], &blur_mask_list[1][2], &blur_mask_list[1][3]) == 1)
				continue;
			if (sscanf(line, "blur_mask_2 = [%f,%f,%f,%f]", &blur_mask_list[2][0], &blur_mask_list[2][1], &blur_mask_list[2][2], &blur_mask_list[2][3]) == 1)
				continue;
			if (sscanf(line, "blur_mask_3 = [%f,%f,%f,%f]", &blur_mask_list[3][0], &blur_mask_list[3][1], &blur_mask_list[3][2], &blur_mask_list[3][3]) == 1)
				continue;
			if (sscanf(line, "img_width = [%d]", &IMGSize.X) == 1)
				continue;
			if (sscanf(line, "img_height = [%d]", &IMGSize.Y) == 1)
				continue;
			if (sscanf(line, "CAR_ALPHA_MODE = [%d,%d,%d,%d]", &car_alpha_mode[0], &car_alpha_mode[1], &car_alpha_mode[2], &car_alpha_mode[3]) == 1)
				continue;
			if (sscanf(line, "BB_VERSION = [%d]", &BB_version) == 1)
				continue;
		}
		fclose(file);
	}
	__AUTOSYS_CHECK_VERSION_STATUS__();


	// Use the read variables as needed
	AVM_LOGI("---------------------------------------\n");
	AVM_LOGI("[system] system file: init_carmodel_Color:  [%d]\n", init_carmodel_Color);
	AVM_LOGI("[system] system file: init_Panel resoultion [%d x %d]\n", screen_width, screen_height);
	AVM_LOGI("[system] system file: MOD version type [%d]\n", init_mod_version);
	AVM_LOGI("[system] system file: BB version type [%d]\n", BB_version);
	AVM_LOGI("---------------------------------------\n");
	car_info->color_style = init_carmodel_Color;
	cur_info->BB_VERSION  = BB_version;
	g_PANEL_WIDTH = screen_width;
	g_PANEL_HEIGHT = screen_height;
	memcpy(blur_info->blur_enable, blur_enable_list, sizeof(int) * 4);
	memcpy(blur_info->blur_level, blur_level_list, sizeof(int) * 4);
	memcpy(blur_info->blur_mask, blur_mask_list, sizeof(float) * 16);
	memcpy(blur_info->blur_mask, blur_mask_list, sizeof(float) * 16);
	memcpy(autosys.system_info.car_alpha_mode, car_alpha_mode, sizeof(int) * 4);
	blur_info->minimap_enable = blur_minimap_enable;
	autosys.system_info.IMGSize = IMGSize;
	*mod_info = init_mod_version;
	avm_check_file_tree_exist();
	g_calibration_info.call_back_calibration_status(&g_calibration_info);
	autosys.system_info.seethrough_resolution_level   = 1.5f;
	tmp_seethrough *seethroughbuffer = &autosys.seethrough;
	seethroughbuffer->tune_scale = 1.0f;
	seethroughbuffer->tune_shift = 0.0f;
}

void read_dynamic_line_file()
{
	read_dynamic_line_user_data();
}

void device_opengl_list()
{
	GLint maxTextureUnits;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
	AVM_LOGI("---------------------------------------\n");
	AVM_LOGI("[***] opengl : max texure img units: [%d]\n", maxTextureUnits);
	AVM_LOGI("---------------------------------------\n");
}

static void init_para(void)
{
	Initial_autosys_avm_package(EVSTable_path[0]);
	avm_ePage_Init();
	// gettimeofday(&start1, NULL);
	init_avmUiInfo(&avmUiInfo);
	// gettimeofday(&end1, NULL);
	// printf("init_avmUiInfo = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);
	int initialCount = 0;
	init_camera_para();
	// gettimeofday(&end1, NULL);
	// printf("init_camera_para = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);
	read_2Dlimit(avm_2d, &carSizeW, &carSizeH);
	carSizeWScale = carSizeW / (carSizeW + AVM_2D_SCOPE_W); // 0.31;
	carSizeHScale = carSizeH / (carSizeH + AVM_2D_SCOPE_H); // 0.55;
															// gettimeofday(&end1, NULL);
															// printf("read_2Dlimit = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);
	init_2d_avm_parameter(&para2Davm, carSizeW, carSizeH);
	// gettimeofday(&end1, NULL);
	// printf("init_2d_avm_parameter = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);
	init_3d_avm_parameter(&para3Davm);
	init_3d_avm_parameter(&paraCorrectEye);
	// gettimeofday(&end1, NULL);
	// printf("init_3d_avm_parameter = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);
	init_seethrough_avm_parameter(&paraSeeThrough, &para2Davm);
	// gettimeofday(&end1, NULL);
	// printf("init_seethrough_avm_parameter = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);
	init_car_parameter(&paracar);
	// gettimeofday(&end1, NULL);
	// printf("init_car_parameter = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);
	main_mem_malloc();
	// gettimeofday(&end1, NULL);
	// printf("main_mem_malloc = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);
	read_homo_matrix();
	// gettimeofday(&end1, NULL);
	// printf("read_homo_matrix = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start1, NULL);

	pthread_t p1 = 0, p2 = 0, p3 = 0, p4 = 0;
	pthread_create(&p1, NULL, read_UI_line_func, NULL);		// 建立子執行緒
	pthread_create(&p2, NULL, read_file_vec_func, NULL);	// 建立子執行緒
	pthread_create(&p3, NULL, read_file_vec_2d_func, NULL); // 建立子執行緒
															// pthread_create(&p4, NULL, read_file_PGL_VIEW_func, NULL); // 建立子執行緒
	open_gl_init_car();
	read_pgl_colorbar();

#ifdef READ_FILE
	read_camera_file();
#endif
	alloc_cam_one_buffer(); 
	// gettimeofday(&start1, NULL);
	// printf("start mod_init!! \n");
	mod_init(carSizeWScale, carSizeHScale, AVM_2D_FBO_W, AVM_2D_FBO_H);
	seethrough_process_init(64,64);
	// gettimeofday(&end1, NULL);
	// printf("mod_init = %f\n", calculate_time(start1, end1));

	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	pthread_join(p3, NULL);
	// read_sysyem_file();
	read_dynamic_line_file();
	read_static_line_user_data();
	//read_static_scene_line_user_data();
	read_calib_fish_para();
	init_current_3davm_fov();
	// device_opengl_list();
	open_gl_2dcarModel_parameter(autosys.avm_page.page[0].contexts.avm_2d_car.axis_density);
	init_BrignessBlance();
	read_vignette_table();
	// pthread_join(p4,NULL);
	// BSD_init();
}

static void init_gl_avm(void)
{
	AVM_LOGI("---------------------------------------\n");
	init_main_program();
	init_car_program();
	init_reverse_program();
	init_2davm_program();
	init_seethrough_program();
	init_mod_program();
	init_blur_program();
	init_rgba2y_program();
	init_FXAA_program();
	AVM_LOGI("---------------------------------------\n");
}

static void gen_texture_buf(void)
{
	AVM_LOGI("---------------------------------------\n");
	open_gl_init_avm_2d();
	open_gl_init_3davm_cam2d();
	open_gl_init_rgba();
	open_gl_init_reverse_line();
	open_gl_init_mod_line();
	open_gl_init_blur();
	open_gl_init_rgba2y();
	avm_fbo = 0;
	open_gl_init_seethrough(para2Davm.inPara.horiFov, para2Davm.inPara.vertiFov);
	fbo_2davm = open_gl_init_2davm_fbo(AVM_2D_FBO_W, AVM_2D_FBO_H);
	create_fbo(&fbo_y, AVM_2D_FBO_W, AVM_2D_FBO_H, RGBA_2_Y, GL_NEAREST);
	create_fbo(&seethroug_crope_FBO, 64, 64, SEE_THROUGH_CROPE, GL_LINEAR);
	

#if FXAA
	FBO_t* fbo_fxaa = &autosys.frame_fbo[FBO_FXAA];
	create_fbo(fbo_fxaa, PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT, FXAA_TEXTURE, GL_LINEAR);
	init_FXAA_opengl();
	//autosys.frame_fbo[FBO_FXAA].depthRenderbuffer = FXAA_FBO.depthRenderbuffer;
#endif

#if PARAM_OPENGL_OIT_BUFFER_FBO_ON

#if PARAM_CARMODEL_OIT_SHARE_DEPTH_BUFFER
	FBO_t* fbo_4to1 = &autosys.frame_fbo[FBO_4TO1];
	create_fbo_OIT(&carmodel_fbo, (int)(PARAM_FXAA_PANEL_WIDTH), (int)(PARAM_FXAA_PANEL_HEIGHT), CAR_MODEL_OIT, GL_LINEAR, 1, autosys.frame_fbo[FBO_FXAA].depthRenderbuffer);
#else
	create_fbo_OIT(&carmodel_fbo, (int)(g_PANEL_WIDTH * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME), (int)(g_PANEL_HEIGHT * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME), CAR_MODEL_OIT, GL_LINEAR, 0, 0);
#endif
	set_OIT_Idx(&carmodel_fbo);
#endif


	//seethroug_view_FBO
	// printf("create_fbo = %f\n", calculate_time(start1, end1));

#if PARAM_FISHEYE_BLUR_FUNCTION 
//GL_LINEAR_MIPMAP_LINEAR
	create_fbo_BLUR(&autosys.frame_fbo[FBO_BLUR_TMP], 
	autosys.system_info.IMGSize.X * PARAM_FISHEYE_BLUR_LOSS_PANEL_W_VOLUME, 
	autosys.system_info.IMGSize.Y * PARAM_FISHEYE_BLUR_LOSS_PANEL_H_VOLUME, 
	FXAA_TEXTURE, GL_LINEAR_MIPMAP_LINEAR);
	create_fbo_BLUR(&autosys.frame_fbo[FBO_BLUR_1],   
	autosys.system_info.IMGSize.X * PARAM_FISHEYE_BLUR_LOSS_PANEL_W_VOLUME, 
	autosys.system_info.IMGSize.Y * PARAM_FISHEYE_BLUR_LOSS_PANEL_H_VOLUME,
	FXAA_TEXTURE, GL_LINEAR_MIPMAP_LINEAR);
	create_fbo_BLUR(&autosys.frame_fbo[FBO_BLUR_2],   
	autosys.system_info.IMGSize.X * PARAM_FISHEYE_BLUR_LOSS_PANEL_W_VOLUME, 
	autosys.system_info.IMGSize.Y * PARAM_FISHEYE_BLUR_LOSS_PANEL_H_VOLUME,
	FXAA_TEXTURE, GL_LINEAR_MIPMAP_LINEAR);
	create_fbo_BLUR(&autosys.frame_fbo[FBO_BLUR_3],   
	autosys.system_info.IMGSize.X * PARAM_FISHEYE_BLUR_LOSS_PANEL_W_VOLUME, 
	autosys.system_info.IMGSize.Y * PARAM_FISHEYE_BLUR_LOSS_PANEL_H_VOLUME, 
	FXAA_TEXTURE, GL_LINEAR_MIPMAP_LINEAR);
	create_fbo_BLUR(&autosys.frame_fbo[FBO_BLUR_4],   
	autosys.system_info.IMGSize.X * PARAM_FISHEYE_BLUR_LOSS_PANEL_W_VOLUME, 
	autosys.system_info.IMGSize.Y * PARAM_FISHEYE_BLUR_LOSS_PANEL_H_VOLUME, 
	FXAA_TEXTURE, GL_LINEAR_MIPMAP_LINEAR);
#endif

	rotate_callculate_2D(&para2Davm);
	autosys_assgin_2Dprojection2core(&para2Davm);
	autosys_assgin_3Dprojection2core(&para3Davm);
	// gettimeofday(&end1, NULL);
	// printf("rotate_callculate_2D = %f\n", calculate_time(start1, end1));
	AVM_LOGI("---------------------------------------\n");
}

void init_avm(const char dir[])
{
	CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;	
	// gettimeofday(&start1, NULL);
	init_gl_avm();
	// gettimeofday(&end1, NULL);
	// printf("init_gl_avm = %f\n", calculate_time(start1, end1));

	// gettimeofday(&start2, NULL);
	init_para();
	// gettimeofday(&end2, NULL);
	// printf("init_para = %f\n", calculate_time(start2, end2));

	// gettimeofday(&start2, NULL);
	gen_texture_buf();
	// gettimeofday(&end2, NULL);
	// printf("gen_texture_buf = %f\n", calculate_time(start2, end2));

	if (car_info->color_style > 0)
		upload_selection_car_color(car_info->color_style);
	AVM_LOGI("DEBUG - init_avm\n");
	gettimeofday(&start360, NULL);
	gettimeofday(&start, NULL);

	g_calibration_info.flag_goingruntime = true;
	g_calibration_info.call_back_calibration_status(&g_calibration_info);
}
GLuint GL_PREVIOUS_FBO;
void panel_fbo(const GLuint Framebuf)
{
	avm_fbo = Framebuf;
	autosys.frame_fbo[FBO_4TO1].framebuffer = Framebuf;
}

void panel_depth(const GLuint Depthbuf)
{
	autosys.frame_fbo[FBO_4TO1].depthRenderbuffer = Depthbuf;
}
/**
 * @brief Copy the 4-to-1 panel FBO data to the provided GLuint array.
 * @param Data A GLuint array of size 3 to store the FBO data.
 *             Data[0] will store the value of framebuffer,
 *             Data[1] will store the value of texture,
 *             Data[2] will store the value of depthRenderbuffer.
 */
void get_panel_4to1_fbo(GLuint *Data)
{
	FBO_t* fbo = &autosys.frame_fbo[FBO_4TO1];
	fbo->framebuffer       = Data[0];
	fbo->texture           = Data[1];
	fbo->depthRenderbuffer = Data[2];
}
int testTran = 0;


/*alfred*/
float m_testtireangle = 0.0f;
float m_testCarSpeed = 50.0f;
struct timespec time2 = {0, 0};
int change_4in1_view_flag = 0;

void avm_panel_moveView()
{
	avm_panelTouch_moveView(avmUiInfo, &touchdata);
}
void sharedmemory()
{
	void *shmaddr = NULL;
	unsigned char data[] = "1";
	int shmid;
	key_t key = (key_t)667;
	long page_size = sysconf(_SC_PAGESIZE);
	int data_size = (1 + page_size - 1) & (~(page_size - 1));
	// printf("data size: %d, page size: %ld\n", data_size, page_size);

	// 1. create shared memory
	shmid = shmget(key, data_size, 0644 | IPC_CREAT);
	if (shmid == -1)
	{
		perror("shmget failed\n");
		exit(EXIT_FAILURE);
	}

	// 2. attach shared memory
	shmaddr = shmat(shmid, NULL, 0);
	if (shmaddr == (void *)-1)
	{
		perror("shmat failed\n");
		exit(EXIT_FAILURE);
	}

	// 3. write data to shared memory
	memset(shmaddr, 0, data_size);
	memcpy(shmaddr, &data, 1);

	// 4. detach shared memory
	if (shmdt(shmaddr) == -1)
	{
		perror("shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	// printf("write done !\n");
}
int screenshotflag = 1; // for take picture , to tool use.

static int testconuuu = 0;
struct can_frame
{
	int can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
	int data[11];
};
#define AVM_set_360_status 0x29
#define IVI_get_360_status 0x30

float dtime, beforeSpeed, meterageEach;
float meterage = 0.0;
double START, END;
double START1, END1;
pthread_t avm_color, free_touch_thread;

void *avm_color_func(void *ptr)
{
	set_trans_camera_color(&avmUiInfo, transCamera, 1);
	pthread_exit(NULL);
}

/**
 * @brief :  usr_get_current_Panel_resoultion
 * @param :  output panel resoultion;
 * @note : the value from avm system.config, please used this method after read_avm_system_file()
 * @author : Alfred
 * @date : 2023/11/21
 */
void usr_set_warning_init_img_position(const char dir[])
{
	int size = strlen(dir);
	ivi_warning.img_path = (char *)malloc(size * sizeof(char));
	strcpy(ivi_warning.img_path, dir);
}
void usr_get_current_panel_resolution(int *panel_width, int *panel_height)
{
	usr_set_warning_init_img_position("warning.png");
	usr_set_warning_init_Fisheye_position(0.9f, -0.9f, 0.1f, 0.1f);
	usr_set_warning_init_Top2D_positionF(0.5f, 0.0f, 0.1f, 0.1f);
	usr_set_warning_init_Top2D_positionB(-0.5f, 0.0f, 0.1f, 0.1f);
	usr_set_warning_init_Top2D_positionL(0.0f, -0.5f, 0.1f, 0.1f);
	usr_set_warning_init_Top2D_positionR(0.0f, 0.5f, 0.1f, 0.1f);

	if (panel_width == NULL || panel_height == NULL)
	{
		AVM_LOGI("set panel resoultion is error, because the buffer is NULL\n");
	}
	*panel_width = g_PANEL_WIDTH;
	*panel_height = g_PANEL_HEIGHT;
}

/**
 * @brief :  user_get_current_ratate_angle
 * @param :  rec is mean Mode, Mode(0) rotate theta(0-360); Mode(1) rotate phi (-56 - 56);
 * @note : the flow is following step:
 * 1. if Ontouch 2.calculator axis(touch) 3. call AxisOffsetTo2DAVM() to offset touch(axis) to carCentor(axis)
 * 4. call isAxisOutofCar() to determind mode 5. calculator angle, accroding to offset axis
 * @author : Alfred
 * @date : 2023/02/03
 */
void usr_get_current_rotate_angle(float *theta, float *phi, float *radiuseye)
{
	if (theta != NULL)
	{
		*theta = para3Davm.exPara.theta;
	}

	if (phi != NULL)
	{
		*phi = para3Davm.exPara.phi;
	}

	if (radiuseye != NULL)
	{
		*radiuseye = para3Davm.exPara.eyeRadius;
	}
}
/**
 * @brief :  user_set_current_alpha_value
 * @param :  rec is mean Mode, Mode(0) rotate theta(0-360); Mode(1) rotate phi (-56 - 56);
 * @note : the flow is following step:
 * 1. if Ontouch 2.calculator axis(touch) 3. call AxisOffsetTo2DAVM() to offset touch(axis) to carCentor(axis)
 * 4. call isAxisOutofCar() to determind mode 5. calculator angle, accroding to offset axis
 * @author : Alfred
 * @date : 2023/09/26
 */
void usr_set_current_3D_carModel_alpha(int alpha, int reset)
{
	g_3D_carModel_alpha_value = alpha;
	if (reset == 1)
		g_3D_carModel_alpha_value = 0;
}
// void usr_set_freetouch_limit(int page, int buff[4])
// {
// 	int cur_page = cam_ch;
// 	if (page != -1)
// 		cur_page = page;
// 	printf("\n\nset buff %d, %d,%d,%d,%d\n", cur_page, buff[0], buff[1], buff[2], buff[3]);
// 	memcpy(&MODE_VIEW[cur_page].touch.limit, buff, sizeof(int) * 4);
// }

// void usr_set_default_color(int color)
// {
// 	if (color < 0)
// 		return;

// 	CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;
// 	car_info->color_style = color;
// 	m_currentColorIx = car_info->color_style;
// 	AVM_LOGI("Usr setting init car color is %d\n", car_info->color_style);
// }



int __SYS_INIT__()
{
	return frame < 5;
}

void reloadobj_ifneed()
{
	autosys_current_info* info     = &autosys.current_info;
	autosys_page *avm              = &autosys.avm_page;
	ePage      *ePage              = &avm->page[avm->current_page];
	if (avm->current_page >= MAX_PAGE_NUM)
		return;

	int avm_3d_height, avm_3d_width;

	info->notify_show_cur_page_context = 1;
	info->TOP2D_RANGE.X        = ePage->set.viewport[PAGE_2DAVM].W;
	info->TOP2D_RANGE.Y        = ePage->set.viewport[PAGE_2DAVM].H;
	info->TOP2D_LayoutPoint.X  = ePage->set.viewport[PAGE_2DAVM].X;
	info->TOP2D_LayoutPoint.Y  = ePage->set.viewport[PAGE_2DAVM].Y;
	/*--------------------*/
	/*MOD information*/
	upload_newMODline(ePage->set.mod.buffer);
	/*--------------------*/

	/*--------------------*/
	/*fov information*/
	avm_3d_height = ePage->set.viewport[PAGE_3DAVM].H;
	avm_3d_width  = ePage->set.viewport[PAGE_3DAVM].W;
	reloading_3d_avm_fov(&para3Davm, avm_3d_width, avm_3d_height);
	AVM_LOGI("-----------------------------\n");
	AVM_LOGI("\033[35mSetting AVM 3d FOV h [%d], w [%d]\n\033[0m", avm_3d_height, avm_3d_width);
	AVM_LOGI("-----------------------------\n");
	/*--------------------*/

	/*rotate information*/
	lock_page_roate = 0;
	/*touch */
	autosys_set_zoomin2DprojectionmartixReset();
}

char *avm_core_version()
{
	char *avm_version = "b09-01";
	return avm_version;
}
float theta_1 = 0;
float phi_1 = 45;
#define AVM_get_SYSTEM_info 0x0A

void *free_touch_func(void *ptr)
{
	struct can_frame _AVM_get_SYSTEM_info;
	int fd_autosys_free_touch;
	fd_autosys_free_touch = open("/dev/autosys_avm_cmd", O_RDWR);
	while (1)
	{
		ioctl(fd_autosys_free_touch, AVM_get_SYSTEM_info, &_AVM_get_SYSTEM_info);
		touchdata.x = _AVM_get_SYSTEM_info.data[3];
		touchdata.y = _AVM_get_SYSTEM_info.data[5];
		if (touchdata.x == 0)
			touchdata.x = -1;
		if (touchdata.y == 0)
			touchdata.y = -1;
		avm_panel_moveView();
		usleep(100);
	}
}

void touch_update(int cur_frame, autosys_avm_info *_autosys_avm_info)
{

	if (cur_frame == 0)
	{
		pthread_create(&free_touch_thread, NULL, free_touch_func, NULL); // 建立子執行緒
	}

	touchdata.x = _autosys_avm_info->touch_x_H;
	touchdata.y = _autosys_avm_info->touch_y_H;

	if (touchdata.x == 0)
		touchdata.x = -1;
	if (touchdata.y == 0)
		touchdata.y = -1;
}

void test_zip_alpha()
{
	gettimeofday(&start1, NULL);
	float *buffer = (float *)malloc(22044 * 2 * sizeof(float));
	FILE *fp = fopen("raw_alpha.bin", "rb");
	fread(buffer, 22044 * 2, sizeof(float), fp);
	fclose(fp);
	printf("\033[31m bin buffer x[0] = %f, x[1] = %f\n \033[0m", buffer[0], buffer[2]);
	gettimeofday(&end1, NULL);
	printf("readbin time = %f\n", calculate_time(start1, end1));

	gettimeofday(&start1, NULL);
	int width, height, channels;
	unsigned char *data = stbi_load("outputX.png", &width, &height, &channels, 4);
	memcpy(buffer, data, sizeof(float) * 22044);
	printf("\033[31m xpicture x[0] = %f, x[1] = %f\n \033[0m", buffer[0], buffer[1]);
	data = stbi_load("outputY.png", &width, &height, &channels, 4);
	memcpy(buffer, data, sizeof(float) * 22044);
	printf("\033[31m xpicture y[0] = %f, y[1] = %f\n \033[0m", buffer[0], buffer[1]);
	gettimeofday(&end1, NULL);
	printf("decode png time = %f\n", calculate_time(start1, end1));
	free(buffer);
}

void read_frame_buffer()
{
	unsigned char *buffer;
	buffer = (unsigned char *)calloc(160 * 90 * 3 * sizeof(unsigned char), sizeof(unsigned char));
	glReadPixels(0, 0, 160, 90, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	// glReadPixels( 0, 0, 768/2, 774, GL_RGBA, GL_UNSIGNED_BYTE, bufferRGBout);

	AVM_LOGI("saving windows readpixels %d\n", buffer[0]);
}

void Notify_avm_animation_done(autosys_page *avm)
{
	ePage      *ePage       = &avm->page[avm->current_page];

	// if (ePage->set.animation.para.TurnFlag)
	// {
	// 	// printf("flag_360_finsih = %d\n",flag_360_finsih);
	// 	int fd = open("/dev/autosys_avm_cmd", O_RDWR);
	// 	struct can_frame _AVM_set_360_status;
	// 	_AVM_set_360_status.data[0] = flag_360_finsih;
	// 	ioctl(fd, AVM_set_360_status, &_AVM_set_360_status);
	// 	close(fd);
	// }

	if (ePage->set.animation.para2.TurnanimationFlag)
	{
		// printf("flag_360_finsih = %d\n",flag_360_finsih);
		int fd = open("/dev/autosys_avm_cmd", O_RDWR);
		struct can_frame _AVM_set_360_status;
		_AVM_set_360_status.data[0] = flag_360_finsih;
		ioctl(fd, AVM_set_360_status, &_AVM_set_360_status);
		close(fd);
	}
}
GLuint query;
void __AVM_UPDATELAYER__(autosys_avm_info *_autosys_avm_info)
{
	//__IVI_SET_CARMODEL_LAMPSTATUS__(_autosys_avm_info.PGL_style_flag);
	//__IVI_SET_WRITECUR_FRAMEBUFFER__(_autosys_avm_info->car_color);
	//__IVI_SET_CARMODEL_STATUS(_autosys_avm_info);
	__IVI_SET_CARMODEL_LAMPSTATUS_SIGNAL__(_autosys_avm_info->light_signal);
	__IVI_SET_CARMODELCOLOR__(_autosys_avm_info->car_color);
	__IVI_SET_PGL_STYLE__(-1, _autosys_avm_info->PGL_style_flag);
	__IVI_RESET_540ROTATE__(_autosys_avm_info->rotate_540_reset_flag);
	__IVI_SET_360ROTATE__(true); // if the value is false, any page will not 360 rotate even the page 360 flag is trun on.
	__IVI_SET_MUTITOUCH__(_autosys_avm_info->muti_touchIndex);
	__IVI_SET_AVM_BB_PID__(_autosys_avm_info->AVM_PID_Latancy);
	__IVI_GET_CAMERA_STATUS_(_autosys_avm_info);
	__IVI_RESET_SEETHROUGH__(_autosys_avm_info);
	//__IVI_SET_SEETHROUGH_PROCESS_SIGNAL__(_autosys_avm_info);
	if (__IVI_ISCHANGEPAGE__() == true)
	{
		autosys.current_info.cur_clear_drawpanel = 1;
		autosys.current_info.line_info.flag_line_gpu_refresh = 1;
		printf("\033[31m AVM Brightness function [%d] \033[0m \n", _autosys_avm_info->brightness_adj);
		reloadobj_ifneed();
	};

	autosys.g_flag.fuc_zoom_birdView = _autosys_avm_info->ZOOM_ON_OFF;
	

	zoom_birdView *zoom        = &autosys.zoom;
	autosys_current_info *info = &autosys.current_info;
	if(info->cur_frame == 0)
	{
		GL_PREVIOUS_FBO = autosys.frame_fbo[FBO_4TO1].framebuffer;
		// GLint numExts = 0;
		// glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);

		// for (int i = 0; i < numExts; i++) 
		// {
    	// 	const char *extName = (const char*) glGetStringi(GL_EXTENSIONS, i);
		//     printf("%s\n", extName);
		// }
	}
	else
	{
		if(zoom->update_status == 1)
		{
			//AVM_LOGE("Up Up\n");
			autosys_updata_zoominprojectionmatrix();
			zoom->update_status = 0;
		}

	}

	if( _autosys_avm_info->vehicle_speed >= 60.0f)
	{
		autosys.seethrough.seethrough_switch = 0;
	}

}

float average_timeCose1 = 0.0f;
float average_timeCose2 = 0.0f;
float average_timeCose3 = 0.0f;
struct timeval start2, end2;
struct timeval start3, end3;
int flag_GPU_time = 0;
static int queryPending = 0;
void routine_avm(user_command_mode_t *user_command, can_bus_info_t *canBus, autosys_avm_info _autosys_avm_info, struct can_frame *_AVM_set_MOD_info)
{
	autosys_current_info* cur_info = &autosys.current_info;
	autosys_page *avm              = &autosys.avm_page;
	avm->current_page              = (_autosys_avm_info.avm_view_page < MAX_PAGE_NUM)?_autosys_avm_info.avm_view_page:MAX_PAGE_NUM + 1;
	ePage      *ePage              = &avm->page[avm->current_page];
	                                  
	cur_info->cur_frame = frame;
	touch_update(cur_info->cur_frame, &_autosys_avm_info);

	mod_recall_flag                      =  ePage->set.mod.sensity * 10; 
	if(!cur_info->flag_disable_ivi_control)
	{
		autosys.seethrough.seethrough_switch = _autosys_avm_info.TRN_ON_OFF;
		autosys.g_flag.fuc_smod              = _autosys_avm_info.mod_ON_OFF;
		autosys.g_flag.fuc_avm_bb            = _autosys_avm_info.brightness_adj;
	}

	if(autosys.g_flag.fuc_freeze == 1) return;
	Notify_avm_animation_done(avm);

	__AVM_UPDATELAYER__(&_autosys_avm_info);

	if (GO_to_QT_flag == 1) // Have screen shot before need to shut down avm
	{
		GO_to_QT_flag = 0;
		sharedmemory();
		usleep(15000);
	}

	set_trans_camera_color(&avmUiInfo, transCamera, (autosys.g_flag.fuc_avm_bb  == 1));
	user_command_mode_t userCommandNow = *user_command;
	canBusAvm[NOW_CAN] = *canBus;
	canBusAvm[NOW_CAN].speed = _autosys_avm_info.vehicle_speed;
	canBusAvm[NOW_CAN].meterage = canBus->meterage;
	canBusAvm[NOW_CAN].steer = _autosys_avm_info.steering_angle * -1.0f;

	if (_autosys_avm_info.gear == 4 || _autosys_avm_info.gear == 2)
	{
		canBusAvm[NOW_CAN].gear = 'D';
		canBusAvm[NOW_CAN].gear = 'N';
	}
	else if (_autosys_avm_info.gear == 8)
		canBusAvm[NOW_CAN].gear = 'R';
	else if (_autosys_avm_info.gear == 1)
		canBusAvm[NOW_CAN].gear = 'P';

	//AVM_LOGI("canBusAvm[NOW_CAN].speed = %f, meterage = %f\n",canBusAvm[NOW_CAN].speed,canBusAvm[NOW_CAN].meterage);
#if SEETHROUGH_DEMO
	//autosys.seethrough.seethrough_switch = 1;
	canBusAvm[NOW_CAN].meterage = meterage;
	canBusAvm[NOW_CAN].speed = 15.0f;
	canBusAvm[NOW_CAN].gear = 'D';
	canBusAvm[NOW_CAN].steer = (float)_autosys_avm_info.PGL_style_flag * 4.0f;
#else
	// canBusAvm[NOW_CAN].speed = 20.0f;
	// canBusAvm[NOW_CAN].gear  = 'D';
	// canBusAvm[NOW_CAN].steer = (float)(frame % 60) - 30.0f;
	// canBusAvm[NOW_CAN].steer = 0.0f;
#endif
	float temp_time360 = calculate_time(start360, end360);

	fps360 = (1 / temp_time360);
	fps360 = 30.0;
	cal_tire_roate_angle(&Rotateangle, &canBusAvm[NOW_CAN].speed, fps360,_autosys_avm_info.gear);
	routine_camera(transCamera[0], canBusAvm[NOW_CAN]);
	trans_cam_cpu_to_gpu(transCamera, canBusAvm[NOW_CAN], cam_ch);
	RotateCallculate(canBusAvm[NOW_CAN]);
	generate_2davm_texture();
	generate_mod_texture(canBusAvm[NOW_CAN]);
	generate_see_through_texture(canBusAvm[NOW_CAN]);
	generate_see_through_process_texture();
	generate_blur_fisheye_texture();
	generate_FXAA_texture();
#ifdef MSAA
	glBindFramebuffer(GL_FRAMEBUFFER, fboMSAA.framebuffer);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, avm_fbo);
#endif
	gettimeofday(&start2, NULL);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CAR_MODEL_INFO *car_info       = &autosys.current_info.carmodel_info;

	switch (_autosys_avm_info.avm_view_page)
	{
	case MAX_PAGE_NUM:
		avmUiInfo.view = VIEW_RECORD_4_IN_1;
		break;
	case MAX_PAGE_NUM + 1:
		avmUiInfo.view = FRONT_VIEW;
		break;
	case MAX_PAGE_NUM + 2:
		avmUiInfo.view = BACK_VIEW;
		break;
	case MAX_PAGE_NUM + 3:
		avmUiInfo.view = LEFT_VIEW;
		break;
	case MAX_PAGE_NUM + 4:
		avmUiInfo.view = RIGHT_VIEW;
		break;
	default:
		avmUiInfo.view = VIEW_AVM_LAYOUT;
		break;
	}
	set_draw_window(&avmUiInfo, &canBusAvm[NOW_CAN]);

	if(car_info->freeze_frame_count > 0)
	{
		car_info->freeze_frame_count--;

	}

#ifdef MSAA
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMSAA.framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, avm_fbo);
	glBlitFramebuffer(0, 0, SCREEN_W, SCREEN_H, 0, 0, SCREEN_W, SCREEN_H, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#endif

	frame++;
	gettimeofday(&canbus_end, NULL);
#if SEETHROUGH_DEMO
	dtime = calculate_time(canbus_start, canbus_end);
	meterageEach = (beforeSpeed + canBusAvm[NOW_CAN].speed) * dtime/ (2 * 3.6) * 0.95;
	meterage = meterage + meterageEach;
	beforeSpeed = canBusAvm[NOW_CAN].speed;
#else
	float dtime = calculate_time(canbus_start, canbus_end);
	meterageEach = (beforeSpeed + (double)canBusAvm[NOW_CAN].speed) * dtime / (2 * 3.6) * 0.95;
	canBusAvm[NOW_CAN].meterage = canBusAvm[NOW_CAN].meterage + meterageEach;
	canBus->meterage = canBusAvm[NOW_CAN].meterage;
	beforeSpeed = (double)canBusAvm[NOW_CAN].speed;
#endif
	gettimeofday(&canbus_start, NULL);

	if (frame % 40 == 0 && cur_info->flag_show_canbus_log == 1)
	{
		AVM_LOGI("CarPara gear[%d], angle[%f], speed[%f], meterage[%f]\n",
		canBusAvm[NOW_CAN].gear,
		canBusAvm[NOW_CAN].steer,
		canBusAvm[NOW_CAN].speed,
		canBusAvm[NOW_CAN].meterage);
	}

	// printf("canBusAvm[NOW_CAN].meterage = %f\n",canBusAvm[NOW_CAN].meterage);
	// printf("meterage = %f\n",meterage);

	avmUiInfo.viewold = avmUiInfo.view;

	if (cleanCommendFlag && (userCommandNow == *user_command))
	{
		*user_command = CMD_NONE;
		cleanCommendFlag = 0;
	}

	canBusAvm[LAST_CAN] = canBusAvm[NOW_CAN];

	fpscount += 1;
	if (fpscount == 500)
	{
		gettimeofday(&end, NULL);
		temp_time = calculate_time(start, end);
#ifdef DEBUG_FPS
		printf("aaaTcount = %f\n", (500.0f / temp_time) * 3.0f);
#endif
		fpscount = 0;
		gettimeofday(&start, NULL);
	}
	_AVM_set_MOD_info->data[1] = AutoSys_mod_front_flag;
	_AVM_set_MOD_info->data[2] = AutoSys_mod_back_flag;
	_AVM_set_MOD_info->data[3] = AutoSys_mod_left_flag;
	_AVM_set_MOD_info->data[4] = AutoSys_mod_right_flag;

	GL_PREVIOUS_FBO = avm_fbo;
	// usleep(30000);
	// END = clock();
	// printf("Total time = %f\n", (END - START) / CLOCKS_PER_SEC);
}
