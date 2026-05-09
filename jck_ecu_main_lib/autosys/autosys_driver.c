#include "autosys_driver.h"
#include "autosys_IVI.h"
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
/*==================================================================

                touch driver parameter

*==================================================================*/
stTouchData touchdata;
static struct timeval start, end;
int touch_count = 0;

float calculate_time_secondunit(struct timeval start, struct timeval end)//sec
{
    float dtime;
    dtime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)* 0.000001;
    return dtime;
}
/**
 * @author : Alfred
 * @date : 2023/02/03
 */
void init_parameterPanel()
{
	touchdata.Key = 0;
	touchdata.isFirstcount = 1;
}

/**
 * @author : Alfred
 * @date : 2023/02/03
 */

int AxisOffsetTo2DAVM()
{
	/*
	screen view 1280 * 720
	AVM to screen view 540*720
	item to AVM view
	*/

#if 0
	Get_Panel_Axis(&touchdata);
#endif

#if DEBUG_MOVE_PANELVIEW
	static int flagtimer = 0;

#if FACKPOINT1 == 1
	int xtable[12] = {632, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640};
	int ytable[12] = {400, 500, 600, 650, 600, 300, 200, 100, 50, 100, 150, 300};
#endif

#if FACKPOINT1 == 2
	int xtable[12] = {600, 650, , 700, 800, 850, 900, 950, 1000, 900, 300, 200};
	int ytable[12] = {400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400};
#endif

	if (flagtimer == 0)
	{
		pthread_t thread1;
		pthread_create(&thread1, NULL, TIMER_1s, (void *)"Timer"); // 建立子執行緒
		pthread_detach(thread1);								   /*test*/
		flagtimer++;
	}

	if (m_timer_count > 11)
		m_timer_count = 0;
	touchdata.x = xtable[m_timer_count];
	touchdata.y = ytable[m_timer_count];
#endif

	if (touchdata.x < 0 || touchdata.y < 0)
	{
		init_parameterPanel();
		return 0;
	}

	int axisX = touchdata.x;
	int axisY = touchdata.y;

	axisX = axisX - touchdata.car_centerX;
	axisY = touchdata.car_centerY - axisY;
	touchdata.offsetX = axisX;
	touchdata.offsetY = axisY;

#if 0
	printf("*********************\n");
	Print(carSizeW);
	Print(carSizeH);
	int temp = AVM_2D_W;
	Print(temp);
	Print(AVM_2D_H);
	printf("*********************\n");
#endif
	return 1;
}

/**
 * @author : Alfred
 * @date : 2023/02/03
 * @brief Check if the touch input angle is out of the predefined limit
 * @param theta The angle theta of the touch input
 * @param phi The angle phi of the touch input
 * @param cam_ch The camera channel index
 * @param touchdata Pointer to the TouchData struct
 * @return An integer value indicating the movement direction and limit status
 *         -1: The angle is within the limit, no movement
 *          0: The angle exceeds the theta limit, move theta only
 *          1: The angle exceeds the phi limit, move phi only
 *          2: The angle exceeds both theta and phi limits, move both
 */
int isAxisOutofCar()
{
	int axisX = abs(touchdata.offsetX);
	int axisY = abs(touchdata.offsetY);

	autosys_page *avm       = &autosys.avm_page;
	ePage      *ePage       = &avm->page[avm->current_page];
	FREETOUCH *freetouch    = &ePage->set.touch.free;

// printf("axisX = %d\n",axisX);
// printf("axisY = %d\n",axisY);
// printf("touchdata.car_minW = %d\n",touchdata.car_minW);
// printf("touchdata.car_minH = %d\n",touchdata.car_minH);
#if Capacitancepanel
	float avm_2d_w = (float)MODE_VIEW[cam_ch].viewport[0].Width;
	int avm_2Dpanel_x_limit;
	if (avm_2d_w == HDwidth)
	{
		avm_2Dpanel_x_limit = 0;
	}
	else
	{
		avm_2Dpanel_x_limit = (float)avm_2d_w * touchdata.fbo2panelratio_x;
	}
#endif

#if ResistorPanel
	int avm_2Dpanel_x_limit = (float)0.f;
#endif
	float Hscle = touchdata.fbo2panelratio_y;
	float Wscle = touchdata.fbo2panelratio_x;

	int w_2dlmit = (int)((float)freetouch->In2DRectangle[2] * Wscle * 0.5f);
	int h_2dlmit = (int)((float)freetouch->In2DRectangle[3] * Hscle * 0.5f);

	int Out_w_2dlmit = (int)freetouch->Out3DRectangle[0];
	avm_2Dpanel_x_limit = Out_w_2dlmit;

	// printf("avm_2Dpanel_x_limit = %d\n",avm_2Dpanel_x_limit);
	// printf("touchdata.x = %d\n",touchdata.x);

	// int _2DAVM_ENABLE_FLAG = (MODE_VIEW[cam_ch].touch.In2DRectangle[0] > 0);
	// _2DAVM_ENABLE_FLAG |= (MODE_VIEW[cam_ch].touch.In2DRectangle[1] > 0);
	// _2DAVM_ENABLE_FLAG |= (MODE_VIEW[cam_ch].touch.In2DRectangle[2] > 0);
	// _2DAVM_ENABLE_FLAG |= (MODE_VIEW[cam_ch].touch.In2DRectangle[3] > 0);

	int x_3dlower = (int)((float)freetouch->Out3DRectangle[0] * Wscle);

#if 1
		axisX = abs(touchdata.x);
		axisY = abs(touchdata.y);
		zoom_birdView *zoom        = &autosys.zoom;
		autosys_current_info *info = &autosys.current_info;
		int left = freetouch->Out3DRectangle[0];
		int top = freetouch->Out3DRectangle[1];
		int right = left + freetouch->Out3DRectangle[2];
		int bottom = top + freetouch->Out3DRectangle[3];

		int left_2d   = freetouch->In2DRectangle[0];
		int top_2d    = freetouch->In2DRectangle[1];
		int right_2d  = left_2d + freetouch->In2DRectangle[2];
		int bottom_2d = top_2d + freetouch->In2DRectangle[3];

		if (touchdata.x >= left && touchdata.x <= right &&
    		touchdata.y >= top && touchdata.y <= bottom) 
		{
			return 2;
		}
		if (touchdata.x >= left_2d && touchdata.x <= right_2d &&
    		touchdata.y >= top_2d && touchdata.y <= bottom_2d) 
		{
			return 0;
			// int w_2dOutlmit = (int)((float)freetouch->Out2DRectangle[2] * Wscle * 0.5f);
			// int h_2dOutlmit = (int)((float)freetouch->Out2DRectangle[3] * Hscle * 0.5f);
			// return (axisX < w_2dOutlmit && axisY < h_2dOutlmit) ? 1 : -1;
		}
		else
		{
			return 1;
		}
		// int axisX = touch_info->first_touch_point.X - info->TOP2D_RANGE.X;
		// int axisY = touch_info->first_touch_point.Y - info->TOP2D_RANGE.Y;
#endif

#if 0
	if (touchdata.x >= avm_2Dpanel_x_limit)
	{
		int x_3dlower   = (int)((float)freetouch->Out3DRectangle[1] * Wscle);
		int y_3dlower   = (int)((float)freetouch->Out3DRectangle[0] * Hscle);
		int w_3dOutlmit = (int)((float)freetouch->Out3DRectangle[2] * Wscle);
		int h_3dOutlmit = (int)((float)freetouch->Out3DRectangle[3] * Hscle);
		int x_3dupper = x_3dlower + w_3dOutlmit;
		int y_3dupper = y_3dlower + h_3dOutlmit;
		// printf("touchdata.x = %d\n",touchdata.x);
		// printf("touchdata.y= %d\n",touchdata.y);

		// printf("x_3dlower = %d\n",x_3dlower);
		// printf("x_3dupper = %d\n",x_3dupper);
		// printf("y_3dlower = %d\n",y_3dlower);
		// printf("y_3dupper = %d\n",y_3dupper);
		if (touchdata.x < x_3dlower || touchdata.x > x_3dupper)
			return -1;
		if (touchdata.y < y_3dlower || touchdata.y > y_3dupper)
			return -1;
		return 2;
	}
	else if (axisX > w_2dlmit || axisY > h_2dlmit)
	{
		int w_2dOutlmit = (int)((float)freetouch->Out2DRectangle[2] * Wscle * 0.5f);
		int h_2dOutlmit = (int)((float)freetouch->Out2DRectangle[3] * Hscle * 0.5f);
		return (axisX < w_2dOutlmit && axisY < h_2dOutlmit) ? 1 : -1;
	}
	else
	{
		return 0;
	}
#endif
}
/**
 * @author : Alfred
 * @date : 2023/02/03
 */
void argminmax(float *val, float min, float max, int extraflag, float *array, float *data)
{
	float value = *val;
	if (value >= max)
		*val = max;
	if (value <= min)
		*val = min;

	for (uint8_t i = 0; i < extraflag; i++)
		if (value == array[i])
			*val = data[i];
}

/**
 * @author : Alfred
 * @date : 2023/02/03
 */
float CalAxisToPolar(int mode)
{
	float offsetAngle = 0;
	float temp = 0;
	int DeltaX, DeltaY;
	float flag[] = {0.0f};
	float fillingdata[] = {1.0f};
    autosys_page *avm       = &autosys.avm_page;
	ePage      *ePage       = &avm->page[avm->current_page];
	FREETOUCH *freetouch    = &ePage->set.touch.free;

	if (mode == CAL_THETA)
	{
		if (touchdata.offsetY != 0 || touchdata.offsetX != 0)
		{
			offsetAngle = (float)atan2((double)touchdata.offsetY, (double)touchdata.offsetX) * 180 / 3.1415926535;
			offsetAngle = 180 - offsetAngle;
			// offsetAngle *= -1;
		}
		else
		{
			return -500;
		}
	}
	else if (mode == CAL_PHI)
	{
		temp = 1 / (float)touchdata.car_minH;
		offsetAngle = touchdata.offsetY * 180.0f * temp;
		argminmax(&offsetAngle, -90.0f, 90.0f, 1, flag, fillingdata);
	}
	else if (mode == CAL_THETA_PHI)
	{
		DeltaX = touchdata.x - touchdata.tempCenterX;
		DeltaY = touchdata.y - touchdata.tempCenterY;

		touchdata.avm_3D_temptheta = touchdata.avm_3D_theta;
		touchdata.avm_3D_tempphi = touchdata.avm_3D_phi;

		if (abs(DeltaX) > touchdata.minCahngeDelta)
		{
            //int Out_w_2dlmit = (int)freetouch->Out3DRectangle[0];
			temp = 1 / (float)freetouch->Out3DRectangle[2];
			offsetAngle = (float)DeltaX * 180.0f * temp;
			touchdata.avm_3D_temptheta = touchdata.avm_3D_theta + offsetAngle;
			touchdata.avm_3D_temptheta = (touchdata.avm_3D_temptheta > 180.0f) ? (touchdata.avm_3D_temptheta - 360.0f) : touchdata.avm_3D_temptheta;
			touchdata.avm_3D_temptheta = (touchdata.avm_3D_temptheta < -180.0f) ? (360.0f + touchdata.avm_3D_temptheta) : touchdata.avm_3D_temptheta;
		}

		if (abs(DeltaY) > touchdata.minCahngeDelta)
		{
			temp = 1 / (float)freetouch->Out3DRectangle[3];
			temp *= z_axis_ViewSensitive;
			offsetAngle = (float)DeltaY * 90.0f * temp;
			touchdata.avm_3D_tempphi = touchdata.avm_3D_phi + offsetAngle;
			argminmax(&touchdata.avm_3D_tempphi, -90.0f, 90.0f, 1, flag, fillingdata);
		}
	}
	return offsetAngle;
}
/**
 * @author : Alfred
 * @date : 2023/02/03
 */
void init_touchPanel()
{
	float temp;
	if (touchdata.init_para == 0)
	{

		// touchdata.fbo2panelratio_x = 0.8f; //(1280/1024)
		// touchdata.fbo2panelratio_y = 0.83333f; //(720/600)

		touchdata.fbo2panelratio_x = 1.0f; //(1280/1024)
		touchdata.fbo2panelratio_y = 1.0f; //(720/600)

		// temp = touchdata.avmTorealAxis_callibrationValue_W * (float)AVM_2D_W;
		// touchdata.avm_2Dpanel_xlimit = (int)temp;
		touchdata.Key = 0;
		touchdata.minCahngeDelta = 0;
		touchdata.init_para++;
	}
}

/**
 * @author : Alfred
 * @date : 2023/02/03
 */
void PANEL_STATUS_RESET(int *key)
{
	*key = -1;
}
/**
 * @author : Alfred
 * @date : 2023/03/20
 */

int CHECK_TOUCH_LEGAL(int mode, float value)
{
	autosys_page *avm       = &autosys.avm_page;
	ePage      *ePage       = &avm->page[avm->current_page];
	FREETOUCH *freetouch    = &ePage->set.touch.free;
	if (mode == CAL_THETA)
	{
		if (freetouch->enable_theta_flag == 0)
		{
			return 0;
		}
		float stp = (float)freetouch->limit.theta[0];
		float edp = (float)freetouch->limit.theta[1];
		float lower_bound = stp < 0.0f ? stp + 360.0f : stp;
		float upper_bound = edp < 0.0f ? edp + 360.0f : edp;

		if (value <= -500)
		{
			return 0;
		};

		if (lower_bound > upper_bound)
		{
			if (!(value >= lower_bound || value <= upper_bound))
			{
				return 0;
			}
		}
		else if (lower_bound == upper_bound)
		{
		}
		else
		{
			if (!(value >= lower_bound && value <= upper_bound))
			{
				return 0;
			}
		}
	}
	else if (mode == CAL_PHI)
	{
		if (freetouch->enable_phi_flag == 0)
		{
			return 0;
		}
		float phi_start = (float)freetouch->limit.phi[0];
		float phi_end = (float)freetouch->limit.phi[1];
		if (value < phi_start || value > phi_end)
		{
			return 0;
		}
	}
	return 1;
}
void assign_move(avm_ui_info_t *uiInfo, camera_para_t *camerapara, int mode, float theta, float phi)
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
/**
 * @brief Calculate the center point of the current block on the 2D bird view image.
 * @param realP The real point on the 2D bird view image.
 * @return The center point of the current block.
 * @note This function is used to calculate the center point of the current block on the 2D bird view image.
 *       The center point is calculated based on the block size and the real point on the image.
 */
float mem_box_center_x = 0.0f;
float mem_box_center_y = 0.0f;
PointF calcu_box_center(PointF realP)
{
	zoom_birdView *zoom        = &autosys.zoom;
	autosys_current_info *info = &autosys.current_info;
	if(zoom->cur_level != 0)
	{
		return (PointF){mem_box_center_x, mem_box_center_y};
	}
	PointF box_center;
	float box_center_x = 0.0f;
	float box_center_y = 0.0f;
	int cut_x = zoom->block_size_col;
	int cut_y = zoom->block_size_row;
	PointF Img_area = {(float)info->TOP2D_RANGE.X, (float)info->TOP2D_RANGE.Y};
	float x_block_size = (Img_area.X / (float)(cut_x));
	float y_block_size = (Img_area.Y / (float)(cut_y));
	int x_block = (int)(realP.X / x_block_size);
	int y_block = (int)(realP.Y / y_block_size);

	float x_absolute_psn, y_absolute_psn;
	x_absolute_psn = x_block * x_block_size;
	y_absolute_psn = y_block * y_block_size;
	box_center.X =  box_center_x = (x_absolute_psn) + (x_block_size / 2.0f);
	box_center.Y =  box_center_y = (y_absolute_psn) + (y_block_size / 2.0f);
	mem_box_center_x = box_center_x;
	mem_box_center_y = box_center_y;
	return box_center;
}
/*Click Event*/

TOUCH_HANDLE_CASE touchClick_handleEvent(int isCuiTouch, float processtime, float responseTime, int *counter, int counter_limit)
{
    autosys_touch *touch_info = &autosys.touch;
    int rec;
    //AVM_LOGI("Touch[%d], status[%d], panel[%d]\n", touch_info->first_touch, touch_info->preious_status, isCuiTouch);
    switch(isCuiTouch)
    {
        case 0://leave
        switch(touch_info->first_touch + (touch_info->preious_status))
        {
            case (1 + TOUCH_PRESS):
            touch_info->preious_status = TOUCH_LEAVE;
            touch_info->current_status = TOUCH_LEAVE;
            if(processtime > responseTime)
            {
                touch_info->preious_status = TOUCH_EMPTY;
                touch_info->current_status = TOUCH_EMPTY;
                touch_info->flag_lock_2dAVM = 1;
                touch_info->first_touch = 0;
                //AVM_LOGI("unLock 2dAVM limit\n");
            }
            break;
            case (2 + TOUCH_PRESS):
            //AVM_LOGI("Emit the click Event\n");
            touch_info->first_touch = 0;
            touch_info->preious_status = TOUCH_EMPTY;
            touch_info->current_status = TOUCH_EMPTY;
            if(processtime > responseTime)
            {
                touch_info->flag_lock_2dAVM = 1;
                //AVM_LOGI("AAAunLock 2dAVM limit\n");
            }
            else
            {
                touch_info->flag_lock_2dAVM = 0;
                return TOUCH_CLICK;
                //AVM_LOGI("Successfully to Zoomin top 2D\n");
            }
            break;
            case (1 + TOUCH_LEAVE):
            case (2 + TOUCH_LEAVE):
            if(processtime > responseTime)
            {
                touch_info->preious_status = TOUCH_EMPTY;
                touch_info->current_status = TOUCH_EMPTY;
                touch_info->flag_lock_2dAVM = 1;
                touch_info->first_touch = 0;
                //AVM_LOGI("unLock 2dAVM limit\n");
            }
            break;
        }
        break;

        case 1://touch
        if(touch_info->first_touch == 0 && touch_info->current_status == TOUCH_EMPTY)
        {
            *counter = 0;
            touch_info->preious_status = TOUCH_PRESS;
            touch_info->current_status = TOUCH_PRESS;  
            gettimeofday(&start, NULL);
            rec = isAxisOutofCar();
            if(rec <= 1)
            {
                touch_info->first_touch     = 1;
                touch_info->flag_lock_2dAVM = 0;
                Point touchPoint = (Point){.X = abs(touchdata.x),.Y = abs(touchdata.y)};
			    touch_info->first_touch_point = touchPoint;
                //AVM_LOGI("\033[31m [0] \033[0m Lock 2dAVM limit\n");
            }
            else
            {
                touch_info->preious_status = TOUCH_EMPTY;
                touch_info->current_status = TOUCH_EMPTY; 
                touch_info->flag_lock_2dAVM = 1;
            } 
            return TOUCH_PRESS;
        }
        else if(touch_info->first_touch == 1 && touch_info->preious_status == TOUCH_PRESS)
        {
            *counter = *counter + 1;
            touch_info->current_status = TOUCH_MOVE;
            if(processtime > (responseTime/2.0f) || (*counter > counter_limit) )
            {
                touch_info->flag_lock_2dAVM = 1;
                //AVM_LOGI("\033[31m [1] \033[0m unLock 2dAVM limit [%f,%d]\n", processtime, *counter);
            }
            return TOUCH_MOVE;
        }
        else if(touch_info->first_touch == 1 && touch_info->preious_status == TOUCH_LEAVE)
        {
            *counter = 0;
            touch_info->first_touch = 2;
            touch_info->preious_status = TOUCH_PRESS;
            touch_info->current_status = TOUCH_PRESS;
            if(processtime > responseTime)
            {
                touch_info->flag_lock_2dAVM = 1;
                //AVM_LOGI("\033[31m [2] \033[0m unLock 2dAVM limit\n");
            }
            return TOUCH_PRESS;
        }
        else if(touch_info->first_touch == 2 && touch_info->preious_status == TOUCH_PRESS)
        {
            *counter = *counter + 1;
            touch_info->current_status = TOUCH_MOVE;
            if(processtime > (responseTime) || (*counter > counter_limit))
            {
                touch_info->flag_lock_2dAVM = 1;
                //AVM_LOGI("\033[31m [3] \033[0m unLock 2dAVM limit [%f,%d]\n", processtime, *counter);
            }
            return TOUCH_MOVE;
        }
        break;
    }
    *counter = 0;
    return TOUCH_EMPTY;
}
/**
 * @brief :  rotate 3D avm view
 * @param :  rec is mean Mode, Mode(0) rotate theta(0-360); Mode(1) rotate phi (-56 - 56);
 * @note : the flow is following step:
 * 1. if Ontouch 2.calculator axis(touch) 3. call AxisOffsetTo2DAVM() to offset touch(axis) to carCentor(axis)
 * 4. call isAxisOutofCar() to determind mode 5. calculator angle, accroding to offset axis
 * @author : Alfred
 * @date : 2023/02/03
 */
void avm_panelTouch_moveView(avm_ui_info_t avm_info, stTouchData* data)
{
    //memcpy(&touchdata, data, sizeof(stTouchData));
	autosys_page *avm       = &autosys.avm_page;
	ePage      *ePage       = &avm->page[avm->current_page];
	FREETOUCH *freetouch    = &ePage->set.touch.free;
    zoom_birdView *zoom     = &autosys.zoom;
    
	int rec = -1, mode = -1;
	float theta = 0, phi = 0;
    
	init_touchPanel();
	float Hscle = touchdata.fbo2panelratio_y;
	float Wscle = touchdata.fbo2panelratio_x;

	touchdata.car_minH = (int)((float)freetouch->In2DRectangle[3] * Hscle);
	touchdata.car_minW = (int)((float)freetouch->In2DRectangle[2] * Wscle);
	touchdata.car_centerX = (int)((float)freetouch->In2DRectangle[0] * Hscle + (float)touchdata.car_minW * 0.5f);
	touchdata.car_centerY = (int)((float)freetouch->In2DRectangle[1] * Wscle + (float)touchdata.car_minH * 0.5f);
	if(autosys.g_flag.fuc_touch != 1) return;
    autosys_touch *touch_info = &autosys.touch;
	{
		#if PARAM_ZOOM_ENABLE
        gettimeofday(&end, NULL);
		bool flag_fakeTouch_mode = false;
		__AVM_CALL_STATUS_IVI__(5, zoom->cur_level);
        int during_count   = autosys.system_info.click_during_Count;
        int isCuiTouch     = AxisOffsetTo2DAVM();
        float process_time = autosys.system_info.click_process_Time;
        float dtime        = calculate_time_secondunit(start, end);
        TOUCH_HANDLE_CASE touchCase = touchClick_handleEvent(isCuiTouch, dtime, process_time, &touch_count, during_count);
        if(touchCase == TOUCH_CLICK || flag_fakeTouch_mode)
        {
			if(flag_fakeTouch_mode == true)
			{
                autosys_current_info *info = &autosys.current_info;
                int axisX = 300;
                int axisY = 900;
                if(axisX < 0 || axisY < 0) return;
                if (axisX < info->TOP2D_RANGE.X && axisY < info->TOP2D_RANGE.Y)
                {
					PointF center;
					center.X = (float)info->TOP2D_RANGE.X / 2.0f;
					center.Y = (float)info->TOP2D_RANGE.Y / 2.0f;
					
					float x_block_size = (float)info->TOP2D_RANGE.X / (float)(zoom->block_size_col);
                    float y_block_size = (float)info->TOP2D_RANGE.Y / (float)(zoom->block_size_row);
                    int x_block = axisX / x_block_size;
                    int y_block = axisY / y_block_size;
			
					if(zoom->cur_level == 0 && zoom->enableblock[y_block][x_block] == 0)
					{
						AVM_LOGI("[HINT] TOUCH ENABLE NON ENABLE\n");
						return;
					}
					

					int block_num = x_block + y_block * (zoom->block_size_col);
                    AVM_LOGI("level [%d], block is %d\n", zoom->cur_level, block_num);
                    if(zoom->cur_level == 0){zoom->firs_select_block = block_num;}
					PointF realSrcP = autosys_get_realP_fromTouchPoint((float)axisX, (float)axisY);
					PointF boxcenter = calcu_box_center(realSrcP);
                    zoom->cur_level = 1;
					float nm_diff_x = (boxcenter.X - center.X) / (float)info->TOP2D_RANGE.X;
					float nm_diff_y = (boxcenter.Y - center.Y) / (float)info->TOP2D_RANGE.Y;
					nm_diff_x *= 2.0f;
					nm_diff_y *= 2.0f;
					autosys_set_zoomin2DprojectionmatrixByVector(-nm_diff_x , nm_diff_y ,zoom->Each_level_num[zoom->cur_level]);
                    zoom->update_status = (zoom->update_status == 0)?1:zoom->update_status;
                }
				return;
			}
            AVM_LOGI("Successfully to Zoomin top 2D\n");
            if (autosys.g_flag.fuc_zoom_birdView == 1)// && !autosys.seethrough.seethrough_switch)
            {
                autosys_current_info *info = &autosys.current_info;
                int axisX = touch_info->first_touch_point.X - info->TOP2D_LayoutPoint.X;
                int axisY = touch_info->first_touch_point.Y - info->TOP2D_LayoutPoint.Y;
                if(axisX < 0 || axisY < 0) return;
                if (axisX < info->TOP2D_RANGE.X && axisY < info->TOP2D_RANGE.Y)
                {
					PointF center;
					center.X = (float)info->TOP2D_RANGE.X / 2.0f;
					center.Y = (float)info->TOP2D_RANGE.Y / 2.0f;

					#if 0 // oldversion
                    float x_block_size = (float)info->TOP2D_RANGE.X / (float)(zoom->linetable[zoom->cur_level][0] - 1);
                    float y_block_size = (float)info->TOP2D_RANGE.Y / (float)(zoom->linetable[zoom->cur_level][1] - 1);
                    int x_block = axisX / x_block_size;
                    int y_block = axisY / y_block_size;
                    int block_num = x_block + y_block * (zoom->linetable[zoom->cur_level][0] - 1);
                    AVM_LOGI("level [%d], block is %d\n", zoom->cur_level, block_num);
                    if(zoom->cur_level == 0){zoom->firs_select_block = block_num;}
                    zoom->cur_level = (zoom->cur_level + 1) % PARAM_ZOOM_BIRDVIEW_MAX;
                    autosys_set_zoomin2Dprojectionmatrix(zoom->Each_level_num[zoom->cur_level], 0);
                    zoom->update_status = (zoom->update_status == 0)?1:zoom->update_status;
					#endif
					
					float x_block_size = (float)info->TOP2D_RANGE.X / (float)(zoom->block_size_col);
                    float y_block_size = (float)info->TOP2D_RANGE.Y / (float)(zoom->block_size_row);
                    int x_block = axisX / x_block_size;
                    int y_block = axisY / y_block_size;
			
					if(zoom->cur_level == 0 && zoom->enableblock[y_block][x_block] == 0)
					{
						AVM_LOGI("[HINT] TOUCH ENABLE NON ENABLE\n");
						return;
					}
					

					int block_num = x_block + y_block * (zoom->block_size_col);
                    AVM_LOGI("level [%d], block is %d\n", zoom->cur_level, block_num);
                    if(zoom->cur_level == 0){zoom->firs_select_block = block_num;}
					PointF realSrcP = autosys_get_realP_fromTouchPoint((float)axisX, (float)axisY);
					PointF boxcenter = calcu_box_center(realSrcP);
					if( zoom->enable_layer_num != 0)
					{
                    	zoom->cur_level = (zoom->cur_level + 1) % zoom->enable_layer_num;
					}
					else
					{
						zoom->cur_level = (zoom->cur_level + 1) % 1;
					}

					//info->TOP2D_RANGE.X
					//diff
					#if PRRAM_ZOOM_DIFF_BY_TOUCH
					float nm_diff_x = (realSrcP.X - center.X) / (float)info->TOP2D_RANGE.X;
					float nm_diff_y = (realSrcP.Y - center.Y) / (float)info->TOP2D_RANGE.Y;
					nm_diff_x *= 2.0f;
					nm_diff_y *= 2.0f;
					#else
					float nm_diff_x = (boxcenter.X - center.X) / (float)info->TOP2D_RANGE.X;
					float nm_diff_y = (boxcenter.Y - center.Y) / (float)info->TOP2D_RANGE.Y;
					nm_diff_x *= 2.0f;
					nm_diff_y *= 2.0f;
					#endif
					autosys_set_zoomin2DprojectionmatrixByVector(-nm_diff_x , nm_diff_y ,zoom->Each_level_num[zoom->cur_level]);
                    zoom->update_status = (zoom->update_status == 0)?1:zoom->update_status;
                }
					
            }
            else
            {
                autosys_set_zoomin2DprojectionmartixReset();
            }
			__AVM_CALL_STATUS_IVI__(5, zoom->cur_level);
        }
		#endif
	}

	if (avm_info.view == VIEW_AVM_LAYOUT)
	{
        camera_para_t* projection = autosys_get_AVM3DProjectionmatrix();
		if (AxisOffsetTo2DAVM() == 0)// || zoom->cur_level != 0)
		{
			return;
		} // hand-touch has been leave touch-panel
		if (!freetouch->enable)
		{
			return;
		}
		rec = isAxisOutofCar();
		int step = 0;
		if (rec == 0 || rec == 1)
		{
			// printf("rec = %d\n",rec);
            if(touch_info->flag_lock_2dAVM != 1) return;

			if (rec == 1) // Move theta angle
			{
				theta = CalAxisToPolar(CAL_THETA);
				if (!CHECK_TOUCH_LEGAL(CAL_THETA, theta))
				{
					PANEL_STATUS_RESET(&rec);
				};
			}
			else
			{
				phi = CalAxisToPolar(CAL_PHI);
				if (!CHECK_TOUCH_LEGAL(CAL_PHI, phi))
				{
					PANEL_STATUS_RESET(&rec);
				};
			}
			// printf("phi = %f\n",phi);
			mode = rec;
			assign_move(&avm_info, projection, mode, theta, phi);
			step = -1;
		}
		else 
		{
			//printf("MODE_VIEW[cam_ch].touch.enable = %d\n", MODE_VIEW[cam_ch].touch.enable);
			//printf("rec = %d\n", rec);
			//printf("cam_ch = %d\n", cam_ch);
			//printf("touchdata.isFirstcount= %d\n", touchdata.isFirstcount);
			if (touchdata.isFirstcount)
			{
				touchdata.avm_3D_theta = projection->exPara.theta;
				touchdata.avm_3D_phi  = projection->exPara.phi;
				touchdata.tempCenterX = touchdata.x;
				touchdata.tempCenterY = touchdata.y;
				PANEL_STATUS_RESET(&rec);
				touchdata.isFirstcount = 0;
			}
			else
			{
				CalAxisToPolar(CAL_THETA_PHI);
				theta = touchdata.avm_3D_temptheta;
				phi = touchdata.avm_3D_tempphi;
				// printf("theta = %f\n",theta);
				// printf("phi = %f\n",phi);
				float theta_start = (float)freetouch->limit.theta[0];
				float theta_end = (float)freetouch->limit.theta[1];
				float phi_start = (float)freetouch->limit.phi[0];
				float phi_end = (float)freetouch->limit.phi[1];

				int out_of_phi_limit = 0;
				int out_of_theta_limit = 0;
				if (phi < phi_start || phi > phi_end)
				{
					touchdata.isFirstcount = 1;
					out_of_phi_limit = 1;
				}

				if (theta < theta_start || theta > theta_end)
				{
					touchdata.isFirstcount = 1;
					out_of_theta_limit = 1;
				}

				if (out_of_theta_limit && out_of_phi_limit)
				{
					PANEL_STATUS_RESET(&rec);
				}
				else if (out_of_theta_limit)
				{
					rec = 0;
				}
				else if (out_of_phi_limit)
				{
					rec = 1;
				}
			}
			// touchdata.isFirstcount = 0;
			mode = rec;
			assign_move(&avm_info, projection, mode, theta, phi);
			step = 1;
		}
	}
}
