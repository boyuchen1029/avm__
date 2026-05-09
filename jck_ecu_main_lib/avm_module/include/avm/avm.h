/**
 *******************************************************************************
 * @file     : avm.h
 * @describe : Header for avm.c file
 * 
 * @author : Woody.
 * @verstion : 0.1.0.
 * @date 20211108 0.1.0 Woody.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/

#pragma once
#ifndef _AVM_H_
#define _AVM_H_

#ifdef __cplusplus
//extern "C" {
#endif
#include "../user_control/user_command.h"
#include "canbus.h"
#include "avmDefine.h"

struct autosys_avm_info
{
	int avm_view_page;
	float vehicle_speed;
	int gear;
	int speed_decide;
	int car_color;
	int touch_x_H;
	int touch_x_L;
	int touch_y_H;
	int touch_y_L;
	int vehicle_alpha;
	float steering_angle;
	int door_signal;
	int light_signal;
	int turn_signal_flag;
	int PGL_style_flag;
	int TRN_ON_OFF;
	int mod_ON_OFF;
	int mod_recall;
	int mod_warring_flag[4];
	int brightness_adj;
	int AVM_PID_Latancy;
	int rotate_540_reset_flag;
	struct 
	{
		/* data */
		int front[4];
		int back[4];
		int left[4];
		int right[4];
	}cam_status;
	struct 
	{
		int left_front_car_door ;
		int left_rear_car_door  ;
		int right_front_car_door ;
		int right_rear_car_door ;
		int trunk_car_door      ;
		/* data */
	}car_model_door;
	int muti_touchIndex;
	int ZOOM_ON_OFF;
};

// typedef enum
// {
//     GET_INFO,
//     SET_INFO,
// }autosys_usr_control;
// struct can_frame
// {
//     int can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
//     int data[11];
// };

extern void init_avm(const char dir[]);

//extern void routine_avm(user_command_mode_t * user_command, can_bus_info_t *canBus ,autosys_avm_info _autosys_avm_info);
//extern void routine_avm(user_command_mode_t *user_command, can_bus_info_t *canBus,struct autosys_avm_info _autosys_avm_info,struct  can_frame * _AVM_set_MOD_info);
extern void GOTOFULLVIEW(uint8_t CameraSelected);
extern void GOTOTOUCHVIEW(stTouchData *TouchDt);
extern void panel_fbo(const GLuint Framebuf);
extern void usr_get_current_rotate_angle(float* theta, float* phi, float *radiuseye);
extern void usr_set_freetouch_limit(int page, int buff[4]);
extern void usr_set_click_parameter(float processTime, int during_count);
extern void usr_get_zoom_status_position(int* out_row, int * out_col);
extern void usr_set_current_3D_carModel_alpha(int alpha, int reset);
extern void usr_get_current_panel_resolution(int *panel_width, int *panel_height);
extern void usr_set_default_color(int color);
extern void read_avm_sysyem_file(const char dir[]);

extern void usr_set_warning_init_img_position(const char dir[]);
extern void usr_set_warning_init_Fisheye_position(float top, float left, float w, float h);
extern void usr_set_warning_init_Top2D_positionF(float top, float left, float w, float h);
extern void usr_set_warning_init_Top2D_positionB(float top, float left, float w, float h);
extern void usr_set_warning_init_Top2D_positionL(float top, float left, float w, float h);
extern void usr_set_warning_init_Top2D_positionR(float top, float left, float w, float h);
//extern void Get_common_toChangePageSetting(int page,unsigned char mode, void* buffer);

extern void usr_seethrough_2D_car_alpha_value(autosys_usr_control contorl, float value);
extern void usr_seethrough_3D_car_alpha_value(autosys_usr_control contorl, float value);
extern void usr_seethrough_3D_car_tire_alpha_value(autosys_usr_control control, float value);
extern void usr_seethrough_3D_car_windows_alpha_value(autosys_usr_control control, float value);
extern void usr_set_2DcarModel_doorStatus(uint8_t rb_dr, uint8_t lb_dr, uint8_t rf_dr, uint8_t lf_dr, uint8_t trunk);
extern void usr_set_3DcarModel_doorStatus(uint8_t rb_dr, uint8_t lb_dr, uint8_t rf_dr, uint8_t lf_dr, uint8_t trunk);
extern void usr_set_carModel_shadow_drawMode(int status);
/**
 * @brief : Retrieves the current progress of the transparent chassis feature.
 * @return : A float value representing the progress percentage.
 *           - The returned value ranges from 0 to 100.
 *           - Note: Once the value reaches 100, it will no longer be updated.
 * @note : Use this function to monitor the progress of the transparent chassis.
 *         Ensure appropriate handling for cases where the progress reaches 100.
 * @author : Alfred
 * @date : 2024/10/11
 */
extern float usr_get_seethroughprossValue();

extern void DEBUG_CHECK_NECESSARY_IVI_STATUS(int status);
extern void __IVI_SET_WRITECUR_FRAMEBUFFER__(int status);
extern char* usr_get_libautosys_version();
/**
 * @brief Copy the 4-to-1 panel FBO data to the provided GLuint array.
 * @param Data A GLuint array of size 3 to store the FBO data.
 *             Data[0] will store the value of framebuffer,
 *             Data[1] will store the value of texture,
 *             Data[2] will store the value of depthRenderbuffer.
 */
extern void get_panel_4to1_fbo(GLuint *Data);

extern void panel_depth(const GLuint Depthbuf);

/**
 * @brief Set the AVM opendoor view mode.
 *
 * @param view_flag Set the view mode. If set to -1, the opendoor view will be turned off.
 * If set to 0, the opendoor view will use the default color of (0.5, 0.5, 0.5).
 * If set to any other value, the opendoor view will use the custom color specified by the color parameter.
 * @param color The custom color to be used for the opendoor view if view_flag is not -1 or 0.
 * The color is a float array of length 3, with each element ranging from 0.0 to 1.0.
 */
void usr_set_avm_opendoor_view(int view_flag, float color[3]);
/**
 * @brief usr_carModel_static_Draw_Page
 * @param control SET_INFO or GET_INFO
 * @param Pages array of page index, start from 0
 * @param num the number of page index in Pages
 * @note This function is used to set or get car model static draw page.
 *        If control is SET_INFO, it will set car model static draw page.
 *        If control is GET_INFO, it will get current car model static draw page.
 * @author : Alfred
 * @date : 2024/11/01
 */
void usr_carModel_static_Draw_Page(autosys_usr_control control,int *Pages, int num);
extern void init_static_car_fbo();
#ifdef __cplusplus
//}
#endif

#endif
