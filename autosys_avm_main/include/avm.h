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

typedef enum
{
    GET_INFO,
    SET_INFO,
}autosys_usr_control;

typedef enum
{
    FW_FRONT = 1,
    FW_REAR  = 2,
    FW_SIDE  = 4,
}lamp_flowing;

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
	int rotate_540_reset_flag;
	int AVM_PID_Latancy;
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
// struct can_frame
// {
//     int can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
//     int data[11];
// };

extern void init_avm(const char dir[]);

// extern void routine_avm(user_command_mode_t * user_command, can_bus_info_t *canBus ,autosys_avm_info _autosys_avm_info);
extern void routine_avm(user_command_mode_t *user_command, can_bus_info_t *canBus,autosys_avm_info _autosys_avm_info,struct  can_frame * _AVM_set_MOD_info);
extern void GOTOFULLVIEW(uint8_t CameraSelected);
extern void GOTOTOUCHVIEW(stTouchData *TouchDt);
extern void panel_fbo(const GLuint Framebuf);

/**
 * @brief Retrieve the current AVM rotation angle.
 * @param input theta, phi, radiuseye (each parameter can be null)
 * @param output If the variable is not null, its value will be updated.
*/
extern void usr_get_current_rotate_angle(float* theta, float* phi, float *radiuseye);
/**
 * @brief Allows the user to set the free touch limit.
 * @param input 
 *        	 page: The target page to set. If set to -1, it indicates the desire to set the current page.
 *        	 limitBuffer[4]: An array.
 *           limitBuffer[0-1]: Define the start and end limits for the phi parameter.
 *           limitBuffer[2-3]: Define the start and end limits for the theta parameter.
 * @param output null
 * @note This function should be placed within the redraw function to function correctly.
 * @warning Ensure correct page and limit values are provided to avoid unexpected behaviors.
*/
extern void usr_set_freetouch_limit(int page, int buff[4]);
/**
 * @brief : Sets the click parameters, including maximum time between two clicks and touch counter limit.
 * @param processTime: The maximum allowed time between two consecutive clicks, in seconds.
 * @param during_count: The counter limit for each touch interaction.
 *
 * This function updates the system information with the given click parameters. The parameters include
 * the maximum time allowed between two clicks and the number of times the touch action is held or counted.
 * This configuration is used in determining the system's click handling behavior.
 *
 * @note : This function affects the click handling for the entire system and should be used
 *         to adjust parameters for any user-initiated click events.
 * @author : Alfred
 * @date : 2024/05/02
 */
extern void usr_set_click_parameter(float processTime, int during_count);
/**
 * @brief Get the current block position of zoom bird view.
 *
 * @param[out] out_row The row of the selected block.
 * @param[out] out_col The column of the selected block.
 *
 * If the zoom bird view is not selected or not available,
 * the function will return -1 for both out_row and out_col.
 *
 * @note The row and column is 0-based.
 */
extern void usr_get_zoom_status_position(int* out_row, int * out_col);

extern void usr_set_current_3D_carModel_alpha(int alpha, int reset);
//extern void Get_common_toChangePageSetting(int page,unsigned char mode, void* buffer);

/**
 * @brief :  usr_get_current_Panel_resoultion
 * @param output:  panel resoultion;
 * @note : the value from avm system.config, please used this method after read_avm_system_file()
 * @author : Alfred
 * @date : 2023/11/21
 */
extern void usr_get_current_panel_resolution(int *panel_width, int *panel_height);

/**
 * @brief :  usr_set_default_color
 * @param output:  default carmodel color;
 * @note : the value from user setting, if the value is smaller than zero, the value will not be set up. and defalt value is read_avm_system_file;
 * @author : Alfred
 * @date : 2024/05/02
 */
extern void usr_set_default_color(int color);

extern void read_avm_sysyem_file(const char dir[]);

/**
 * @brief : usr_set_warning_init_img_position
 * @param dir: the directory path of the image
 * @note : Sets the image path for the warning initialization. Memory is allocated for the path. 
 * @author : Alfred
 * @date : 2024/05/02
 */
extern void usr_set_warning_init_img_position(const char dir[]);
/**
 * @brief : usr_set_warning_init_Fisheye_position
 * @param top: top position of the fisheye
 * @param left: left position of the fisheye
 * @param w: width of the fisheye area
 * @param h: height of the fisheye area
 * @note : Initializes the fisheye position parameters for the warning system. Values are set based on user input.
 * @author : Alfred
 * @date : 2024/05/02
 */
extern void usr_set_warning_init_Fisheye_position(float top, float left, float w, float h);
/**
 * @brief : usr_set_warning_init_Top2D_positionF,B,L,R
 * @param top: top position of the 2D top view for camera
 * @param left: left position of the 2D top view for camera
 * @param w: width of the 2D top view area for camera
 * @param h: height of the 2D top view area for camera
 * @note : Sets the 2D top view position parameters for the camera in the warning system.
 * @author : Alfred
 * @date : 2024/05/02
 */
extern void usr_set_warning_init_Top2D_positionF(float top, float left, float w, float h);
extern void usr_set_warning_init_Top2D_positionB(float top, float left, float w, float h);
extern void usr_set_warning_init_Top2D_positionL(float top, float left, float w, float h);
extern void usr_set_warning_init_Top2D_positionR(float top, float left, float w, float h);


extern void usr_seethrough_2D_car_alpha_value(autosys_usr_control contorl, float value);
extern void usr_seethrough_3D_car_alpha_value(autosys_usr_control contorl, float value);
extern void usr_seethrough_3D_car_tire_alpha_value(autosys_usr_control control, float value);
extern void usr_seethrough_3D_car_windows_alpha_value(autosys_usr_control control, float value);

/**
 * @brief : Sets the door status for the car model in both 2D and 3D views.
 *          This status determines whether each door (right-back, left-back, right-front, left-front)
 *          and trunk are open or closed. The 2D and 3D functions store the current status and update
 *          the display when there is a change.
 * 
 * @param rb_dr: Status of the right-back door (1 for open, 0 for closed).
 * @param lb_dr: Status of the left-back door (1 for open, 0 for closed).
 * @param rf_dr: Status of the right-front door (1 for open, 0 for closed).
 * @param lf_dr: Status of the left-front door (1 for open, 0 for closed).
 * @param trunk: Status of the trunk (1 for open, 0 for closed).
 * 
 * @note : This function is primarily used to adjust the appearance of the car model
 *         on the display based on the door and trunk status, updating only when a change is detected.
 * 
 * @date : 2024/11/01
 */
void usr_set_2DcarModel_doorStatus(uint8_t rb_dr, uint8_t lb_dr, uint8_t rf_dr, uint8_t lf_dr, uint8_t trunk);
void usr_set_3DcarModel_doorStatus(uint8_t rb_dr, uint8_t lb_dr, uint8_t rf_dr, uint8_t lf_dr, uint8_t trunk);

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
/**
 * @brief   Retrieve the libautosys version information string.
 * @return  char*  
 *          - Pointer to a static, null-terminated string in the format  
 *            "v<MAJOR>-<MINOR>-00 <BUILD_DATE> <BUILD_TIME>".  
 *          - Example: "v1-03-00 Apr  5 2025 14:32:10".  
 *          - The returned pointer remains valid for the lifetime of the program and must not be freed.  
 * @note    The version string is generated at compile time using the __DATE__ and __TIME__ macros.  
 *          This function is thread-safe.  
 * @author  Alfred
 * @date    2025/07/04
 */
extern char* usr_get_libautosys_version();

/**
 * @brief Copy the 4-to-1 panel FBO data to the provided GLuint array.
 * @param Data A GLuint array of size 3 to store the FBO data.
 *             Data[0] will store the value of framebuffer,
 *             Data[1] will store the value of texture,
 *             Data[2] will store the value of depthRenderbuffer.
 */
extern void get_panel_4to1_fbo(GLuint *Data);


void panel_depth(const GLuint Depthbuf);
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
 * @brief ivi to contorl carModel shadow draw mode
 * @param status draw mode 
 * @param[in] 0: any view not draw shadow panel \n
 * @param[in] 1: only offroad draw shadow panel \n
 * @param[in] 2: except offroad draw shadow panel \n
 * @param[in] 3: all view draw shadow panel \n
 * @note This function is used to enable or disable carModel shadow draw mode.
 */
extern void usr_set_carModel_shadow_drawMode(int status);
/**
 * @brief Sets or retrieves the parameters of the lamp flow animation in the 3D car model.
 * @param control: Control option to determine whether to set or get the lamp flow animation parameters.
 *                  Use `SET_INFO` to set the parameters and `GET_INFO` to retrieve the current parameters.
 * @param enable: Enable or disable the lamp flow animation. Bits 0, 1, and 2 control the front, rear, and side lamps respectively.
 * @param speed: Sets the speed of the lamp flow animation.
 * @param max_length: Sets the maximum length of the lamp flow animation.
 * @note : This function is used to set or retrieve the parameters of the lamp flow animation in the 3D car model.
 */
extern void usr_3dcarModel_lamp_flowing_para(autosys_usr_control control, uint8_t enable, uint8_t speed, int max_length);
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
extern void usr_carModel_static_Draw_Page(autosys_usr_control control,int *Pages, int num);
/**
 * @brief Sets or retrieves the seethrough resolution level.
 * @param control: Control option to determine whether to set or get the seethrough resolution level.
 * @param rank: resoultion (0: original version, 1: 2x resolution)
 *         the rank should be between 0 and 1
 * @note This function is used to adjust the seethrough resoultion effect for AVM.
 * @author Alfred
 * @date 2026/03/17
 */
extern void usr_seethrough_resoultion_level(autosys_usr_control control, float level);
/**
 * @brief Resets the zoom-in state for the Top2D view.
 * @param control The control option to determine whether to set or get the zoom-in state.
 * @note This function will reset the zoom-in state for the Top2D view if the control option is SET_INFO.
 * @author Alfred
 * @date 2026/05/02
 */
extern void usr_avm_zoomIn_reset(autosys_usr_control control);
/**
 * @brief Sets or retrieves the positional tuning parameters of the see-through blending area in AVM.
 * @param control Indicates whether to set or retrieve the parameters.
 * @param shift Positional offset. Range: [-0.05, 0.05], default: 0.0.
 * @param scale Scaling factor. Range: [0.95, 1.05], default: 1.0.
 * @note Values outside the supported range will be clamped automatically.
 * @author Alfred
 * @date 2026/03/16
 */
extern void usr_seethrough_frame_TuneArea(autosys_usr_control control, float shift, float scale);
#ifdef __cplusplus
//}
#endif

#endif
