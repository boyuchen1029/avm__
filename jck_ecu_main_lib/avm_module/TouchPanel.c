/**
 *******************************************************************************
 * @file : TouchPanel.c
 * @describe : .
 *
 * @author : Alfred
 * @verstion : 0.1.0.
 * @date 20230203 0.1.0 Alfred.
 *******************************************************************************
*/

#include "../../system.h"
#include "user_control/ui.h"
#include "avm/camera_matrix.h"

#include "display_location.h"
#include "avm_2d_3d_para.h"

#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>
#include <pthread.h>

#include <avmDefine.h>

#include <unistd.h>
#include <string.h>


#define INPUT_DEV_PATH "/dev/input/event2"
struct input_event Panel_key_info;
pthread_t thread_panel;

int x =-1, y = -1;
int tempX = 0, tempY = 0;


char statusOn[] = "OnPress" ;
char statusOff[] = "OffPress" ;
char *panel_status;

int fd = -1;

/**
*
* @brief : get panel event
* @param : no used
* @note : keycode = 53 is mean axis-X, 54 is axix-Y;
*  and keycode = 48 is mean touch mode
*  So, if keycode(48) value > 0, meaning is something touch the panel
* @property :Capacitancepanel, ResistorPanel
* Capacitancepanel x and y value is as axis-x and axis-y in touch panel
* ResistorPanel x and y value is 0-4095 range, so it must be normalized to touch panel(current) 
*
*/
void *Panel_Event(void *arg)
{

while(1)
{
	usleep(100);

	memset(&Panel_key_info, 0, sizeof(struct input_event));

	if (read(fd, &Panel_key_info, sizeof(struct input_event)) > 0)
	{
		#if Capacitancepanel
		if (Panel_key_info.code == 53) tempX =  (Panel_key_info.value);      
		if (Panel_key_info.code == 54) tempY =  (Panel_key_info.value) ;
		#endif
		// printf("Here is avm ! Panel_key_info.code = %d\n",Panel_key_info.code);
		// printf("Here is avm ! Panel_key_info.value = %d\n",Panel_key_info.value);
		#if ResistorPanel
		if (Panel_key_info.code == 53) tempX =  Panel_key_info.value  * 0.3125 ;         //  x-value min is 160, max is 3900, 0.000267 = 1/ (3900-160)
		if (Panel_key_info.code == 54) tempY =  Panel_key_info.value * 0.176366843 ;      //  y-value min is 400m nax is 3650, 0.0003076 = 1/ (3650-400)
		#endif

		if (Panel_key_info.code == 54) 
		{
			if(Panel_key_info.value > 0) /*has been touch*/
			{
				x = tempX;
				y = tempY;
				panel_status = statusOn;
			}
			else
			{
				x = -1; y = -1;
				panel_status = statusOff;
			}

		}
		else if (Panel_key_info.code == 57 && Panel_key_info.value == -1)
		{
			x = -1;
			y = -1;
		}
		// printf("Panel Status : %s ;;; current X = %d, current Y = %d ;;; last X = %d, last Y = %d\n",panel_status,x,y,tempX,tempY);
		
	}
	else
	{
		x = -1;
		y = -1;
		panel_status = statusOff;
		// printf("****no event****\n");
	}

}
	close(fd);
}


void Init_Panel_monitor()
{
	/*
	INPUT_DEV_PATH = dev/input/event2
	*/
	// printf("***init panel x = %d, y = %d ;***\n", x,y);
	fd = open(INPUT_DEV_PATH , O_RDONLY );
    if(fd < 0) {
        printf("open failed, error:%d\n", errno);
        return -1;
    }
	pthread_create(&thread_panel, NULL, Panel_Event, (void *)"Panel_Event"); // 建立子執行緒
	pthread_detach(thread_panel); 
}

void Get_Panel_Axis(stTouchData *data)
{
	/*get current x and y axis, if situation(current) is non-touch panel, return -1,-1*/
	data->x = x;
	data->y = y;
	//AVM_LOGI("touch x = %d, y = %d\n",data->x,data->y);
	// return &data;
} 