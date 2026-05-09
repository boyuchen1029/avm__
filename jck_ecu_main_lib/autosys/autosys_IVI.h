#ifndef _AUTOSYS_IVI_H_
#define _AUTOSYS_IVI_H_

#include "autosys.h"

#define IVI_set_AVM_view_page 0x05
#define AVM_get_AVM_view_page 0x06

#define IVI_set_DVR_view_page 0x07
#define AVM_get_DVR_view_page 0x08

#define IVI_set_SYSTEM_info 0x09
#define AVM_get_SYSTEM_info 0x0A
#define AVM_set_SYSTEM_info 0x0B

#define IVI_set_MOD_info 0x0D
#define IVI_get_MOD_info 0x0E

#define AVM_set_MOD_info 0x0F
#define AVM_get_MOD_info 0x10

#define IVI_set_IMS_info 0x11
#define IVI_get_IMS_info 0x12

#define AVM_set_IMS_info 0x13
#define AVM_get_IMS_info 0x14

#define IVI_set_BSD_info 0x15
#define IVI_get_BSD_info 0x16

#define AVM_set_BSD_info 0x17
#define AVM_get_BSD_info 0x18

#define IVI_set_ODA_info 0x19
#define IVI_get_ODA_info 0x1A

#define AVM_set_ODA_info 0x1B
#define AVM_get_ODA_info 0x1C

#define IVI_set_LDWS_info 0x1D
#define IVI_get_LDWS_info 0x1E

#define AVM_set_LDWS_info 0x1F
#define AVM_get_LDWS_info 0x20

#define IVI_set_turn_signal_info 0x21
#define AVM_get_turn_signal_info 0x22

#define IVI_set_PGL_style_info 0x23
#define AVM_get_PGL_style_info 0x24

#define IVI_set_CONTRL_info 0x25
#define IVI_get_CONTRL_info 0x26

#define AVM_set_CONTRL_info 0x27
#define AVM_get_CONTRL_info 0x28

#define AVM_set_360_status 0x29
#define IVI_get_360_status 0x30

#define IVI_set_STATUS_info 0x3A
#define IVI_get_STATUS_info 0x3B

#define IVI_set_CONTRL_CarModel_info 0x41
#define IVI_get_CONTRL_CarModel_info 0x42

#define IVI_set_CONTRL_touch_info 0x43
#define IVI_get_CONTRL_touch_info 0x44

#define IVI_set_calibration_info 0x47
#define IVI_get_calibration_info 0x48

/*==============================================================
*                     Autosys calibration status
===============================================================*/

typedef enum 
{
    NON_CALIBRATION,
    CALIBRATION,
    ADNORMAL_FLASH,
    ADNORMAL_APP
}calibration_status;

typedef enum
{
    DATA_SAFE    = 0,
    DATA_NONFIND_OR_EMPTY = 2,
    DATA_NO_FIND = 2,
    DATA_EMPTY   = 2,
};

typedef struct stCalibrationStatus
{
    calibration_status status;
    unsigned char flag_emergency;
    unsigned char flag_goingruntime;
    unsigned char flag_initData;
    union 
    {
        int buffer[3];
        struct 
        {
           int avm_file;
           int pgl_file;
           int goldensample;
        };
        
    }flash_info;

    union  
    {
        int buffer[7];
        struct 
        {
           int avm_file;
           int car_model_file;
           int config;
           int golden;
           int input_img;
           int pgl;
           int sceneView;
        };
    }app_info;

    void (*call_back_calibration_status)(struct stCalibrationStatus *);
}stCalibrationStatus;

typedef struct
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
	} cam_status;
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
} autosys_avm_info;

extern stCalibrationStatus g_calibration_info;
extern CAR_LAMP_UPLOAD_INFO g_car_lamp_upload_info[3][2];

#define UPDATE_CALIB_STATUS(current, base) (((current) > (base)) ? (current) : (base))

#define SET_STATUS_CALIBRATION_IVI(target_field, signal, isforce, itemstatus) \
    do {                                                                       \
        g_calibration_info.status = UPDATE_CALIB_STATUS(g_calibration_info.status, (signal)); \
        g_calibration_info.flag_emergency = (isforce);                         \
        g_calibration_info.app_info.target_field |= (itemstatus);             \
        if (g_calibration_info.call_back_calibration_status) {                \
            g_calibration_info.call_back_calibration_status(&g_calibration_info); \
        }                                                                      \
    } while (0)


/*==============================================================
*                     Autosys funtion for IVI
===============================================================*/
extern void DEBUG_CHECK_NECESSARY_IVI_STATUS(int status);
extern void __AVM_CALL_STATUS_IVI__(int index, int value);
extern void __AVM_CALL_CONTORL_IVI__(int index, int value);
extern void __IVI_SET_PGL_STYLE__(int page, int PGL_style);
extern void __IVI_SET_WRITECUR_FRAMEBUFFER__(int status);
extern void __IVI_Calibration_Status_Init__();
extern void __IVI_SET_CARMODEL_LAMPSTATUS_SIGNAL__(int signal);
extern void __IVI_GET_CAMERA_STATUS_(autosys_avm_info *autosys_avm_info);
/**
 * @brief Resets the seethrough status to the current one.
 *
 * This function is used to reset the seethrough status to the current one.
 * It will check if the current seethrough status is different from the previous one.
 * If it is, it will update the previous seethrough status and call the static_car_signal_colorChange function.
 * @param autosys_avm_info : Pointer to autosys_avm_info structure.
 */
extern void __IVI_RESET_SEETHROUGH__(autosys_avm_info *autosys_avm_info);
#endif //_AUTOSYS_IVI_H_
