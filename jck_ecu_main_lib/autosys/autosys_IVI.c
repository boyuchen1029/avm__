#include "autosys_IVI.h"
#include "../../system.h"
#include "../avm_module/include/avm/fp_source.h"
#include "../avm_module/include/program/program_main.h"
#include "program/program_car.h"

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

#if(PLAFORM != NT98690)
#include <linux/videodev2.h>
#include <linux/input.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

#pragma region DEBUG

stCalibrationStatus g_calibration_info;
/**
 * @brief Function to check the IVI status
 *
 * @param status The status of the IVI, the range is [1, 10]
 *
 * @return void
 *
 * @details This function is used to check the IVI status, the current status is represented by the status variable.
 * The function will print out the current status of the IVI, the global setting, the detail setting, the AVM setting, and the zoom in setting.
 */
void DEBUG_CHECK_NECESSARY_IVI_STATUS(int status)
{
	int flag_read_file = 0;
	#ifdef READ_FILE
	flag_read_file = 1;
	#else
	flag_read_file = 0;
	#endif

	autosys_current_info *curInfo = &autosys.current_info;

	if(status >= 1)
	{
		int fd_autosys = open("/dev/autosys_avm_cmd", O_RDWR);
		int buffer[12] = {0};
		ioctl(fd_autosys, IVI_get_STATUS_info, &buffer);
		buffer[6] = 0;
		ioctl(fd_autosys, IVI_set_STATUS_info, &buffer);
		close(fd_autosys);
	}

	if(status == 1)
	{
		/*
		* Print Debug Message	
		*/
		printf("\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("[DEBUG] CHECK_NECESSARY_IVI_STATUS list\n");
		AVM_LOGI("***************************************\n");

		AVM_LOGI("*\t\033[32m global setting\033[0m *\t\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("*.[PAGE]         \t[%d]...*\n", autosys.avm_page.current_page); 
		AVM_LOGI("2.[READFILE]     \t[%d]...2\n", flag_read_file);
		AVM_LOGI("3.[MOD] 	       \t[%d]...3\n", autosys.g_flag.fuc_smod);
		AVM_LOGI("4.[SEETHROUGH]   \t[%d]...4\n", autosys.seethrough.seethrough_switch);
		AVM_LOGI("5.[BBLANCE]      \t[%d]...5\n", autosys.g_flag.fuc_avm_bb);
		AVM_LOGI("6.[ZOOMIN]       \t[%d]...6\n", autosys.g_flag.fuc_zoom_birdView);
		AVM_LOGI("7.[OFFLINEMODE]  \t[%d]...7\n", READ_OFFLINE_SHADER);
		AVM_LOGI("8.[CANBUS_MODE]  \t[%d]...8\n", curInfo->flag_show_canbus_log);
		AVM_LOGI("9.[AVM]          \t[%d]...9\n", 0);
		AVM_LOGI("10.[IVI_contorl] \t[%d]...10\n", curInfo->flag_disable_ivi_control);
	}

	if(status == 10)
	{
		curInfo->flag_disable_ivi_control = !curInfo->flag_disable_ivi_control;
	}

	if (status == 5)
	{
		AVM_LOGI("***************************************\n");
		AVM_LOGI("*\t\033[32m detail setting\033[0m \t*\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("*\t\033[33m BB setting\033[0m \t*\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("1.[BB] PID     =  [%d]...1\n", curInfo->BB_PID_Latency);
		AVM_LOGI("2.[BB] Version =  [%d]...2\n", curInfo->BB_VERSION );
	}
	if(status == 510 && curInfo->flag_disable_ivi_control == 1)
	{
		curInfo->BB_VERSION = 0;
	}
	if(status == 511 && curInfo->flag_disable_ivi_control == 1)
	{
		curInfo->BB_VERSION = 1;
	}

	if(status == 51 && curInfo->flag_disable_ivi_control == 1)
	{
		autosys.g_flag.fuc_avm_bb = !autosys.g_flag.fuc_avm_bb;
	}

	if(status == 30 && curInfo->flag_disable_ivi_control == 1)
	{
		autosys.g_flag.fuc_smod = 0;
	}
	if(status == 31 && curInfo->flag_disable_ivi_control == 1)
	{
		autosys.g_flag.fuc_smod = 1;
	}
	if(status == 32 && curInfo->flag_disable_ivi_control == 1)
	{
		autosys.g_flag.fuc_freeze = !autosys.g_flag.fuc_freeze;
	}


	if(status == 8)
	{
		curInfo->flag_show_canbus_log = !curInfo->flag_show_canbus_log;
	}

	if (status == 9)
	{
		int buffer[12] = {0};
		int fd_autosys = open("/dev/autosys_avm_cmd", O_RDWR);
		ioctl(fd_autosys, IVI_get_calibration_info, &buffer);
		close(fd_autosys);
		AVM_LOGI("***************************************\n");
		AVM_LOGI("*\t\033[32m detail setting\033[0m \t*\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("*\t\033[33m AVM setting\033[0m \t*\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("1.[AVM] PID          =  [%d]...1\n", curInfo->BB_PID_Latency);
		AVM_LOGI("2.[AVM] F_angle      =  [%d]...2\n", 0);
		AVM_LOGI("3.[AVM] OpenDoorMode =  [%d]...3\n", curInfo->avm_info.draw_opendoor_mode);
		AVM_LOGI("4.[AVM] CalibStatus  =  [%d]...4\n", buffer[1]);
		//AVM_LOGI("3.[AVM] frame   =  [%d]...3\n", curInfo->cur_frame);
	}

	if(status == 93)
	{
		float opendoor_color[] = {0.5f, 0.5f, 0.5f};
		int tmpV = curInfo->avm_info.draw_opendoor_mode;
		tmpV  = (tmpV + 1 + 1) % 3 - 1;
		curInfo->avm_info.draw_opendoor_mode = tmpV;
		if(tmpV == 1)
		{
			opendoor_color[0] = 0.5f;
			opendoor_color[1] = 0.5f;
			opendoor_color[2] = 1.0f;
		}
		memcpy(curInfo->avm_info.opendoor_color, opendoor_color, sizeof(float) * 3);
	}

	if(status == 6)
	{
		zoom_birdView *zoom     = &autosys.zoom;
		int row = 0, col = 0;
		if(zoom->firs_select_block == 255)  
    	{
		    row = -1;
	        col = -1;
    	}
    	else
    	{
        	row = autosys.zoom.firs_select_block / (zoom->block_size_col);
        	col = autosys.zoom.firs_select_block % (zoom->block_size_col);
    	}
		AVM_LOGI("***************************************\n");
		AVM_LOGI("*\t\033[32m detail setting\033[0m \t*\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("*\t\033[33m ZOOMIN setting\033[0m \t*\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("1.[ZOOMIN] Select_block  =  [%d]...1\n", zoom->firs_select_block);
		AVM_LOGI("2.[ZOOMIN] Select_row    =  [%d]...2\n", row);
		AVM_LOGI("3.[ZOOMIN] Select_col    =  [%d]...3\n", col);
		AVM_LOGI("4.[ZOOMIN] Cur_level     =  [%d]...4\n", zoom->cur_level);
	}
}
#pragma endregion


/**
 * @author alfred
 * @date 2024/12/30
 * @brief avm call ivi
 * @param
 */
void __AVM_CALL_STATUS_IVI__(int index, int value)
{
	int fd_autosys = open("/dev/autosys_avm_cmd", O_RDWR);
	int buffer[12] = {0};
	ioctl(fd_autosys, IVI_get_STATUS_info, &buffer);
	buffer[index] = value;
	ioctl(fd_autosys, IVI_set_STATUS_info, &buffer);
	close(fd_autosys);
}
/**
 * @author alfred
 * @date 2024/01/22
 * @brief avm call ivi
 * @param
 */
void __AVM_CALL_CONTORL_IVI__(int index, int value)
{
	int fd_autosys = open("/dev/autosys_avm_cmd", O_RDWR);
	int buffer[12] = {0};
	ioctl(fd_autosys, 0x28, &buffer);
	buffer[index] = value;
	ioctl(fd_autosys, 0x25, &buffer);
	close(fd_autosys);
}
/**
 * @author alfred
 * @date 2024/01/22
 * @brief avm call ivi
 * @param[in] status Calibration status (0: normal, 1: calibration, 2: calibration_fail, 3: appdata_fail)
 * @param[in] start_idx Start index of data to be written
 * @param[in] data Data to be written
 * @param[in] len Length of data
 * @param[in] Isinit Initialize flag (0: init, 1: read, 2: write)
 */
void __AVM_CALL_CALIBRATION_IVI__(int status, int start_idx, int* data, int len, int Isinit)
{
	// Open the autosys device
	int fd_autosys = open("/dev/autosys_avm_cmd", O_RDWR);

	// Create a buffer to store the data
	int buffer[12] = {0};

	// Handle the different cases
	if (Isinit == 0)
	{
		// Set calibration info
		ioctl(fd_autosys, IVI_set_calibration_info, &buffer);
	}
	else if (Isinit == 1)
	{
		// Get calibration info
		ioctl(fd_autosys, IVI_get_calibration_info, &buffer);
		// Set the status
		buffer[0 + 1] = status;
		// Set calibration info
		ioctl(fd_autosys, IVI_set_calibration_info, &buffer);
	}
	else
	{
		// Get calibration info
		ioctl(fd_autosys, IVI_get_calibration_info, &buffer);
		// Set the status
		buffer[0 + 1] = status;
		// Copy the data
		memcpy(&buffer[0 + start_idx], data, sizeof(int) * len);
		// Set calibration info
		ioctl(fd_autosys, IVI_set_calibration_info, &buffer);
	}

	// Close the autosys device
	close(fd_autosys);
}

void __IVI_calibration_status_update__(stCalibrationStatus* status)
{
	if(status->flag_initData == 0)
	{
		__AVM_CALL_CALIBRATION_IVI__(0, 0, NULL, 0, 0);
		status->flag_initData = 1;
	}
	else if(status->flag_emergency == true)
	{
		AVM_LOGI(LOG_RED("[WARNING] AppData has issue\n"));
		__AVM_CALL_CALIBRATION_IVI__(status->status, 2, status->app_info.buffer, 7, 2);
	}
	else if(status->flag_emergency == false && status->flag_goingruntime == true && status->status >= ADNORMAL_FLASH)
	{
		__AVM_CALL_CALIBRATION_IVI__(status->status, 2, status->flash_info.buffer, 3, 2);
	}
	else if(status->flag_emergency == false && status->flag_goingruntime == true)
	{
		__AVM_CALL_CALIBRATION_IVI__(status->status, 0, NULL, 7, 1);
	}
}

void __IVI_Calibration_Status_Init__()
{
    g_calibration_info.flag_initData = 0;
    g_calibration_info.call_back_calibration_status = __IVI_calibration_status_update__;
    g_calibration_info.call_back_calibration_status(&g_calibration_info);
}
/**
 * @author alfred
 * @date 2023/04/28
 * @brief ivi to contorl PGL-draw style
 * @param
 */
void __IVI_SET_PGL_STYLE__(int page, int PGL_style)
{
	// printf("current set PGL style is %d\n", PGL_style);
	if (page <= -1)
	{
		page = autosys.avm_page.current_page;
	}
	if (PGL_style < 0 || PGL_style >= maxPGLStyle)
	{
		printf("set PGL style error\n");
		return;
	}
	
	#if false
	int cur_page = autosys.avm_page.current_page;
	autosys.avm_page.page[cur_page].contexts.avm_pgl.vcam;
	for(int ix = 0; ix < autosys.avm_page.page[cur_page].set.context_num; ix++)
	{
		avm_draw_list* lists = &autosys.avm_page.page[cur_page].list;
		if((int)lists[ix].ID == PAGE_PGL)
		{
			lists[ix].supply.sup_pgl.Select_style = PGL_style;
		}
	}
	#endif
	//MODE_VIEW[page].style = PGL_style;
}

/**
 * @brief Save the current frame buffer to a file
 * @param[in] status Calibration status (0: normal, 1: calibration, 2: calibration_fail, 3: appdata_fail)
 * @note This function will be called when the calibration status is 1 or greater than 1.
 * If the calibration status is 1, this function will save the frame buffer to a file named "AVMframe_buffer_<count>.rgba".
 * If the calibration status is greater than 1, this function will reset the frame buffer save count to 0 and save the frame buffer to a file named "AVMframe_buffer_0.rgba".
 */
static int m_write_frame_buffer_conut = 0;
void __IVI_SET_WRITECUR_FRAMEBUFFER__(int status)
{
	if(status >= 1 && m_write_frame_buffer_conut > 0)
	{
		int fd_autosys = open("/dev/autosys_avm_cmd", O_RDWR);
		int buffer[12] = {0};
		ioctl(fd_autosys, IVI_get_STATUS_info, &buffer);
		buffer[8] = 0;
		ioctl(fd_autosys, IVI_set_STATUS_info, &buffer);
		close(fd_autosys);
		/*
		* Print Debug Message	
		*/
		printf("\n");
		AVM_LOGI("***************************************\n");
		AVM_LOGI("[DEBUG] WRITE_FRAME_BUFFER\n");
		AVM_LOGI("***************************************\n");

		char dataname[1024];
		sprintf(dataname, "AVMframe_buffer_%d.rgba", m_write_frame_buffer_conut);
		int frame_4to1 = autosys.frame_fbo[FBO_4TO1].framebuffer;
		glBindFramebuffer(GL_FRAMEBUFFER, frame_4to1);
		fwrite_readpixels(g_PANEL_WIDTH, g_PANEL_HEIGHT, dataname);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_4to1);
		printf("\033[31m current save screen frame buffer \033[0m\n");
		system("sync");
		m_write_frame_buffer_conut += 1;
	}
	else if(status >= 1 && m_write_frame_buffer_conut == 0)
	{
		int fd_autosys = open("/dev/autosys_avm_cmd", O_RDWR);
		int buffer[12] = {0};
		ioctl(fd_autosys, IVI_get_STATUS_info, &buffer);
		buffer[8] = 0;
		ioctl(fd_autosys, IVI_set_STATUS_info, &buffer);
		close(fd_autosys);
		m_write_frame_buffer_conut++;
	}
}


void IVI_lamp_decode(int total, int *front, int *rear, int *side)
{
	*front = total & 0xFF;
	*rear = (total >> 8) & 0xFF;
	*side = (total >> 16) & 0xFF;
}


/**
 * @brief Set CarModel lamp status signal
 * @param signal : Lamp of status for single
 *
 * @details This function is used to set the lamp status of CarModel.
 * It will check if the current lamp status is different from the previous one.
 * If it is, it will update the lamp status and set the flag to indicate that the lamp status has been changed.
 */
CAR_LAMP_UPLOAD_INFO g_car_lamp_upload_info[Classify_LAMP_max][2] = {0};
void __IVI_SET_CARMODEL_LAMPSTATUS_SIGNAL__(int signal)
{
	static int m_firstRun = 1;
	if(m_firstRun == 1)
	{
		m_firstRun = 0;
		CAR_MODEL_INFO* current_info = &autosys.current_info.carmodel_info;
		for(int idx = 0; idx < Classify_LAMP_max; idx++)
		{
			for(int sub_idx = 0; sub_idx < 2; sub_idx++)
			{
				g_car_lamp_upload_info[idx][sub_idx].cur_idx = 1;
				g_car_lamp_upload_info[idx][sub_idx].pre_idx = -1;
				g_car_lamp_upload_info[idx][sub_idx].flowing_enable = current_info->flowing_lamp_enable[idx];
				g_car_lamp_upload_info[idx][sub_idx].flag_flowing_img_exist = 1;
				g_car_lamp_upload_info[idx][sub_idx].signal = -1;
			}
		}
		init_flowing_init();
	}
	static int m_LampisChange = 0;

	int front, rear, side;
	IVI_lamp_decode(signal, &front, &rear, &side);
	int lamp_status_mode[3] = {front, rear, side};
	for (int idx = 0; idx < Classify_LAMP_max; idx++)
	{
		for (int sub_idx = 0; sub_idx < 2; sub_idx++)
		{
			int Precode = lamp_status_mode[idx];
			int lrside  = (Precode & (0x01 << (6 + sub_idx))) != 0;
			int status  = Precode & ~(0x01 << 6 | 0x01 << 7);
			if (lrside != 0 && g_car_lamp_upload_info[idx][sub_idx].signal != status)
			{
				m_LampisChange = 1;
				g_car_lamp_upload_info[idx][sub_idx].signal = status;
				g_car_lamp_upload_info[idx][sub_idx].flag_flowing_img_exist = 1;
				gettimeofday(&g_car_lamp_upload_info[idx][sub_idx].time_start, NULL);
			}
			else
			{
				if (g_car_lamp_upload_info[idx][sub_idx].flowing_enable == 1)
				{
					if(g_car_lamp_upload_info[idx][sub_idx].flag_flowing_img_exist == 1)
					{
						m_LampisChange = 1;
					}
					else
					{
						m_LampisChange = 0;
					}	
				}
				else
				{
					m_LampisChange = 0;
				}
			}

			if (m_LampisChange == 1)
			{
				status = g_car_lamp_upload_info[idx][sub_idx].signal;
				int _API_lamp_status_ = status + (sub_idx) * (TotalLampIMG); // left is 0, right is 32;
				upload_car_lamp_status(idx, 0, _API_lamp_status_); //realCar
				m_LampisChange = 0;
			}
		}
	}

#if 0
	if (m_LampisChange)
	{
		//AVM_LOGI("AVM signal %d\n", signal);
		for (int lamp_obj = 0; lamp_obj < Classify_LAMP_max; lamp_obj++)
		{
			int Precode = lamp_status_mode[lamp_obj];
			int rside = (Precode & (0x01 << 6)) != 0;
			int lside = (Precode & (0x01 << 7)) != 0;
			int whichside[] = {lside, rside};
			int status = Precode & ~(0x01 << 6 | 0x01 << 7);
			for (int ix = 0; ix < 2; ix++)
			{
				if (whichside[ix] == 0)
					continue;

				int _API_lamp_status_ = status + (ix) * (32) * (whichside[ix]); // left is 0, right is 32;
				upload_car_lamp_status(0, _API_lamp_status_);
				upload_car_lamp_status(1, _API_lamp_status_);
			}
		}
		m_LampisChange = 0;
	}
#endif
}

#pragma region GET
/**
 * @brief Get the camera status information from autosys_avm_info.
 *
 * @param[in] autosys_avm_info : Pointer to autosys_avm_info structure.
 *
 * This function will get the camera status information from autosys_avm_info.
 * It will get the front, back, left and right camera status.
 */
void __IVI_GET_CAMERA_STATUS_(autosys_avm_info *autosys_avm_info)
{
	int front_camera_flag = autosys_avm_info->cam_status.front[0];
	int back_camera_flag  = autosys_avm_info->cam_status.back[0];
	int left_camera_flag  = autosys_avm_info->cam_status.left[0];
	int right_camera_flag = autosys_avm_info->cam_status.right[0];

	//g_avm_camera_flag = (front_camera_flag) | (back_camera_flag << 1) | (left_camera_flag << 2) | (right_camera_flag << 3);
}

#pragma endregion


#pragma region RESET


/**
 * @brief Resets the seethrough status to the current one.
 *
 * This function is used to reset the seethrough status to the current one.
 * It will check if the current seethrough status is different from the previous one.
 * If it is, it will update the previous seethrough status and call the static_car_signal_colorChange function.
 * @param autosys_avm_info : Pointer to autosys_avm_info structure.
 */
void __IVI_RESET_SEETHROUGH__(autosys_avm_info *autosys_avm_info)
{
	static int pre_seethrough_status = -1;
	if(pre_seethrough_status != autosys_avm_info->TRN_ON_OFF)
	{
		pre_seethrough_status = autosys_avm_info->TRN_ON_OFF;
		static_car_signal_colorChange();
	}
}

#pragma endregion