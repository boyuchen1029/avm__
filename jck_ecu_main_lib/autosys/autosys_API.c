#include "autosys_API.h"
#include <dirent.h> 
#include "../avm_module/include/program/program_car.h"
#include <string.h>
/*==================================================================

                usr set carmodel parameter

*==================================================================*/
#pragma region CARMODEL



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
void usr_carModel_static_Draw_Page(autosys_usr_control control,int *Pages, int num)
{
    CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;
    memset(car_info->Pages_key, 0, sizeof(car_info->Pages_key));
    switch(control)
    {
        case SET_INFO:
            if(Pages == NULL || num <= 0) 
            {
                AVM_LOGI("[USER] car model static draw page is invalid\n");
                car_info->Pages_key_num = 0;
                return;
            }
            else
            {
                AVM_LOGI("[USER] Seting car model static draw page is ");
                memcpy(car_info->Pages_key, Pages, sizeof(int) * num);
                memcpy(car_info->Pages_Chosen, Pages, sizeof(int) * num);
                car_info->Pages_key_num = num;
                for(int i = 0; i < num; i++)
                {
                    car_info->Pages_key[Pages[i]] = 1;
                    AVM_LOGI("%d ", Pages[i]);
                }
                car_info->currentPage = -1;
                AVM_LOGI("\n");
                init_static_car_fbo();
            }
        break;
        case GET_INFO:
            AVM_LOGI("[USER] Get car model static draw page is ");
            for(int i = 0; i < 10; i++)
            {
                if(car_info->Pages_key[i] == 1)
                {
                    AVM_LOGI("%d ", i);
                }
            }
            AVM_LOGI("\n");
        break;
    }
}

/**
 * @author alfred
 * @date 2023/09/26
 * @brief ivi to contorl carModel color
 * @param
 */

void usr_set_default_color(int color)
{
	if (color < 0)
		return;

	CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;
	car_info->color_style = color;
	AVM_LOGI("Usr setting init car color is %d\n", car_info->color_style);
}

void usr_seethrough_2D_car_alpha_value(autosys_usr_control control, float value)
{
    tmp_seethrough *seethroughbuffer = &autosys.seethrough;
    if(autosys.g_flag.fuc_user_control == 0) return;
    if(seethroughbuffer->alpha_2d_car_value == value) return;
    AVM_LOGI("[USER] Seting seethrough 2d car alpha is %f\n", value);
    switch(control)
    {
        case SET_INFO:
            seethroughbuffer->alpha_2d_car_value = value;
        break;
        case GET_INFO:
            AVM_LOGI("[USER] Seting seethrough 2d car alpha is %f\n", seethroughbuffer->alpha_2d_car_value);
        break;
    }
}

void usr_seethrough_3D_car_alpha_value(autosys_usr_control control, float value)
{
    tmp_seethrough *seethroughbuffer = &autosys.seethrough;
    if(autosys.g_flag.fuc_user_control == 0) return;
    if(seethroughbuffer->alpha_3d_car_value == value) return;
    AVM_LOGI("[USER] Seting seethrough alpha_3d_car_value is %f\n", value);
    switch(control)
    {
        case SET_INFO:
            seethroughbuffer->alpha_3d_car_value = value;
        break;
        case GET_INFO:
            AVM_LOGI("[USER] Seting seethrough 2d car alpha is %f\n", seethroughbuffer->alpha_3d_car_value);
        break;
    }
}

void usr_seethrough_3D_car_tire_alpha_value(autosys_usr_control control, float value)
{
    tmp_seethrough *seethroughbuffer = &autosys.seethrough;
    if(autosys.g_flag.fuc_user_control == 0) return;
    if(seethroughbuffer->alpha_3d_car_tire_value == value) return;
    AVM_LOGI("[USER] Seting seethrough alpha_3d_car_tire_value is %f\n", value);
    switch(control)
    {
        case SET_INFO:
            seethroughbuffer->alpha_3d_car_tire_value = value;
        break;
        case GET_INFO:
            AVM_LOGI("[USER] Seting seethrough 3d car tire alpha is %f\n", seethroughbuffer->alpha_3d_car_tire_value);
        break;
    }
}

void usr_seethrough_3D_car_windows_alpha_value(autosys_usr_control control, float value)
{
    tmp_seethrough *seethroughbuffer = &autosys.seethrough;
    if(autosys.g_flag.fuc_user_control == 0) return;
    if(seethroughbuffer->alpha_3d_car_windows_value == value) return;
    AVM_LOGI("[USER] Seting seethrough alpha_3d_car_windows_value is %f\n", value);
    switch(control)
    {
        case SET_INFO:
            seethroughbuffer->alpha_3d_car_windows_value = value;
        break;
        case GET_INFO:
            AVM_LOGI("[USER] Seting seethrough 3d car windows alpha is %f\n", seethroughbuffer->alpha_3d_car_windows_value);
        break;
    }
}
/**
 * @brief Sets or retrieves the positional tuning parameters of the see-through blending area in AVM.
 * @param control Indicates whether to set or retrieve the parameters.
 * @param shift Positional offset. Range: [-0.05, 0.05], default: 0.0.
 * @param scale Scaling factor. Range: [0.95, 1.05], default: 1.0.
 * @note Values outside the supported range will be clamped automatically.
 * @author Alfred
 * @date 2026/03/16
 */
void usr_seethrough_frame_TuneArea(autosys_usr_control control, float shift, float scale)
{
    shift = clampf(shift, -0.05f, 0.05f);
    scale = clampf(scale, 0.95f, 1.05f);
    AVM_LOGI("[USER] Seting seethrough tune_shift is %f, tune_scale is %f\n", shift, scale);
    tmp_seethrough *seethroughbuffer = &autosys.seethrough;

    switch(control)
    {
        case SET_INFO:
            seethroughbuffer->tune_shift = shift;
            seethroughbuffer->tune_scale = scale;
        break;
        case GET_INFO:
            AVM_LOGI("[USER] Seting seethrough tune_shift is %f, tune_scale is %f\n", seethroughbuffer->tune_shift, seethroughbuffer->tune_scale);
        break;
    } 
}

/**
 * @author alfred
 * @date 2023/11/01
 * @brief ivi to contorl carModel open/close door
 * @param
 */
void usr_set_3DcarModel_doorStatus(uint8_t rb_dr, uint8_t lb_dr, uint8_t rf_dr, uint8_t lf_dr, uint8_t trunk)
{
    static int m_currentStatusIx = 0;
	int target = 0;

	target |= (rb_dr << 0);
	target |= (lb_dr << 1);
	target |= (rf_dr << 2);
	target |= (lf_dr << 3);
	target |= (trunk << 4);
	if (m_currentStatusIx != target)
	{
        static_car_signal_colorChange();
		m_currentStatusIx = target;
	}
	//if (m_isStatusChange)
	{
		CAR_MODEL_INFO *carinfo = &autosys.current_info.carmodel_info;
		for(int ix = 0; ix < 5; ix++)
		{
			//carinfo->car2d_status[ix] = (target & (0x01 << ix)) ? 1 : 0;
			carinfo->car3d_status[ix] = (target & (0x01 << ix)) ? 1 : 0;
		}
		//upload_selection_car_color(carinfo->color_style);
		//m_isStatusChange = 0;
	}

}
int m_2d_currentStatusIx = 0;
int m_2d_isStatusChange  = 0;
void usr_set_2DcarModel_doorStatus(uint8_t rb_dr, uint8_t lb_dr, uint8_t rf_dr, uint8_t lf_dr, uint8_t trunk)
{
	int target = 0;

	target |= (rb_dr << 0);
	target |= (lb_dr << 1);
	target |= (rf_dr << 2);
	target |= (lf_dr << 3);
	target |= (trunk << 4);
    if (m_2d_currentStatusIx != target)
	{
	 	m_2d_currentStatusIx = target;
	 	m_2d_isStatusChange = 1;
	}
	if (m_2d_isStatusChange)
	{
		CAR_MODEL_INFO *carinfo = &autosys.current_info.carmodel_info;
        AVM_INFO *avminfo       = &autosys.current_info.avm_info;
		for(int ix = 0; ix < 5; ix++)
		{
			carinfo->car2d_status[ix] = (target & (0x01 << ix)) ? 1 : 0;
            avminfo->door_status[ix] = (target & (0x01 << ix)) ? 1 : 0;
			//carinfo->car3d_status[ix] = (target & (0x01 << ix)) ? 1 : 0;
		}
		upload_selection_car_color(carinfo->color_style);
		m_2d_isStatusChange = 0;
	}
}

/**
 * @brief ivi to contorl carModel shadow draw mode
 * @param status draw mode 
 * @param[in] 0: any view not draw shadow panel \n
 * @param[in] 1: only offroad draw shadow panel \n
 * @param[in] 2: except offroad draw shadow panel \n
 * @param[in] 3: all view draw shadow panel \n
 * @note This function is used to enable or disable carModel shadow draw mode.
 */
void usr_set_carModel_shadow_drawMode(int status)
{
    CAR_MODEL_INFO* current_info = &autosys.current_info.carmodel_info;
    AVM_LOGI("[USER]PreValue drawMode is %d, SetValue drawMode is %d\n", current_info->Shadow_draw_mode, status);
    current_info->Shadow_draw_mode = status;
}



/**
 * @brief Sets or retrieves the parameters of the lamp flow animation in the 3D car model.
 * @param control: Control option to determine whether to set or get the lamp flow animation parameters.
 *                  Use `SET_INFO` to set the parameters and `GET_INFO` to retrieve the current parameters.
 * @param enable: Enable or disable the lamp flow animation. Bits 0, 1, and 2 control the front, rear, and side lamps respectively.
 * @param speed: Sets the speed of the lamp flow animation.
 * @param max_length: Sets the maximum length of the lamp flow animation.
 * @note : This function is used to set or retrieve the parameters of the lamp flow animation in the 3D car model.
 */
void usr_3dcarModel_lamp_flowing_para(autosys_usr_control control, uint8_t enable, uint8_t speed, int max_length)
{
    #define FLOW0_BIT 0   // front 對應 bit0
    #define FLOW1_BIT 1   // rear  對應 bit1
    #define FLOW2_BIT 2   // side  對應 bit2
    CAR_MODEL_INFO* current_info = &autosys.current_info.carmodel_info;
    switch(control)
    {
        case SET_INFO:
            current_info->flowing_lamp_enable[0] = (enable >> FLOW0_BIT) & 1; // front
            current_info->flowing_lamp_enable[1] = (enable >> FLOW1_BIT) & 1; // rear
            current_info->flowing_lamp_enable[2] = (enable >> FLOW2_BIT) & 1; // side
            char* typeName[] = {"Front", "Rear", "Side"};
            char FolderPath[256] = {0};
            sprintf(FolderPath, "%sEVSTable/car_model/flowing/", EVSTable_path[0]);
            DIR *dir;
            struct dirent *entry;
            dir = opendir(FolderPath);
            if (dir != NULL)
            {
                for (int ix = 0; ix < 3; ix++)
                {
                    if (current_info->flowing_lamp_enable[ix] == 1)
                    {
                        bool is_exist = false;
                        AVM_LOGI("[USER] Seting 3dcarModel lamp %s enable\n", typeName[ix]);
                        while ((entry = readdir(dir)) != NULL) {
                            if(strncmp(entry->d_name, typeName[ix], strlen(typeName[ix])) == 0)
                            {
                                is_exist = true;
                                break;
                            }
                        }

                        if(is_exist)
                        {
                            AVM_LOGI("[USER]Successfully find flowing lamp %s\n", typeName[ix]);
                        }
                        else
                        {
                            AVM_LOGI("[USER][WARNING]Failed to find flowing lamp %s\n", typeName[ix]);
                        }

                        rewinddir(dir);
                    }
                }
                closedir(dir);
            }
            else
            {
                AVM_LOGI("[USER][WARNING]Failed to open flowing lamp folder in %s\n", FolderPath);
            }
            current_info->flowing_lamp_speed      = speed * 5;
            current_info->flowing_lamp_max_length = max_length;
        break;
        case GET_INFO:
            AVM_LOGI("[USER] Seting 3dcarModel lamp para is [%d,%d,%d]\n",
            current_info->flowing_lamp_enable[0],
            current_info->flowing_lamp_speed,
            current_info->flowing_lamp_max_length);
        break;
    }
}
#pragma endregion
/*==================================================================

                usr set camera warining

*==================================================================*/
#pragma region CAMERA
void usr_set_warning_init_Fisheye_position(float top, float left, float w, float h)
{

}

void usr_set_warning_init_Top2D_positionF(float top, float left, float w, float h)
{

}

void usr_set_warning_init_Top2D_positionB(float top, float left, float w, float h)
{

}

void usr_set_warning_init_Top2D_positionL(float top, float left, float w, float h)
{

}
void usr_set_warning_init_Top2D_positionR(float top, float left, float w, float h)
{

}
#pragma endregion
/*==================================================================

                usr set touch parameter

*==================================================================*/


void usr_set_freetouch_limit(int page, int buff[4])
{
	int cur_page = autosys.avm_page.current_page;
	if (page != -1)
		cur_page = page;
	printf("\n\nset buff %d, %d,%d,%d,%d\n", cur_page, buff[0], buff[1], buff[2], buff[3]);
    FREETOUCH *free = &autosys.avm_page.page[cur_page].set.touch.free;
	memcpy(&free->limit, buff, sizeof(int) * 4);
}

void usr_set_click_parameter(float processTime, int during_count)
{
    autosys.system_info.click_during_Count = during_count;
    autosys.system_info.click_process_Time = processTime;
    AVM_LOGI("[USER] Seting click parameter [%f,%d]\n", processTime, during_count);
}
/*==================================================================

                usr set touch parameter

*==================================================================*/
#pragma region SEETHROUGH AREA

float usr_get_seethroughprossValue()
{
    return autosys.current_info.seethrough_process;
}


/**
 * @brief Sets or retrieves the seethrough resolution level.
 * @param control: Control option to determine whether to set or get the seethrough resolution level.
 * @param rank: resoultion (0: original version, 1: 2x resolution)
 *         the rank should be between 0 and 1
 * @note This function is used to adjust the seethrough resoultion effect for AVM.
 * @author Alfred
 * @date 2026/03/17
 */
void usr_seethrough_resoultion_level(autosys_usr_control control, float level)
{
    static bool is_init_seethrough_resoultion_level = false;
    autosys_system_info *system = &autosys.system_info;
    if(is_init_seethrough_resoultion_level == true) 
    {
        AVM_LOGI("[USER] Seting seethrough already setup and the value\n");
        return;
    }

    if(autosys.current_info.flag_system_init == 1)
    {
        AVM_LOGI("[USER][WARNING] AVM has been initialized\n");
        return;
    }
    level = clampf(level, 0.0f, 0.7f);
    AVM_LOGI("[USER] Seting seethrough resoultion_level is [%f]\n", level);
    switch(control)
    {
        case SET_INFO:
            is_init_seethrough_resoultion_level = true;
            system->seethrough_resolution_level = level;
        break;
        case GET_INFO:
            AVM_LOGI("[USER] Geting seethrough resoultion_level is [%f]\n", system->seethrough_resolution_level);
        break;
    }
}
/**
 * @brief : Sets or retrieves the see-through blending level and range for the AVM.
 * @param control: Control option to determine whether to set or get the blending level and range.
 * @param level: Smoothness of the connection (0: almost not smooth, 1: smooth)
 * @param range: Blending range ranging from 0.0 (no blending) to 1.0 (long blending area).
 * @note : This function is used to adjust the see-through blending effect for AVM.     
 * @author : Alfred
 * @date : 2026/03/16
 */
void usr_seethrough_alpha_level(autosys_usr_control control, float level, float range)
{
    //level is [0,1] closing the 1 is become soomth but picture is blur long, range is [0,1] blending area, 0 is no blending
    autosys_system_info *system = &autosys.system_info;
    AVM_LOGI("[USER] Seting seethrough blending level is [%f], range is [%f]\n", level, range);
    const float range_max = 60.0f;
    const float level_max = 10.0f;
    level = clampf(level, 0.01f, 1.0f);
    range = clampf(range, 0.01f, 1.0f);
    switch(control)
    {
        case SET_INFO:
            system->seethrough_sigmoid_alpha      = level_max * level;
            system->seethrough_top2d_alpha_center = 70.0f / 130.0f;
            system->seethrough_top2d_alpha_range  = (range_max * range) / 130.0f;
        break;
        case GET_INFO:
            level = system->seethrough_sigmoid_alpha / level_max;
            range = system->seethrough_top2d_alpha_range * 130.0f / range_max;
            AVM_LOGI("[USER] Geting seethrough blending level is [%f], range is [%f]\n", level, range);
        break;
    }
}
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
void usr_get_zoom_status_position(int* out_row, int * out_col)
{
    zoom_birdView *zoom     = &autosys.zoom;
    if(zoom->firs_select_block == 255)  
    {
        *out_row = -1;
        *out_col = -1;
    }
    else
    {

        *out_row = autosys.zoom.firs_select_block / (zoom->block_size_col);
        *out_col = autosys.zoom.firs_select_block % (zoom->block_size_col);
    }
}

/**
 * @brief Resets the zoom-in state for the Top2D view.
 * @param control The control option to determine whether to set or get the zoom-in state.
 * @note This function will reset the zoom-in state for the Top2D view if the control option is SET_INFO.
 * @author Alfred
 * @date 2026/05/02
 */
void usr_avm_zoomIn_reset(autosys_usr_control control)
{
    if(control != SET_INFO) return;
    AVM_LOGI("[USER] Reset ZoomIn for Top2D\n");
    autosys_set_zoomin2DprojectionmartixReset();
}
#pragma endregion
/*==================================================================

                usr get version

*==================================================================*/
#pragma region VERSION AREA
/**
 * @brief Retrieves the version of the libautosys library combined with the current date.
 *
 * This function returns a string containing the version number of the libautosys 
 * library in the format "v<MAJOR>-<MINOR>-00 <DATE>", where <MAJOR> and <MINOR> 
 * are version numbers defined by VERSION_MAJOR and VERSION_MINOR macros, and 
 * <DATE> is the current date at the time of compilation.
 *
 * @return A pointer to a static character array containing the version information.
 */

char* usr_get_libautosys_version()
{
    static char avm_version_with_datetime[50];
	snprintf(avm_version_with_datetime, sizeof(avm_version_with_datetime), 
             "v%d-%02d-00  %s", VERSION_MAJOR, VERSION_MINOR, __DATE__);
    return avm_version_with_datetime;
}
#pragma endregion


/*==================================================================

                usr AVM Status

*==================================================================*/
#pragma region AVM Status

/**
 * @brief Set the AVM opendoor view mode.
 *
 * @param view_flag Set the view mode. If set to -1, the opendoor view will be turned off.
 * If set to 0, the opendoor view will use the default color of (0.5, 0.5, 0.5).
 * If set to any other value, the opendoor view will use the custom color specified by the color parameter.
 * @param color The custom color to be used for the opendoor view if view_flag is not -1 or 0.
 * The color is a float array of length 3, with each element ranging from 0.0 to 1.0.
 */
void usr_set_avm_opendoor_view(int view_flag, float color[3])
{
    AVM_INFO* current_info = &autosys.current_info.avm_info;
    if(view_flag == -1)
    {
        current_info->draw_opendoor_mode = -1;
        memcpy(current_info->opendoor_color, color, sizeof(float) * 3);
        AVM_LOGI("[USER] AVM opendoor view OFF\n");
    }
    else if (view_flag == 0)
    {
        float default_color[3] = {0.5f, 0.5f, 0.5f};
        current_info->draw_opendoor_mode = 1;
        memcpy(current_info->opendoor_color, default_color, sizeof(float) * 3);
        AVM_LOGI("[USER] AVM opendoor view Default Mode\n");
        AVM_LOGI("[USER] Setting Opoendoor color is %f, %f, %f\n", current_info->opendoor_color[0], current_info->opendoor_color[1], current_info->opendoor_color[2]);
    }
    else
    {
        current_info->draw_opendoor_mode = 1;
        memcpy(current_info->opendoor_color, color, sizeof(float) * 3);
        AVM_LOGI("[USER] AVM opendoor view customzied Mode\n");
        AVM_LOGI("[USER] Setting Opoendoor color is %f, %f, %f\n", current_info->opendoor_color[0], current_info->opendoor_color[1], current_info->opendoor_color[2]);
    }
}

#pragma endregion