/**
 *******************************************************************************
 * @file : draw_windows_handle.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20201220 0.1.0 Linda.
 *******************************************************************************
 */
#include <math.h>
#include <sys/types.h>
#include "../../system.h"
#include "avm/GLHeader.h"
#include "user_control/ui.h"
#include "avm/camera_matrix.h"

#include "set_memory.h"
#include "program/program_line.h"
#include "program/program_car.h"
#include "program/program_rgba.h"
#include "program/program_main.h"
#include "program/program_seeThrough.h"
#include "program/program_TSAA.h"
#include "program/program_mod.h"

#include "draw_windows_handle.h"
#include "display_location.h"
#include "avmDefine.h"
#include "avm/avm_support_funcs.h"
#include "avm_2d_3d_para.h"
#include "avm/maths.h"
#include "avm/bb_func.h"
#include "gl/glFunUtils.h"
#include <string.h>
// status_bar_init.txt

float alpha2D;
extern float Rotateangle;
extern camera_para_t para2Davm, para3Davm, paraCorrectEye;
// extern stAVMPGLdrawView MODE_VIEW[max_ViewPage];
// extern int cam_ch;
int page_rotate_flag = 0;
int lock_page_roate = 0;
static int test_count = 0;
int rolate_360_flag = 0;
static int rolate_360_angle_flag = 0;
static int init_EightTouch_angle = 1;
static int current_EightTouch_index = 0;
static float m_glViewPort[2][4] = {0.0f};
struct timeval time_360_clock[2];
static SET_CUR_VIEWDATA Aligmentdata;
stEightTouch eight;
int g_CarModel_ever_do_rotate_ones[50] = {0};

typedef struct 
{
	float startAngle;
	float endAngle;
	int direction;
	float circle;
	float timer;
	float gap;
	float ESP_total_angle;
	int cur_level_flag;
	int recall_level_flag;
	int async_init_angle_flag;
	
}AnimationData;

AnimationData animationData;


void carmodel_fbo_createOrdelete()
{

	CAR_MODEL_INFO *car_info       = &autosys.current_info.carmodel_info;
	if(car_info->Triger_Destory == 1)
	{
		car_info->Triger_Destory = 0;
		//destroy_fbo(&autosys.frame_fbo[FBO_STATIC_CAR_MODEL]);
		//AVM_LOGI("[SYSTEM] destory fbo\n");
	}

	if(car_info->Triger_Regester == 1)
	{
		// GLint viewport[4];
		// glGetIntegerv(GL_VIEWPORT, viewport);
		car_info->Triger_Regester = 0;
		// autosys_current_info* info     = &autosys.current_info;
		// autosys_page *avm              = &autosys.avm_page;
		// ePage      *ePage              = &avm->page[avm->current_page];
		// int car_3d_height = ePage->set.viewport[PAGE_3DAVM].H *  PARAM_FXAA_LOSS_PANEL_VOLUME;
		// int car_3d_width  = ePage->set.viewport[PAGE_3DAVM].W *  PARAM_FXAA_LOSS_PANEL_VOLUME;
		// if(autosys.frame_fbo[FBO_STATIC_CAR_MODEL].framebuffer == 0)
		// {
		// 	AVM_LOGI("[SYSTEM] create carmodel fbo w [%d], h [%d]\n", car_3d_width, car_3d_height);
		// 	create_fbo(&autosys.frame_fbo[FBO_STATIC_CAR_MODEL], car_3d_width, car_3d_height, UI_TEST1_enable, GL_LINEAR);
		// 	AVM_LOGI("[SYSTEM] create fbo\n");
		// }
	}
}
void draw_3d_avm_static_car(float carAngle, AVM_CARMODEL *carmodel)
{
	camera_para_t para3Dcar = para3Davm;
	para3Dcar.exPara.tmpCenter[0] += carmodel->vcam.x_position;
	para3Dcar.exPara.tmpCenter[1] += carmodel->vcam.y_position;
	para3Dcar.exPara.tmpCenter[2] += carmodel->vcam.z_position;
	rotate_callculate(&para3Dcar);
	setting_CarModel_display(0, NULL);
	int* alphaMode = (int*)calloc(4, sizeof(int));
	memcpy(alphaMode, autosys.system_info.car_alpha_mode, sizeof(int) * 4);
	if(carmodel->OffroadMode_displayItem  != NULL && PARAM_OPENGL_OIT_BUFFER_FBO_ON)
	{
		alphaMode[2] = 1;
	}
	else if(PARAM_OPENGL_OIT_BUFFER_FBO_ON == 0)
	{
		alphaMode[0] = 0; // car_body
		alphaMode[1] = 0; // window
		alphaMode[2] = 0; // tire
		alphaMode[3] = 0; // interior
	}

	CAR_MODEL_INFO *car_info       = &autosys.current_info.carmodel_info;
	autosys_page *avm              = &autosys.avm_page;
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int FBO_ID_USED = FBO_STATIC_CAR_MODEL_0 + (car_info->Pages_key[avm->current_page] - 1);
	if(car_info->f_record == 1)
	{
		GLint  previousFBO;
    	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, autosys.frame_fbo[FBO_ID_USED].framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int disable_item[20]  = {0};
		disable_item[TIRE]    = 1;
		disable_item[LAMP]    = 1;
		disable_item[WINDOWS] = 1;
		disable_item[INTERIOR] = 1;
		glViewport(0, 0, viewport[2], viewport[3]);
		draw_runtime_car(para3Dcar, carmodel, alphaMode, disable_item);
		draw_pre_oit_real_car(para3Dcar, carmodel, alphaMode, disable_item);

		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		AVM_LOGI("viweport: %d, %d, %d, %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
		draw_runtime_car_oit(para3Dcar, carmodel, alphaMode, disable_item);

		backup_oit_main(true);
		glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
		car_info->f_record = 0;
	}
	
	glViewport(viewport[0],viewport[1], viewport[2], viewport[3]);
	draw_static_car_png(1.0f, autosys.frame_fbo[FBO_ID_USED].texture);
	glClear(GL_DEPTH_BUFFER_BIT);
	GL_COPY_FRAMEBUFFER_DEPTH(autosys.frame_fbo[FBO_ID_USED].framebuffer, autosys.frame_fbo[FBO_FXAA].framebuffer,
								0, 0, autosys.frame_fbo[FBO_ID_USED].width, autosys.frame_fbo[FBO_ID_USED].height,
								viewport[0],viewport[1], viewport[2], viewport[3]);

	int disable_item[20] = {0};
	disable_item[CAR_BODY] = 1;
	draw_runtime_car(para3Dcar, carmodel, alphaMode, disable_item);
	draw_pre_oit_real_car(para3Dcar, carmodel, alphaMode, disable_item);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(
		viewport[0] + PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME,
		viewport[1] + PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME,
		viewport[2] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME,
		viewport[3] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME);
	draw_runtime_car_oit(para3Dcar, carmodel, alphaMode, disable_item);
	glViewport(0, 0, PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT);
	glClear(GL_DEPTH_BUFFER_BIT);
	draw_main_oit( (true & autosys.seethrough.seethrough_switch));
	free(alphaMode);
}
static void draw_3d_avm_car(float carAngle, avm3d_windows_t windows, int cameraPosition, Car_item_Alpha *alpha, stCamera *cameraParameter)
{

}

static void draw_3d_avm_car_img(float carAngle, avm3d_windows_t windows, int cameraPosition, Car_item_Alpha *alpha, stCamera *cameraParameter)
{

}

static void set_inpara(float width, float height)
{
	para3Davm.inPara.width = width;	  // m_viewport[2];//AVM_3D_W;//para2Davm.inPara.width;
	para3Davm.inPara.height = height; // m_viewport[3];//AVM_3D_H;

	para3Davm.inPara.horiFov = 0.30;
	para3Davm.inPara.vertiFov = (para3Davm.inPara.horiFov) * (para3Davm.inPara.height) / (para3Davm.inPara.width);
}

/**
 * @author alfred
 * @brief
 * @date 2023/03/28
 */
void reset_EightTouch_para3Davm(int screen, int index)
{

}

void reset_para3Davm(int screen)
{

}

float CalculateGapByValue(float startAngle, float endAngle, int direction,float value)
{
	float gap = 0.0f;
	if (direction == 1)
	{
		if (endAngle > startAngle)
			gap = endAngle - startAngle;
		else
			gap = value - (startAngle - endAngle);
	}
	else
	{
		if (endAngle >= startAngle)
			gap = value - fabsf(endAngle - startAngle);
		else
			gap = startAngle - endAngle;
	}
	return gap;
}
float CalculateCurrentAngleByValue(float processtime, float startAngle, float total_angle, int direction, float Time, float value)
{
	float current_angle = startAngle + ((processtime / Time) * total_angle);
	return (direction == 0) ? ((value/2.0f) - fmodf(current_angle, value )) : (fmodf(current_angle, value));
}

float CalculateCurrentAngleByphi(float processtime, float startAngle, float total_angle, int direction, float Time, float value)
{
	float isminus = (direction == 0)?-1.0f:1.0f;
	float current_angle = startAngle + ((processtime / Time) * total_angle * isminus);
	return current_angle;
	// float current_angle = startAngle + ((processtime / Time) * total_angle);
	// return (direction == 0) ? ((value/2.0f) - fmodf(current_angle, value )) : (fmodf(current_angle, value));
}

float CalculateGap(float startAngle, float endAngle, int direction)
{
	float gap = 0.0f;
	if (direction == 1)
	{
		if (endAngle > startAngle)
			gap = endAngle - startAngle;
		else
			gap = 360.0f - (startAngle - endAngle);
	}
	else
	{
		if (endAngle > startAngle)
			gap = 360.0f - fabsf(endAngle - startAngle);
		else
			gap = startAngle - endAngle;
	}
	return gap;
}

float CalculateGap_EightTouch(float startAngle, float endAngle, int direction)
{
	float gap = 0.0f;
	if (direction == 1)
	{
		if (endAngle > startAngle)
			gap = endAngle - startAngle;
		else
			gap = 360.0f - (startAngle - endAngle);
	}
	else
	{
		if (endAngle > startAngle)
			gap = 360.0f - fabsf(endAngle - startAngle);
		else
			gap = startAngle - endAngle;
	}
	return gap;
}

float CalculateCurrentAngle_EightTouch(float processtime, float startAngle, float total_angle, int direction, float Time)
{
	if (direction == 1)
	{
		float current_angle = startAngle + ((processtime / Time) * total_angle);
		return fmodf(current_angle, 360.0f);
	}
	else
	{
		float current_angle = startAngle - ((processtime / Time) * total_angle);
		return fmodf(current_angle, 360.0f);
	}
}

float CalculateCurrentAngle(float processtime, float startAngle, float total_angle, int direction, float Time)
{
	float current_angle = startAngle + ((processtime / Time) * total_angle);
	return (direction == 0) ? (180.0f - fmodf(current_angle, 360.0f)) : (fmodf(current_angle, 360.0f));
}

void InitAnimationPara(AnimationData *animationData)
{
	animationData->cur_level_flag = 0;
	flag_360_finsih = 0;
	animationData->recall_level_flag = 1;
	animationData->async_init_angle_flag = 0;
}

void EndAnimationPara(AnimationData *animationData, int page)
{
	flag_360_finsih = 1;
	animationData->recall_level_flag = 0;
	g_CarModel_ever_do_rotate_ones[page] = 1;
}

void GotoNextAnimationPara(AnimationData *animationData)
{
	animationData->recall_level_flag = 1;
	animationData->cur_level_flag++;
}
//bool CalAnimationPara(int curLevel, global_setting *glob, int* async_flag,  void* data[])
bool CalAnimationPara(global_setting *glob, AnimationData *animationData)
{
	int curLevel           = animationData->cur_level_flag;
	int* async_flag        = &animationData->async_init_angle_flag;
	animation_group* group = &glob->animation.para2.group[(curLevel >> 1)];
	animation_para* para   = NULL;

	switch(curLevel % 2)
	{
		case 0: //theta
			para           = &group->theta_para;
			if(!para->Flag_Enablerun)
			{
				return true;
			}

			if(*async_flag == 0)
			{
				animation_para *phi_para  = &group->phi_para;
				if(phi_para->Flag_Enablerun)
				{
					*async_flag = 1;
					para3Davm.exPara.phi = phi_para->StartPoint- 90.0f;
				}
			}

			animationData->startAngle = para->StartPoint;
			animationData->endAngle   = para->EndPoint;
			animationData->direction  = para->Direction;
			animationData->circle     = (float)para->Cycle;
			animationData->timer      = para->Time;
			para3Davm.exPara.theta = animationData->startAngle;
			animationData->gap = CalculateGapByValue(animationData->startAngle, animationData->endAngle, animationData->direction, 360.0f);
			animationData->ESP_total_angle = (360.0f * animationData->circle) + animationData->gap;
		break;
		case 1: //phi
			para           = &group->phi_para;
			if(!para->Flag_Enablerun)
			{
				return true;
			}

			if(*async_flag == 0)
			{
				if( (curLevel + 1) >> 1 < MAX_ANIMATION_NUM)
				{
					animation_para *theta_para  = &glob->animation.para2.group[(curLevel + 1)>>1].theta_para;
					if(theta_para->Flag_Enablerun)
					{
						*async_flag = 1;
						para3Davm.exPara.theta = theta_para->StartPoint;
					}
				}
			}

			animationData->startAngle = para->StartPoint- 90.0f;
			animationData->endAngle   = para->EndPoint- 90.0f;
			animationData->direction  = para->Direction;
			animationData->circle     = (float)para->Cycle;
			animationData->timer      = para->Time;
			para3Davm.exPara.phi = animationData->startAngle;
			animationData->gap = CalculateGapByValue(animationData->startAngle, animationData->endAngle, animationData->direction, 180.0f);
			animationData->ESP_total_angle = (180.0f * animationData->circle) + animationData->gap;
		break;
	}		

	AVM_LOGI("***Start angle is %f***\n", animationData->startAngle);
	AVM_LOGI("***End angle is %f***\n",   animationData->endAngle);
	AVM_LOGI("***Circle is %f***\n",      animationData->circle);
	AVM_LOGI("***Total angle is %f***\n", animationData->ESP_total_angle);
	gettimeofday(&time_360_clock[0], NULL);
	rotate_callculate(&para3Davm);
	return false;
}

int checkDirection(float startAngle, float endAngle) 
{

    float angleDiff = startAngle - endAngle;

	int direction = 0;
    if (angleDiff > 0) {
        printf("The nearest direction is clockwise.\n");
		direction = 0;
    } 
	else 
	{
        printf("The nearest direction is counterclockwise.\n");
		direction = 1;
    }
	return direction;
}

int flag_360_finsih = 0;
void angle_display(int show, int cam, int screen, avm_ui_info_t *uiInfo, can_bus_info_t *can)
{

}


void show_car_mode(int show, int cam, int screen, avm_ui_info_t *uiInfo, can_bus_info_t *can)
{

}

int FindTureIndex(bool (*input)[10], int view, int length)
{
	for (int i = 0; i < length; i++)
	{
		if (input[(int)view][i] == 1)
			return i;
	}
	return -1;
}

#if dynamic_line_union_test
static float anglecount = 0;
static float ccccangle = 0;
#endif

void draw_PGL(int _channel)
{

}

void Cal_Correct_Fisheye_rotate(int screen)
{

}

void FXAA_SHOW(int show, int cam, int screen, avm_ui_info_t *uiInfo, can_bus_info_t *can)
{

}

void FXAA_preDraw_VIEW(avm_ui_info_t *uiInfo, can_bus_info_t *can)
{
	#if 1
	autosys_page *avm      = &autosys.avm_page;
    int current_page       = avm->current_page;
	if(current_page >= MAX_PAGE_NUM) return;
    ePage      *ePage      = &avm->page[current_page];

    avm_draw_list *list    = &ePage->list;
    avm_contexts *contexts = &ePage->contexts;
    int context_num        = ePage->set.context_num;
    int ix = 0;
	/*---------------------flag intergartion------------*/
	is_seethrough_enabled(&avm->flag);
	is_fxaa_enabled(&avm->flag);
	is_mod_enabled(&avm->flag);
	is_avm_3d_enabled(ePage, &avm->flag);
	is_scene_enabled(&avm->flag);
	is_carModel_shadow_line_enabled(&avm->flag);
	/*---------------------end--------------------------*/
	flash_seethrough_texture();
	flash_car_system(can);
	carmodel_fbo_createOrdelete();
    while(ix < context_num && ix < MAX_PAGE_CONTENTS)
    {
        int ID = list[ix].ID;
        Rectangle viewport = list[ix].viewport;
		viewport.Y = g_PANEL_HEIGHT - viewport.Y - viewport.H;
        glViewport(viewport.X * PARAM_FXAA_LOSS_PANEL_VOLUME, viewport.Y * PARAM_FXAA_LOSS_PANEL_VOLUME, viewport.W * PARAM_FXAA_LOSS_PANEL_VOLUME, viewport.H * PARAM_FXAA_LOSS_PANEL_VOLUME);
        switch(ID)
        {
			case PAGE_FISHEYE:
			{
				FISHEYE_SUPPLY *supfisheye;
				// printf("avm->flag.disaply_inner_sixtouch = %d\n",avm->flag.disaply_inner_sixtouch);
				// printf("avm->flag.display_fisheye = %d\n",avm->flag.display_fisheye);
				if(avm->flag.disaply_inner_sixtouch == true && avm->flag.display_fisheye == true)
				{
					supfisheye = &list[ix].supply.sup_fisheye;
					int8_t select_index = autosys.current_info.mutiple_touch_index - 8;
					select_index = (select_index < 0) ? 0 : select_index;
					//AVM_LOGI("select cam [%d]\n",select_index);
					draw_cam_fish(CAM_VIEW_FISHEYE,
					ePage->set.touch.fourteen.Select_cam[select_index],
					&ePage->set.touch.fourteen.cropped[select_index], 0);
				}
				else if(avm->flag.display_fisheye == true)
				{
					supfisheye = &list[ix].supply.sup_fisheye;
					int view = (supfisheye->mode == 0)?CAM_VIEW_FISHEYE:CAM_VIEW_FISHEYE_CYLINDRICAL;
					draw_cam_fish(view,supfisheye->Select_cam, &supfisheye->cropped, supfisheye->rotate);
				}

				break;
			}
			case PAGE_SCENE:
			{
				int select_cam = list[ix].supply.sup_scene.Select_virtualCam_id;
				virtual_cam *camview = &(ePage->contexts.avm_scene.vcam[select_cam]);
				float xratio = ePage->contexts.avm_scene.view_x_axis_ratio[select_cam];
				float yratio = ePage->contexts.avm_scene.view_y_axis_ratio[select_cam];
				xratio = (xratio == 0.0f) ? 1.0f : xratio;
				yratio = (yratio == 0.0f) ? 1.0f : yratio;
				float transformMatrix[] =
				{
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1
				};
				memcpy(paraCorrectEye.Model, transformMatrix, sizeof(float) * 16);
				paraCorrectEye.Model[2] = 0.0f + camview->eyeRadius;	//(float)MODE_VIEW[cam_ch].eyeRadius[screen][0];
				paraCorrectEye.Model[6] = 0.0f + camview->phi;			//(float)MODE_VIEW[cam_ch].phi[screen][0];
				paraCorrectEye.Model[10] = 0.0f + camview->theta;		//(float)MODE_VIEW[cam_ch].theta[screen][0];
				paraCorrectEye.Model[12] = 0.0f + camview->x_position;	//(float)MODE_VIEW[cam_ch].cam_x[screen][0];
				paraCorrectEye.Model[13] = 0.0f + camview->y_position;	//(float)MODE_VIEW[cam_ch].cam_y[screen][0];
				paraCorrectEye.Model[11] = 0.0f + camview->z_position;	//(float)MODE_VIEW[cam_ch].cam_z[screen][0];
				paraCorrectEye.Model[0]  = 1.0f *  xratio;
				paraCorrectEye.Model[5]  = 1.0f *  yratio;
				//AVM_LOGI("style = %d\n", list->supply.sup_scene.Style);
				draw_cam_correct_2d(&paraCorrectEye.Model[0], CAM_VIEW_CORRECT, select_cam + 2, list[ix].supply.sup_scene.Style);
				//draw_cam_correct_2d(&paraCorrectEye.Model[0], CAM_VIEW_CORRECT, select_cam + 2, 1);
				break;
			}
            case PAGE_PGL:
			{
				//if(list[ix].supply.sup_pgl.mode == FISHEYE )
				//if(list[ix].supply.sup_pgl.mode == FISHEYE || list[ix].supply.sup_pgl.mode == SCENE)
				{
					glClear(GL_DEPTH_BUFFER_BIT);
					draw_PGL_line(ePage, &contexts->avm_pgl, &list[ix].supply.sup_pgl, viewport);
				}
            	break;
			}
			
			case PAGE_2DAVM:
			{
                render_2d_avm(ePage, &avm->flag);
				break;
			}
			case PAGE_3DAVM:
			{
				render_3d_avm(ePage, current_page, viewport, &avm->flag);
				break;
			}
			case PAGE_2DCAR:
			{
				render_2d_car(ePage, contexts, &avm->flag);
				break;
			}
			case PAGE_CARMODEL:
			{
				#if 1
					render_3d_car(ePage, contexts, &list[ix], can, &avm->flag);
				#endif
				break;
			}
			case PAGE_MOD:
			{
				//render_mod(ePage, &avm->flag);
				break;
			}
			case PAGE_SHADOW: // or render seethrough
			{
				render_shadow(ePage, &list[ix], &avm->flag);
				break;
			}
		}
        ix++;
    }
	#endif
}

void __IVI_CAMERA_WARINING__(int mode, int cam)
{
	if (g_avm_camera_flag > 0)
	{
		// glViewport(0, 0, g_PANEL_WIDTH, g_PANEL_HEIGHT);
		int status = g_avm_camera_flag;
		for (int ix = 0; ix < 4; ix++)
		{
			if (mode != FISHEYE_DISTORTION)
			{
				if (status & (0x01 << ix))
					draw_camera_warning(ix + 1);
			}
			else
			{
				if (status & (0x01 << ix) && ix == cam)
					draw_camera_warning(0);
			}
		}
	}
}

// static int test_count = 0;
void ELEAD_CAR_MODE_VIEW(avm_ui_info_t *uiInfo, can_bus_info_t *can)
{

}

void set_draw_window(avm_ui_info_t *uiInfo, can_bus_info_t *can)
{
	// printf("%d\n", uiInfo->view);
	switch (uiInfo->view)
	{
	/* ADAS AND IMS */
	case VIEW_IMS_ADAS:
		glViewport(0, 0, 1280, 720);
		draw_adas_ims(CAM_VIEW_FISHEYE, CAM_NUM_BACK);
		break;

	case VIEW_RECORD_4_IN_1:
		glViewport(ORIGIN_W, HALF_H + ORIGIN_H, HALF_W, HALF_H);
		draw_cam_2d(CAM_VIEW_FISHEYE, CAM_NUM_LEFT, NULL);
		glViewport(HALF_W, HALF_H + ORIGIN_H, HALF_W, HALF_H);
		draw_cam_2d(CAM_VIEW_FISHEYE, CAM_NUM_RIGHT, NULL);
		glViewport(ORIGIN_W, ORIGIN_H, HALF_W, HALF_H);
		draw_cam_2d(CAM_VIEW_FISHEYE, CAM_NUM_FRONT, NULL);
		glViewport(HALF_W, ORIGIN_H, HALF_W, HALF_H);
		draw_cam_2d(CAM_VIEW_FISHEYE, CAM_NUM_BACK, NULL);
		break;
	case FRONT_VIEW:
		glViewport(ORIGIN_W, ORIGIN_H, SCREEN_W, SCREEN_H);
		draw_cam_2d(CAM_VIEW_FISHEYE, CAM_NUM_FRONT, NULL);
		break;
	case BACK_VIEW:
		glViewport(ORIGIN_W, ORIGIN_H, SCREEN_W, SCREEN_H);
		draw_cam_2d(CAM_VIEW_FISHEYE, CAM_NUM_BACK, NULL);
		break;
	case LEFT_VIEW:
		glViewport(ORIGIN_W, ORIGIN_H, SCREEN_W, SCREEN_H);
		draw_cam_2d(CAM_VIEW_FISHEYE, CAM_NUM_LEFT, NULL);
		break;
	case RIGHT_VIEW:
		glViewport(ORIGIN_W, ORIGIN_H, SCREEN_W, SCREEN_H);
		draw_cam_2d(CAM_VIEW_FISHEYE, CAM_NUM_RIGHT, NULL);
		break;
	case VIEW_ELEAD_LAYOUT:
		ELEAD_CAR_MODE_VIEW(uiInfo, can);
		break;
	case VIEW_AVM_LAYOUT:
		AVM_VIEW(uiInfo, can);
		break;
	default:
		break;
	}
}

void set_3davminpara(avm_view_t view)
{
	switch (view)
	{
	case VIEW_START:
	case VIEW_3D_RV:
		set_inpara(AVM_VIEW_L_W, SCREEN_H);
		break;

	case VIEW_3D_LEFT_SV_2D_AVM:
	case VIEW_3D_RIGHT_SV_2D_AVM:
	case VIEW_3D_RV_2D_AVM:
	case VIEW_AUTOSYS_3D_AVM_2D_AVM:
		set_inpara(AVM_3D_W, AVM_3D_H);
		break;

	case VIEW_3D_LEFT_SV_2D_RV:
	case VIEW_3D_RIGHT_SV_2D_RV:
	case VIEW_LEFT_LIGHT_3D_SV_2D_SV:  // event
	case VIEW_RIGHT_LIGHT_3D_SV_2D_SV: // events
		set_inpara(HALF_W, SCREEN_H);
		break;

	default:
		break;
	}
}

void set_trans_camera_color(avm_ui_info_t *uiInfo, trans_camera_t *transCamera, int turnflag)
{

#if PARAM_AVM_BB_FUNCTION
	if (turnflag == 1)
	{
		calculate_color();
	}
	else
	{
		clear_color_diff();
	}
#else
	clear_color_diff();
#endif
	transCamera[0].front = 1;
	transCamera[0].back = 1;
	transCamera[0].left = 1;
	transCamera[0].right = 1;
}

void init_current_3davm_fov()
{

}


/*----------------------------------------------------

			2024/09/19 intergartion to autosys

-----------------------------------------------------*/
int id_list[MAX_PAGE_LIST_NUM] = {0};
Rectangle id_viewport[MAX_PAGE_LIST_NUM] = {0};
void GL_Function_glScissor(Rectangle viweport, VECF4 _color, int flag)
{
	glEnable(GL_SCISSOR_TEST);

	glScissor(viweport.X, viweport.Y, viweport.W, viweport.H);
	switch(flag)
	{
		case 0:
		glClearColor(_color.X, _color.Y, _color.Z, _color.W);
		glClear(GL_COLOR_BUFFER_BIT);
		break;
		case 1:
		glClear(GL_DEPTH_BUFFER_BIT);
		break;
		case 2:
		glClearColor(_color.X, _color.Y, _color.Z, _color.W);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		break;
	}
	glDisable(GL_SCISSOR_TEST);
	glClearColor(0, 0, 0, 0);
}
void Load_AVM_projectionMatrix(ePage *ePage, AVM_3D_BOWL *para, Rectangle viewport)
{
	para3Davm.inPara.height = viewport.H;
	para3Davm.inPara.width  = viewport.W;
	init_3d_avm_parameter(&para3Davm);
	reloading_3d_avm_fov(&para3Davm, viewport.W, viewport.H);
	para3Davm.exPara.theta        = para->vcam.theta;
	para3Davm.exPara.phi          = para->vcam.phi;
	para3Davm.exPara.eyeRadius    = para->vcam.eyeRadius;
	para3Davm.exPara.tmpCenter[0] = para->vcam.x_position;
	para3Davm.exPara.tmpCenter[1] = para->vcam.y_position;
	para3Davm.exPara.tmpCenter[2] = para->vcam.z_position;
	// LOGI
	// AVM_LOGI("3D AVM PARA [%f][%f][%f][%f][%f][%f]\n",
	// para3Davm.exPara.theta		  ,
	// para3Davm.exPara.phi          ,
	// para3Davm.exPara.eyeRadius    ,
	// para3Davm.exPara.tmpCenter[0] ,
	// para3Davm.exPara.tmpCenter[1] ,
	// para3Davm.exPara.tmpCenter[2] );
	rotate_callculate(&para3Davm);
}

void Load_AVM_EightTouch_projectionMatrix(ePage *ePage, Rectangle viewport, int id)
{
	global_setting *glob = &ePage->set;
	para3Davm.inPara.height = viewport.H;
	para3Davm.inPara.width  = viewport.W;
	init_3d_avm_parameter(&para3Davm);
	reloading_3d_avm_fov(&para3Davm, viewport.W, viewport.H);
	float *p = &(glob->touch.eight.theta);
	if (p != NULL)
	{
		para3Davm.exPara.theta        = p[id];
		para3Davm.exPara.phi          = glob->touch.eight.camera.phi;
		para3Davm.exPara.eyeRadius    = glob->touch.eight.camera.eyeridus;
		para3Davm.exPara.tmpCenter[0] = glob->touch.eight.camera.x;
		para3Davm.exPara.tmpCenter[1] = glob->touch.eight.camera.y;
		para3Davm.exPara.tmpCenter[2] = glob->touch.eight.camera.z;
		rotate_callculate(&para3Davm);
	}
}

static void draw_3d_car(float carAngle, AVM_CARMODEL *carmodel)
{
	camera_para_t para3Dcar = para3Davm;
	para3Dcar.exPara.tmpCenter[0] += carmodel->vcam.x_position;
	para3Dcar.exPara.tmpCenter[1] += carmodel->vcam.y_position;
	para3Dcar.exPara.tmpCenter[2] += carmodel->vcam.z_position;
	rotate_callculate(&para3Dcar);
	setting_CarModel_display(0, NULL);

	int* alphaMode = (int*)calloc(4, sizeof(int));
	memcpy(alphaMode, autosys.system_info.car_alpha_mode, sizeof(int) * 4);
	if(carmodel->OffroadMode_displayItem  != NULL && PARAM_OPENGL_OIT_BUFFER_FBO_ON)
	{
		alphaMode[2] = 1;
	}
	else if(PARAM_OPENGL_OIT_BUFFER_FBO_ON == 0)
	{
		alphaMode[0] = 0; // car_body
		alphaMode[1] = 0; // window
		alphaMode[2] = 0; // tire
		alphaMode[3] = 0; // interior
	}

	int disable_item[20]  = {0};
	disable_item[WINDOWS] = 1;

	draw_runtime_car(para3Dcar, carmodel, alphaMode, NULL);

	draw_pre_oit_real_car(para3Dcar, carmodel, alphaMode, NULL);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(
		viewport[0] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME, 
		viewport[1] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME, 
		viewport[2] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME, 
		viewport[3] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME);
	draw_runtime_car_oit(para3Dcar, carmodel, alphaMode, NULL);
	glViewport(0, 0, PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT);
	glClear(GL_DEPTH_BUFFER_BIT);
	draw_main_oit(false);
	free(alphaMode);

	#if 0 // old-version
	if(!PARAM_OPENGL_OIT_BUFFER_FBO_ON)
	{
	 	draw_runtime_car(para3Dcar, carmodel);
	}
	else
	{
		draw_pre_oit_real_car(para3Dcar, carmodel);
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glViewport(
			viewport[0] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME, 
			viewport[1] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME, 
			viewport[2] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME, 
			viewport[3] * PARAM_CARMODEL_OIT_LOSS_PANEL_VOLUME);
		draw_runtime_car_oit(para3Dcar, carmodel);
		glViewport(0, 0, PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);
		draw_main_oit();
	}
	#endif
}

void draw_PGL_line(ePage *ePage, AVM_PGL * pgl, PGL_SUPPLY *sup_pgl, Rectangle viewport)
{
	float steeringAngle = current_carAngle * -1.000f;
	steeringAngle = (steeringAngle > PARAM_DYNAMIC_PGL_MAX_ANGLE)? PARAM_DYNAMIC_PGL_MAX_ANGLE : (steeringAngle < PARAM_DYNAMIC_PGL_MIN_ANGLE)?PARAM_DYNAMIC_PGL_MIN_ANGLE : steeringAngle;
	steeringAngle = (steeringAngle == 0.0f)? 0.01f :steeringAngle;
	for(int ix = 0; ix < 4 ; ix++)
	{
		int selct_cam = sup_pgl->Select_cam[ix];
		if(selct_cam < 0) continue;
		switch(sup_pgl->mode)
		{
			case FISHEYE:
			draw_fish_line(0.01f, para3Davm.Rotate, sup_pgl->Select_cam[ix], sup_pgl->Select_style, &sup_pgl->cropped, FISHEYE_STATIC_DISTORTION, NULL, sup_pgl->rotate);
			if(sup_pgl->flag_draw_type == 1 && sup_pgl->Select_cam[ix] < 2)// && fabs(steeringAngle) >= 0.5f)
			{
				draw_fish_line(steeringAngle, para3Davm.Rotate, sup_pgl->Select_cam[ix], sup_pgl->Select_style, &sup_pgl->cropped, FISHEYE_DISTORTION, sup_pgl, sup_pgl->rotate);
			}
			break;
			case SCENE:
			{
				Aligmentdata.viewport = viewport;
				setLineAlignment(1, &Aligmentdata);
				setSceneViewCustomLineWidth(ePage->contexts.avm_scene.width_customized);
				virtual_cam* vcam = &(ePage->contexts.avm_scene.vcam[selct_cam - 2]);
				float xratio = ePage->contexts.avm_scene.view_x_axis_ratio[selct_cam - 2];
				float yratio = ePage->contexts.avm_scene.view_y_axis_ratio[selct_cam - 2];
				xratio = (xratio == 0.0f) ? 1.0f : xratio;
				yratio = (yratio == 0.0f) ? 1.0f : yratio;
				float transformMatrix[] =
				{
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1
				};
				transformMatrix[2]  = vcam->eyeRadius;
				transformMatrix[6]  = vcam->phi;
				transformMatrix[10] = vcam->theta;
				transformMatrix[12] = vcam->x_position;
				transformMatrix[13] = vcam->y_position;
				transformMatrix[11] = vcam->z_position;		
				transformMatrix[0]  = 1.0f *  xratio;
				transformMatrix[5]  = 1.0f *  yratio;
				
				// AVM_LOGI("Select vcam = [%f, %f, %f] position[%f, %f, %f] ratio[%f, %f]\n",selct_cam,
				//  vcam->eyeRadius,
				//  vcam->phi,
				//  vcam->theta,
				//  vcam->x_position,
				//  vcam->y_position,
				//  vcam->z_position,
				//  xratio,
				//  yratio);
				// AVM_LOGI("Select cam = [%d]\n",selct_cam);
				// AVM_LOGI("CurStyle = [%d]\n",sup_pgl->Select_style);
				//draw_fish_line(0.01f, transformMatrix, selct_cam, 1, &sup_pgl->cropped, FISHEYE_STATIC_SCENE, NULL);
				read_pPage_file(FISHEYE_STATIC_SCENE);
				draw_fish_line(0.01f, transformMatrix, selct_cam, 0, &sup_pgl->cropped, FISHEYE_STATIC_SCENE, NULL, 0.0f);
				//draw_fish_line(0.01f, transformMatrix, selct_cam, sup_pgl->Select_style, &sup_pgl->cropped, FISHEYE_STATIC_SCENE, NULL, 0.0f);
			}
			break;
			case AVM3D:
				draw_3D_avm_dynamicline(0.01f, para3Davm.Rotate, sup_pgl->Select_cam[ix], (sup_pgl->Select_style*2), AVM_3D_VIEW, sup_pgl);
				if(sup_pgl->flag_draw_type == 1)// && fabs(steeringAngle) >= 0.5f)
				{
					//GL_Function_glScissor(viewport, (VECF4){0.0f, 0.0f, 0.0f, 1.0f}, 1);
					glClear(GL_DEPTH_BUFFER_BIT);
					draw_3D_avm_dynamicline(steeringAngle, para3Davm.Rotate, sup_pgl->Select_cam[ix], (sup_pgl->Select_style*2) + 1, AVM_3D_VIEW, sup_pgl);
				}
			break;
			case AVM2D:
				// AVM_LOGI("select cam = [%d]\n",sup_pgl->Select_cam[ix]);
				// AVM_LOGI("select style = [%d]\n",sup_pgl->Select_style);
				Aligmentdata.viewport = viewport;
				setLineAlignment(1, &Aligmentdata);
				draw_3D_avm_dynamicline(0.01f, autosys_get_zoomin2Dprojectionmatrix(), sup_pgl->Select_cam[ix], (sup_pgl->Select_style*2), AVM_2D_VIEW, sup_pgl);
				if(sup_pgl->flag_draw_type == 1)// && fabs(steeringAngle) >= 0.5f)
				{
					//GL_Function_glScissor(viewport, (VECF4){0.0f, 0.0f, 0.0f, 1.0f}, 1);
					glClear(GL_DEPTH_BUFFER_BIT);
					setLineAlignment(0, &Aligmentdata);
					draw_3D_avm_dynamicline(steeringAngle, autosys_get_zoomin2Dprojectionmatrix(), sup_pgl->Select_cam[ix], (sup_pgl->Select_style*2) + 1, AVM_3D_VIEW, sup_pgl);
				}
			break;
		}
	}
}

void avm_view_rotate_calculate(ePage  *ePage, int page, Rectangle viewport)
{
	avm_contexts *contexts = &ePage->contexts;
	global_setting *glob = &ePage->set;
	static float total_angle     = 0;
	float startAngle      = glob->animation.para.StartPoint;
	float endAngle        = glob->animation.para.EndPoint;
	int direction         = glob->animation.para.Direction;

	int flag_360_rotate         = 0;//glob->animation.para.TurnFlag;
	int flag_animation_rotate   = glob->animation.para2.TurnanimationFlag;
	int flag_540_rotate         = glob->touch.free.enable;
	int flag_eight_rotate       = (int)glob->touch.eight.enable;

	int mode = (flag_animation_rotate << 4) + (flag_eight_rotate << 3) + (flag_540_rotate << 2) + (flag_360_rotate << 1);
	mode = (mode == 0)?None:( 1 << (31 - __builtin_clz(mode)) );
	// AVM_LOGI("CurPage[%d], 540 flag[%d],360 flag[%d], mode [%d]\n",
	// page,
	// flag_540_rotate, 
	// flag_360_rotate, 
	// mode);
	switch(mode)
	{
		case EIGHT:
		if (init_EightTouch_angle == 1)
		{
			Load_AVM_EightTouch_projectionMatrix(ePage, viewport, 0);
			//reset_EightTouch_para3Davm(1, 0);
			init_EightTouch_angle = 0;
			current_EightTouch_index = 0;
			eight.busy = 0;
		}

		if (eight.interrupt == 1)
		{
			Load_AVM_EightTouch_projectionMatrix(ePage, viewport, eight.cur_id);
			eight.interrupt = 0;
			eight.busy = 0;
			eight.level = 0;
		}

		if (eight.busy == 1)
		{
			if (eight.moving == 0)
			{
				float *p = &(glob->touch.eight.theta);
				if (p != NULL)
				{
					eight.total_angle = CalculateGap_EightTouch(p[eight.cur_id], p[eight.net_id], eight.director);
					gettimeofday(&time_360_clock[0], NULL);
					eight.moving = 1;
				}
			}
			else if(eight.level == 2)
			{
				float *p = &(glob->touch.eight.theta);
				if(eight.signal == 1)
				{
					AVM_LOGI("\033[35m[INTERRUPT] stack new position\033[0m\n");
					eight.signal = 0;
					float startAngle = para3Davm.exPara.theta;
					eight.cur_angle = startAngle;
					float endAngle = p[eight.net_id];
					float total_clockwise        = CalculateGap_EightTouch(startAngle, p[eight.net_id], 0);
					float total_counterclockwise = CalculateGap_EightTouch(startAngle, p[eight.net_id], 1);
					if(total_clockwise < total_counterclockwise)
					{
						eight.director = 0;
						eight.total_angle = total_clockwise;
					}
					else
					{
						eight.director = 1;
						eight.total_angle = total_counterclockwise;
					}
					//eight.director = checkDirection(startAngle, endAngle);
					gettimeofday(&time_360_clock[0], NULL);
					eight.moving = 1;

					AVM_LOGI("leve2 start angle [%f],", eight.cur_angle);
					AVM_LOGI("end angle [%f]\n", p[eight.net_id]);
					AVM_LOGI("level2 total angle : [%f] ,", eight.total_angle);
					AVM_LOGI("current angle is %f\n", para3Davm.exPara.theta);
				}
				gettimeofday(&time_360_clock[1], NULL);
				float processtime = calculate_time(time_360_clock[0], time_360_clock[1]);
				float angle_pres = ePage->set.touch.eight.processTime / 360.0f;
				para3Davm.exPara.theta = CalculateCurrentAngle_EightTouch(processtime, eight.cur_angle, eight.total_angle, eight.director, eight.total_angle * angle_pres);

				// AVM_LOGI("leve2 process :%f\n", processtime);
				// AVM_LOGI("leve2 start angle [%f],", eight.cur_angle);
				// AVM_LOGI("end angle [%f]\n", p[eight.net_id]);
				// AVM_LOGI("level2 total angle : [%f] ,", eight.total_angle);
				// AVM_LOGI("current angle is %f\n", para3Davm.exPara.theta);
				if (processtime >= eight.total_angle * angle_pres)
				{
					para3Davm.exPara.theta = p[eight.net_id];
					eight.busy = 0;
					eight.moving = 0;
					eight.cur_id = eight.net_id;
					eight.level = 0;
				}
				rotate_callculate(&para3Davm);
			}
			else
			{
				gettimeofday(&time_360_clock[1], NULL);
				float processtime = calculate_time(time_360_clock[0], time_360_clock[1]);
				float angle_pres = ePage->set.touch.eight.processTime / 360.0f;
				float *p = &(glob->touch.eight.theta);
				if (p != NULL)
				{
					para3Davm.exPara.theta = CalculateCurrentAngle_EightTouch(processtime, p[eight.cur_id], eight.total_angle, eight.director, eight.total_angle * angle_pres);

					// AVM_LOGI("***process :%f s ,", processtime);
					// AVM_LOGI("start angle [%f],", p[eight.cur_id]);
					// AVM_LOGI("end angle [%f],", p[eight.net_id]);
					// AVM_LOGI("total angle : [%f] ,", total_angle);
					// AVM_LOGI("current angle is %f\n", para3Davm.exPara.theta);
					if (processtime >= eight.total_angle * angle_pres)
					{
						para3Davm.exPara.theta = p[eight.net_id];
						eight.busy = 0;
						eight.moving = 0;
						eight.cur_id = eight.net_id;
						eight.level = 0;
					}
				}
				rotate_callculate(&para3Davm);
			}
		}

		lock_page_roate = 0;
		flag_360_finsih = 0;
		break;
		case FREEANGLE:
		if (lock_page_roate == 0 || g_reset_540sysyem == 1)
		{
			Load_AVM_projectionMatrix(ePage, &contexts->avm_3d_bowl, viewport);
			rotate_callculate(&para3Davm);
			lock_page_roate = 1; // 鎖住狀態
			g_reset_540sysyem = 0;
		}
		break;

		case ANIMATION:
		{
			//! only one times key*/
			if (glob->animation.para2.only_one_key == 1)
			{
				if (g_CarModel_ever_do_rotate_ones[page] == 1)
				{
					Load_AVM_projectionMatrix(ePage, &contexts->avm_3d_bowl, viewport);
					return;
				}
			}

			//* lock_page_roate is == 0, that means the page is first open*/
			if (lock_page_roate == 0)
			{
				lock_page_roate = 1; 
				InitAnimationPara(&animationData);
				Load_AVM_projectionMatrix(ePage, &contexts->avm_3d_bowl, viewport);
			}

			//* wait for animation done*/
			if(lock_page_roate == 1 && animationData.recall_level_flag == 1)
			{
				if(animationData.cur_level_flag >= (MAX_ANIMATION_NUM * 2))
				{
					EndAnimationPara(&animationData, page);
				}

				if(CalAnimationPara(glob, &animationData) == true)
				{
					animationData.cur_level_flag++;
					return;
				}
				animationData.recall_level_flag = 0;
			}

			//* calculate the angle of gap for animation*/
			if (flag_360_finsih == 0)
			{
				gettimeofday(&time_360_clock[1], NULL);
				float processtime = calculate_time(time_360_clock[0], time_360_clock[1]);
				switch(animationData.cur_level_flag % 2)
				{
					case 0:
						para3Davm.exPara.theta = CalculateCurrentAngleByValue(
							processtime, animationData.startAngle, 
							animationData.ESP_total_angle, animationData.direction, 
							animationData.timer, 360.0f);
						if (processtime >= animationData.timer)
						{
							GotoNextAnimationPara(&animationData);
							para3Davm.exPara.theta = animationData.endAngle;
						}
					break;
					case 1:
						para3Davm.exPara.phi = CalculateCurrentAngleByphi(processtime, 
						animationData.startAngle, animationData.ESP_total_angle, 
						animationData.direction, animationData.timer, 180.0f) ;

						// AVM_LOGI("direct %d, Esp_startAngle %f, totalangle %f, phi is %f\n", animationData.direction, animationData.startAngle, 
						// animationData.ESP_total_angle, para3Davm.exPara.phi);

						if (processtime >= animationData.timer)
						{
							GotoNextAnimationPara(&animationData);
							para3Davm.exPara.phi = animationData.endAngle ;
						}
						para3Davm.exPara.phi = (para3Davm.exPara.phi == 0.0f)?0.01f:para3Davm.exPara.phi;
					break;
				}
				
				if(animationData.cur_level_flag >= (MAX_ANIMATION_NUM * 2))
				{
					EndAnimationPara(&animationData, page);
				}
				
				rotate_callculate(&para3Davm);
			}
		}

		break;

		case CIRCLE:

		#if 0
		if (glob->animation.para.only_one_key == 1)
		{
			if (g_CarModel_ever_do_rotate_ones[page] == 1)
			{
				Load_AVM_projectionMatrix(ePage, &contexts->avm_3d_bowl, viewport);
				return;
			}
		}
		if (lock_page_roate == 0)
		{
			float gap;
			Load_AVM_projectionMatrix(ePage, &contexts->avm_3d_bowl, viewport);
			para3Davm.exPara.theta = startAngle;
			rotate_callculate(&para3Davm);
			lock_page_roate = 1; // 鎖住狀態
			flag_360_finsih = 0;
			gap = CalculateGap(startAngle, endAngle, direction);
			total_angle = (360.0f * (float)glob->animation.para.Cycle) + gap;
			AVM_LOGI("***Start angle is %f***\n", startAngle);
			AVM_LOGI("***Total angle is %f***\n", total_angle);
			gettimeofday(&time_360_clock[0], NULL);
		}

		if (flag_360_finsih == 0)
		{
			gettimeofday(&time_360_clock[1], NULL);
			float processtime = calculate_time(time_360_clock[0], time_360_clock[1]);
			para3Davm.exPara.theta = CalculateCurrentAngle(processtime, startAngle, total_angle, direction, glob->animation.para.Time);
			// AVM_LOGI("***process :%f s ,",processtime);
			// AVM_LOGI("current angle is %f\n",para3Davm.exPara.theta);
			if (processtime >= glob->animation.para.Time)
			{
				para3Davm.exPara.theta = glob->animation.para.EndPoint;
				flag_360_finsih = 1;
				g_CarModel_ever_do_rotate_ones[page] = 1;
			}
			rotate_callculate(&para3Davm);
		}
		#endif
		break;
		case None:
		Load_AVM_projectionMatrix(ePage, &contexts->avm_3d_bowl, viewport);
		lock_page_roate = 0;
		flag_360_finsih = 0;
		total_angle = 0.0f;
		break;
	}
}

void is_seethrough_enabled(AVMflag *flag)
{
	flag->display_seeth = (autosys.seethrough.seethrough_switch & autosys.g_flag.fuc_seethrough) == 1;
}

void is_fxaa_enabled(AVMflag *flag)
{
	flag->disaply_fxaa = (get_FXAA_ready_to_draw() & autosys.g_flag.fuc_fxaa) == 1;
    //return (get_FXAA_ready_to_draw() & autosys.g_flag.fuc_fxaa) == 1;
}

void is_mod_enabled(AVMflag *flag)
{
	flag->display_mod =  (autosys.g_flag.fuc_smod) == 1;
    //return (autosys.g_flag.fuc_smod) == 1;
}

void is_avm_3d_enabled(ePage *ePage, AVMflag *flag)
{
	flag->display_avm3d   = 1;
	flag->display_fisheye = 1;
	if(ePage->set.touch.fourteen.enable == 1.0f)
	{
    	flag->display_avm3d          = (autosys.current_info.mutiple_touch_index < 8);
		flag->display_fisheye        = !flag->display_avm3d;
    	flag->disaply_inner_sixtouch = (autosys.current_info.mutiple_touch_index >= 8);
	}
	else
	{
		flag->disaply_inner_sixtouch = false;
	}
}

void is_scene_enabled(AVMflag *flag)
{
	flag->display_scene = 1;
}

void is_carModel_shadow_line_enabled(AVMflag *flag)
{
	autosys_page *avm      = &autosys.avm_page;
    int current_page       = avm->current_page;
    ePage      *ePage      = &avm->page[current_page];
	int context_num        = ePage->set.context_num;
    avm_draw_list *list    = &ePage->list;
	CAR_MODEL_INFO* car_info = &autosys.current_info.carmodel_info;
	int ix = 0;
	bool find_carmodel_page = false;
	while(ix < context_num)
	{
	
		if(list[ix].ID == PAGE_CARMODEL)
		{
			find_carmodel_page = true;
			break;
		}
		ix++;
	}


	bool enable_shadow_line = false;
	switch(car_info->Shadow_draw_mode)
	{
		case 0:
		enable_shadow_line = false;
		break;
		case 1:
		enable_shadow_line = flag->display_seeth && autosys.system_info.seethrough_frame_read_flag;
		if(find_carmodel_page)
			enable_shadow_line &= list[ix].supply.sup_offroad.Select_middle_view_id;
		else
			enable_shadow_line = false;
		break;
		case 2:
		enable_shadow_line = flag->display_seeth && autosys.system_info.seethrough_frame_read_flag;
		if(find_carmodel_page)
			enable_shadow_line &= !list[ix].supply.sup_offroad.Select_middle_view_id;
		else
			enable_shadow_line = true;
		break;
		case 3:
		enable_shadow_line = flag->display_seeth && autosys.system_info.seethrough_frame_read_flag;
		break;
	}


	flag->display_shadow_line = enable_shadow_line;
}

void render_fxaa(ePage *ePage, Rectangle viewport, bool isEnableFxaa)
{
	viewport.Y = g_PANEL_HEIGHT - viewport.Y - viewport.H;
	glViewport(viewport.X, viewport.Y, viewport.W, viewport.H);
	float glViewport_data[4] = {viewport.Y, viewport.X, viewport.W, viewport.H};
	draw_FXAA(glViewport_data, isEnableFxaa);
}
void render_fisheye(ePage *ePage, avm_draw_list *list, Rectangle viewport, AVMflag *flag)
{
    FISHEYE_SUPPLY *supfisheye = &list->supply.sup_fisheye;
    if (!flag->disaply_fxaa)
    {
		if(flag->disaply_inner_sixtouch == true && flag->display_fisheye == true)
		{
			int8_t select_index = autosys.current_info.mutiple_touch_index - 8;
			select_index = (select_index < 0) ? 0 : select_index;
			
			draw_cam_fish(CAM_VIEW_FISHEYE,
			ePage->set.touch.fourteen.Select_cam[select_index],
			&ePage->set.touch.fourteen.cropped[select_index], 0);
		}
		else if(flag->display_fisheye == true)
		{
			draw_cam_fish(CAM_VIEW_FISHEYE,supfisheye->Select_cam, &supfisheye->cropped, supfisheye->rotate);
		}
        //draw_cam_fish(CAM_VIEW_FISHEYE, supfisheye->Select_cam, &supfisheye->cropped);
    }
	else if(flag->display_fisheye == true)
	{
		
		// glViewport(viewport.X, -viewport.Y, viewport.W, g_PANEL_HEIGHT);
		// float glViewport_data[4] = {-viewport.Y, viewport.X, viewport.W, g_PANEL_HEIGHT};

		viewport.Y = g_PANEL_HEIGHT - viewport.Y - viewport.H;
		glViewport(viewport.X, viewport.Y, viewport.W, viewport.H);
		float glViewport_data[4] = {viewport.Y, viewport.X, viewport.W, viewport.H};
        draw_FXAA(glViewport_data, true);
	}
}

void render_3d_avm(ePage *ePage, int current_page, Rectangle viewport, AVMflag *flag)
{
	if (flag->display_avm3d == false) return;
    avm_view_rotate_calculate(ePage, current_page, viewport);
    draw_3d_avm(para3Davm.Rotate, AVM_3D, AVM);
}

void render_2d_avm(ePage *ePage, AVMflag *flag)
{
	int fram_count = autosys.current_info.cur_frame;
    if (!flag->display_seeth && fram_count < 10)
    {
        draw_3d_avm(autosys_get_zoomin2Dprojectionmatrix(), AVM_2D, AVM);
		//draw_2DAVM_texture();
    }
	else
	{
		draw_2DAVM_texture();
	}
}

void render_2d_car(ePage *ePage, avm_contexts *contexts, AVMflag *flag)
{
    float alpha_value = (flag->display_seeth) ? autosys.seethrough.alpha_2d_car_value : contexts->avm_2d_car.alpha;
    draw_car_2d(alpha_value);
}

void render_3d_car(ePage *ePage, avm_contexts *contexts, avm_draw_list *list,can_bus_info_t *can, AVMflag *flag)
{
	if (flag->display_avm3d == false) return;

	contexts->avm_3d_car.Car_RotateAngle  = can->steer;
	contexts->avm_3d_car.Tire_RotateAngle = Rotateangle;
	if(list->supply.sup_offroad.Select_middle_view_id == 1)
	{
		contexts->avm_3d_car.OffroadMode_displayItem = &list->supply.sup_offroad.CarModel_view;
	}
	else
	{
		contexts->avm_3d_car.OffroadMode_displayItem = NULL;
	}

	CAR_MODEL_INFO *car_info       = &autosys.current_info.carmodel_info;
	autosys_page *avm              = &autosys.avm_page;
	if(car_info->enable_draw == 1 && car_info->Pages_key[avm->current_page] != 0)
	{
		draw_3d_avm_static_car(Rotateangle, &contexts->avm_3d_car);
	}
	else
	{
    	draw_3d_car(Rotateangle, &contexts->avm_3d_car);
	}
}

void render_pgl(ePage *ePage, avm_contexts *contexts, avm_draw_list *list, Rectangle viewport, AVMflag *flag)
{
	//AVM_LOGI("pgl mode[%d]\n",list->supply.sup_pgl.mode);
    if (!( (list->supply.sup_pgl.mode == FISHEYE || list->supply.sup_pgl.mode == SCENE) && (flag->disaply_fxaa == 1)))
    {
		glClear(GL_DEPTH_BUFFER_BIT);
		draw_PGL_line(ePage, &contexts->avm_pgl, &list->supply.sup_pgl, viewport);
    }
}

void render_shadow(ePage *ePage, avm_draw_list *list, AVMflag *flag)
{
    switch (list->supply.flag_projection_mode)
    {
        case MODE_3DAVM:
			if (flag->display_avm3d == false) return;
            if (flag->display_seeth == 1)
            {
				autosys.current_info.seethrough_frameline_flag = flag->display_shadow_line;
                autosys_cal_bowl_shadow_range();
                draw_car_shadow_seethrough(para3Davm.Rotate, autosys_get_bowl_shadow_range());
            }
            else
            {
                draw_car_shadow(para3Davm.Rotate);
            }
            break;

        case MODE_2DAVM:
            if (flag->display_seeth == 1)
            {
				autosys.current_info.seethrough_frameline_flag = 0 && autosys.system_info.seethrough_frame_read_flag;
				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                draw_2DAVM_texture();
				autosys_cal_bowl_shadow_range();
                draw_car_shadow_seethrough(autosys_get_zoomin2Dprojectionmatrix(), autosys_get_bowl_shadow_range());
				//draw_car_shadow_process_seethrough(NULL, autosys_get_bowl_shadow_range());
				glDisable(GL_BLEND);
            }
            else
            {
                draw_car_shadow(autosys_get_zoomin2Dprojectionmatrix());
            }
            break;
    }
}

void render_mod(ePage *ePage, AVMflag *flag)
{
	if(flag->display_mod == 0) return;
	draw_mod();
}

void render_scene(ePage *ePage, avm_draw_list *list, Rectangle viewport, AVMflag *flag)
{
	if (!flag->disaply_fxaa)
{
    int ix = list->supply.sup_scene.Select_virtualCam_id;
	//AVM_LOGI("[Show] scene %d\n", ix);
    virtual_cam* camview = &(ePage->contexts.avm_scene.vcam[ix]);
	float xratio = ePage->contexts.avm_scene.view_x_axis_ratio[ix];
	float yratio = ePage->contexts.avm_scene.view_y_axis_ratio[ix];
	xratio = (xratio == 0.0f) ? 1.0f : xratio;
	yratio = (yratio == 0.0f) ? 1.0f : yratio;
	float transformMatrix[] =
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1};

	memcpy(paraCorrectEye.Model, transformMatrix, sizeof(float) * 16);

	paraCorrectEye.Model[2]  = 0.0f + camview->eyeRadius;//(float)MODE_VIEW[cam_ch].eyeRadius[screen][0];
	paraCorrectEye.Model[6]  = 0.0f + camview->phi;//(float)MODE_VIEW[cam_ch].phi[screen][0];
	paraCorrectEye.Model[10] = 0.0f + camview->theta;//(float)MODE_VIEW[cam_ch].theta[screen][0];
	paraCorrectEye.Model[12] = 0.0f + camview->x_position;//(float)MODE_VIEW[cam_ch].cam_x[screen][0];
	paraCorrectEye.Model[13] = 0.0f + camview->y_position;//(float)MODE_VIEW[cam_ch].cam_y[screen][0];
	paraCorrectEye.Model[11] = 0.0f + camview->z_position;//(float)MODE_VIEW[cam_ch].cam_z[screen][0];
	paraCorrectEye.Model[0]  = 1.0f *  xratio;
	paraCorrectEye.Model[5]  = 1.0f *  yratio;
	// AVM_LOGI("[Show] paraCorrectEye.Model[0] %f\n", xratio);
	// AVM_LOGI("[Show] paraCorrectEye.Model[5] %f\n", yratio);
		draw_cam_correct_2d(&paraCorrectEye.Model[0], CAM_VIEW_CORRECT, ix + 2, list->supply.sup_scene.Style);
}
	else if(flag->display_scene == true)
	{
		viewport.Y = g_PANEL_HEIGHT - viewport.Y - viewport.H;
		glViewport(viewport.X, viewport.Y, viewport.W, viewport.H);
		float glViewport_data[4] = {viewport.Y, viewport.X, viewport.W, viewport.H};
		draw_FXAA(glViewport_data, false);
	}
}

void flash_car_system(can_bus_info_t *can)
{
	current_carAngle = can->steer;
}
void AVM_VIEW(avm_ui_info_t *uiInfo, can_bus_info_t *can)
{
    autosys_page *avm      = &autosys.avm_page;
    int current_page       = avm->current_page;
    ePage      *ePage      = &avm->page[current_page];

    avm_draw_list *list    = &ePage->list;
    avm_contexts *contexts = &(ePage->contexts);
    int context_num        = ePage->set.context_num;
    int ix = 0;

	/*---------------------flag intergartion------------*/
	is_seethrough_enabled(&avm->flag);
	is_fxaa_enabled(&avm->flag);
	is_mod_enabled(&avm->flag);
	is_avm_3d_enabled(ePage, &avm->flag);
	is_scene_enabled(&avm->flag);
	/*---------------------end--------------------------*/
	flash_seethrough_texture();
	flash_car_system(can);
	Rectangle previewport = {0, 0, 0, 0};
	while (ix < context_num && ix < MAX_PAGE_CONTENTS)
    {
		int ID = list[ix].ID;
        Rectangle viewport = list[ix].viewport;
		if((viewport.X != previewport.X) || (viewport.Y != previewport.Y) ||
		   (viewport.W != previewport.W) || (viewport.H != previewport.H))
		{
			Rectangle viewport = list[ix].viewport;
			viewport.Y = g_PANEL_HEIGHT - viewport.Y - viewport.H;
			glViewport(viewport.X, viewport.Y, viewport.W, viewport.H);
			previewport = viewport;
		}

		#if DEBUG_SHOWN_CURRENT_PAGE_COMPONECT
		id_list[ix]     = ID;
		id_viewport[ix] = viewport;
		#endif
        switch (ID)
        {
            case PAGE_FISHEYE:
				render_fxaa(ePage, viewport, true);
                //render_fisheye(ePage, &list[ix], viewport, &avm->flag);
                break;
            case PAGE_3DAVM:
				render_fxaa(ePage, viewport, false);
                //render_3d_avm(ePage, current_page, viewport, &avm->flag);
                break;
            case PAGE_2DAVM:
				render_fxaa(ePage, viewport, false);
                //render_2d_avm(ePage, &avm->flag);
                break;
            case PAGE_2DCAR:
                //render_2d_car(ePage, contexts, &avm->flag);
                break;
            case PAGE_CARMODEL:
                //render_3d_car(ePage, contexts, &list[ix], can, &avm->flag);
                break;
            case PAGE_PGL:
                //render_pgl(ePage, contexts, &list[ix], viewport,  &avm->flag);
                break;
            case PAGE_SHADOW: //or render seethrough
                //render_shadow(ePage, &list[ix], &avm->flag);
                break;
            case PAGE_MOD:
                render_mod(ePage, &avm->flag);
                break;
			case PAGE_SCENE:
				render_fxaa(ePage, viewport, false);
				//render_scene(ePage, &list[ix], viewport, &avm->flag);
				break;
        }
        ix++;
    }

	#if DEBUG_SHOWN_CURRENT_PAGE_COMPONECT

	if(autosys.current_info.notify_show_cur_page_context == 0) return;
	const char *item[] =
		{
    		"PAGE_START",
    		"PAGE_3DAVM",
    		"PAGE_2DAVM",
    		"PAGE_SHADOW",
    		"PAGE_2DCAR",
    		"PAGE_CARMODEL",
    		"PAGE_PGL",
    		"PAGE_MOD",
    		"PAGE_FISHEYE",
    		"PAGE_SCENE",
    		"PAGE_TOUCH",
    		"PAGE_CARMODEL_STATIC",
    		"PAGE_EIGHT",
    		"PAGE_END",
		};
	autosys.current_info.notify_show_cur_page_context = 0;
	ix = 0;
	AVM_LOGI("----------------------------------------------------------------\n");
	while(ix < context_num && ix < MAX_PAGE_CONTENTS)
	{
		AVM_LOGI("ld:%d [%s] view[%d,%d,%d,%d]\n",ix, item[id_list[ix]],
		id_viewport[ix].X,
		id_viewport[ix].Y,
		id_viewport[ix].W,
		id_viewport[ix].H);
		ix++;
	}
	AVM_LOGI("----------------------------------------------------------------\n");
	#endif
}

/*----------------------------------------------------

					2024/09/19 END

-----------------------------------------------------*/
