#define board6702 0
#define board8368 1

#define Board board6702

#define USB_LOGGIN
#define CLOCK_LOGGIN
#define AUTOSYS_CAN_SERVICE
#include <iostream>
#include <fstream>
#include <string>
#include "wayland_display.h"
#include <ctime>
#include <vector>
#include <sstream>
#include <cstring>
#ifdef AUTOSYS_CAN_SERVICE
#include "UART_Reader.h"
#include "SerialPort.h"
#define DOOR_FRONT_RIGHT_MASK  0x10
#define DOOR_FRONT_LEFT_MASK   0x20
#define DOOR_REAR_LEFT_MASK    0x04
#define DOOR_REAR_RIGHT_MASK   0x08
#define DOOR_TAILGATE_MASK     0x02
#endif
 int Cam_PixelFMT;
 int VIEW;
  int imageWidth;
 int imageHeight;
int cam_ch_change_count = 0;
#define FBDbg	  printf("%s(%d)\n", __func__, __LINE__)
#define DBG_count 60 * 30 // print per minute

#define CLEAR(x) memset(&(x), 0, sizeof(x))

/*==============IVI interface=====================*/
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

#define IVI_set_FASTCAL_STATUS_info 0x3C
#define IVI_get_FASTCAL_STATUS_info 0x3D

#define IVI_set_CONTRL_CarModel_info 0x41
#define IVI_get_CONTRL_CarModel_info 0x42

#define IVI_set_CONTRL_touch_info 0x43
#define IVI_get_CONTRL_touch_info 0x44

#define AVM_IVI_INTERFACE 1
struct can_frame
{
    int can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    int data[11];
};
struct can_frame AVM_view_page = {0};
struct can_frame AVM_view_page1 = {0};
struct can_frame AVM_view_page2 = {0};
struct can_frame _AVM_get_DVR_view_page;
struct can_frame _AVM_get_SYSTEM_info;
struct can_frame _AVM_get_SYSTEM_info_compar;
struct can_frame _AVM_get_MOD_info;
struct can_frame _AVM_set_MOD_info;
struct can_frame _AVM_get_IMS_info;
struct can_frame _AVM_set_IMS_info;
struct can_frame _AVM_get_BSD_info;
struct can_frame _AVM_set_BSD_info;
struct can_frame _AVM_get_ODA_info;
struct can_frame _AVM_set_ODA_info;
struct can_frame _AVM_get_LDWS_info;
struct can_frame _AVM_set_LDWS_info;
struct can_frame _AVM_get_turn_signal_info;
struct can_frame _AVM_get_PGL_style_info;
struct can_frame _AVM_set_CONTRL_info;
struct can_frame _AVM_get_CONTRL_info;
struct can_frame _IVI_set_CONTRL_info;
struct can_frame _IVI_get_360_status;
struct can_frame _IVI_set_SYSTEM_info;
struct can_frame _IVI_set_STATUS_info;
struct can_frame _IVI_get_STATUS_info;
struct can_frame _IVI_set_CONTRL_CarModel_info;
struct can_frame _IVI_get_CONTRL_CarModel_info;
struct can_frame _IVI_set_CONTRL_touch_info;
struct can_frame _IVI_get_CONTRL_touch_info;
struct can_frame _IVI_set_FASTCAL_STATUS_info;
struct can_frame _IVI_get_FASTCAL_STATUS_info;
// struct can_frame _AVM_get_CONTRL_info;
//using namespace spplfctrl;
int fd_autosys;
int amv_layer_enable_flag = 0;
static sig_atomic_t paused = 0;
int SetLayerDisable = 0;
int SetLayerEnable = 0;

bool bWhiteBalance=false;

/*==============IVI interface=====================*/

using namespace adas_cam;
using namespace std;
SP_adas_cam *myInstance = NULL;

struct ivi_hmi_controller *hmiCtrl;

struct timeval loop_cur = {0}, loop_priv = {0};
struct timeval start, end;
struct  avmJsonData
{
    int color;
    bool mod;
    int page;
    bool turn;
    avmJsonData(){
        color = 0;
        mod = true;
        page = 1;
        turn = false;
    }
};
avmJsonData _avm_json_data;
#ifdef CLOCK_LOGGIN
struct timespec adas_cam_start, adas_cam_end;
struct timeval canbus_start, canbus_end;
#else
struct timeval adas_cam_start,adas_cam_end;
#endif
unsigned long diff = 0;

V4L2_Service_Param_t_cxx CamInfo;
V4L2_Frame_Param_t_cxx V4L2_Frame_Param;
uint32_t v4l_format;
unsigned char *V4L2Buf_VAF;
unsigned char *V4L2Buf_VAB;
unsigned char *V4L2Buf_VAL;
unsigned char *V4L2Buf_VAR;
unsigned char *V4L2Buf_4to1_Luma;
unsigned char *V4L2Buf_4to1_Chroma;

int V4L2Buf_FdF = 0;
int V4L2Buf_FdB = 0;
int V4L2Buf_FdL = 0;
int V4L2Buf_FdR = 0;
unsigned int printDBG = 0;
int getFrameCount = 0;
int TimeCount = 0;
sem_t *layer_sem;

user_command_mode_t user_command = CMD_NONE;
can_bus_info_t canBus = {0.0, 5.0, 'D', 'O', 0, 0, 0.0, 0};
struct autosys_avm_info _autosys_avm_info;
static int current_avm_status = 0;
bool Show4to1TS = false; // Set true to Add 4to1 TS
int CurrentLayerStatus = 1; // Current avm layer status. 0: disable, 1: enable
float calculate_time(struct timeval start, struct timeval end);
typedef enum{
	front_camera,
	back_camera,
	left_camera,
	right_camera,
};

typedef enum{
	normal,
	failure,
};

static const struct wl_callback_listener frame_listener = {
	redraw};

static int
v4l_connect()
{
	myInstance->V4L2_Serv_Init_API();
	myInstance->V4L2_Serv_GetServiceInfo_API(&CamInfo);

	if (V4L2_PLANE_SEL > 4) {
		V4L2_PLANE_SEL = 0;
	}

	// v4l_format = parse_format((char *)(&CamInfo.pixelformat[V4L2_PLANE_SEL]));
	imageWidth = CamInfo.V4L2_Frame_w[V4L2_PLANE_SEL];
	imageHeight = CamInfo.V4L2_Frame_h[V4L2_PLANE_SEL];

	return 1;
}

static int
queue_initial_buffers(struct display *display,
					  struct buffer buffers[NUM_BUFFERS])
{
	struct buffer *buffer;
	int index;

	for (index = 0; index < NUM_BUFFERS; ++index) {
		buffer = &buffers[index];
		buffer->index = index;
		if (!_queue(buffer)) {
			fprintf(stderr, "Failed to _queue buffer\n");
			return 0;
		}

		assert(!buffer->buffer);
	}

	return 1;
}

static void
dequeue()
{
	myInstance->V4L2_Serv_GrabV4L2Frame_API(&V4L2_Frame_Param);
}

static int
fill_buffer_format(struct display *display)
{
	display->format.num_planes = 1;
	display->format.width = CamInfo.V4L2_Frame_w[V4L2_PLANE_SEL];
	display->format.height = CamInfo.V4L2_Frame_h[V4L2_PLANE_SEL];
	display->format.strides[0] = CamInfo.V4L2Buf_stride[V4L2_PLANE_SEL];
	if (Cam_PixelFMT == 1) {
		display->format.strides[0] <<= 1;
	}
	display->format.strides[1] = display->format.strides[0];

	return 1;
}

static int
v4l_init(struct display *display, struct buffer buffers[NUM_BUFFERS])
{
	if (!fill_buffer_format(display)) {
		fprintf(stderr, "Failed to fill buffer format\n");
		return 0;
	}

	if (!queue_initial_buffers(display, buffers)) {
		fprintf(stderr, "Failed to _queue initial buffers\n");
		return 0;
	}

	return 1;
}

static void
avm_layer_enable(int enable, struct display *display)
{
	struct wl_array layers;
	int *layers_item = NULL;
	int plane_mask = 0x1; // OSD_0 plane mask

	wl_array_init(&layers);
	layers_item = (int *)wl_array_add(&layers, sizeof(int));
	if (layers_item) {
		*layers_item = 3000; // OSD_0 layer-id
	}

	if (enable == 1) {
		ivi_hmi_controller_enable_layers(hmiCtrl, &layers);
	} else {
		ivi_hmi_controller_disable_layers(hmiCtrl, &layers);
	}
	wl_display_flush(display->display);
	wl_array_release(&layers);

	inform_drm_tako_hardware_init(plane_mask, display);
}

stTouchData touchdata1;
int touch_write_flag = 0;
int GO_to_QT_flag = 0;
int Fisheye_capture = 0;
int Fisheye_file_count = 0;
std::string USB_EVSTABLE_LOCATION;
FILE * fp1;
FILE * fp2;
FILE * fp3;
FILE * fp4;
int g_avm_detect_camera[4] = {0};
bool read_ttySData_flag = false;
float g_speed = 0.0f;
int g_gear = 0;
int g_door = 0;
float g_steering_angle = 0;
double beforeSpeed = 0.0;
double meterageEach = 0.0;
void read_ttySData()
{
	#ifdef AUTOSYS_CAN_SERVICE
	int status = -1;
	int index_Page = 1;
	int index_EightDir = 0;

	read_ttySData_flag = true;

	uint32_t result = 0;
	std::string data;

	SerialPort *serial = nullptr;
	#if (VEHICLE_TYPE == __URX__)
	serial = new SerialPort("/dev/ttyUSB0", 9600);
	if (serial == nullptr || !serial->isOpen())
	{
		printf("\033[31mFailed to open serial port.\033[0m\n");
		delete serial;
		return;
	}
	printf("\033[33mSessfully to open serial port.\033[0m\n");
	UART_Reader reader;
	std::string ss;
	std::string buffer;
	//canBus
	while (serial && serial->isOpen())
	{
		ss = serial->readString();
		buffer += ss;
		if(buffer.length() > 0)
		{
			printf("ReadString: %s\n" , buffer.c_str());
		}
		while (buffer.length() >= 6)
		{												// Check for at least 4 bytes (32 bits)
			std::string sentence = buffer.substr(0, 6); // Get the first 4 bytes
			buffer.erase(0, 6);							// Remove processed data from buffer

			// Convert the binary data to a hex string
			std::stringstream hexStream;
			for (unsigned char c : sentence)
			{
				hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)c;
			}
			std::string hexString = hexStream.str();

			std::cout << "ReadString: " << hexString << std::endl;
			if (reader.read_and_decode_data(hexString, result))
			{
				reader.get_seethrough_data(g_gear, g_speed, g_steering_angle);
				AVM_LOGI("Seethrough gear{%d} angle{%f} speed{%f}\n", g_gear, g_steering_angle, g_speed);
				// std::cout << "Data received and verified successfully. Data: " << std::hex << result << std::endl;
				if(g_gear == 0 || g_gear == 1)
				{
					canBus.speed = 0;
					canBus.meterage = 0;
				}
				else
				{
					// {
					// 	gettimeofday(&canbus_end, NULL);
                    // 	float dtime = calculate_time(canbus_start, canbus_end);
                    	// canBus.speed = (double)g_speed;

                    // 	meterageEach = (beforeSpeed + (double)canBus.speed) * dtime / (2 * 3.6) * 0.95;

		            //     canBus.meterage = canBus.meterage + meterageEach;
                    // 	beforeSpeed = (double)canBus.speed;
                    // 	gettimeofday(&canbus_start, NULL);
					// }
				}
			}
			else
			{
				std::cerr << "Failed to verify data." << std::endl;
			}
			// _IVI_set_CONTRL_info.data[0] = status;
			// ioctl(fd_autosys, IVI_set_CONTRL_info, &_IVI_set_CONTRL_info);

			// _IVI_set_SYSTEM_info.data[0] = speed;
			// _IVI_set_SYSTEM_info.data[1] = gear;
			// ioctl(fd_autosys, IVI_set_SYSTEM_info, &_IVI_set_SYSTEM_info);
		}
	}
	#else
	serial = new SerialPort("/dev/ttyUSB0", 921600);
	if (serial == nullptr || !serial->isOpen())
	{
		printf("\033[31mFailed to open serial port.\033[0m\n");
		delete serial;
		return;
	}
	printf("\033[33mSessfully to open serial port.\033[0m\n");
	UART_Reader reader;
	std::string ss;
	std::string buffer;
	//canBus
	while (serial && serial->isOpen())
	{
		ss = serial->readString();
		buffer += ss;
		#define PARSE_DATA_SIZE 9
		while (buffer.length() >= PARSE_DATA_SIZE)
		{												// Check for at least 4 bytes (32 bits)
			if ((uint8_t)buffer[0] != 0x55 && (uint8_t)buffer[1] != 0xAA)
			{
				buffer.erase(0, 1);
				continue;
			}
			std::string sentence = buffer.substr(0, PARSE_DATA_SIZE); // Get the first 4 bytes
			buffer.erase(0, PARSE_DATA_SIZE);	

			// Convert the binary data to a hex string
			std::stringstream hexStream;
			for (unsigned char c : sentence)
			{
				hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)c;
			}
			std::string hexString = hexStream.str();

			//std::cout << "ReadString: " << hexString << std::endl;
			if (reader.read_and_decode_data(hexString, result))
			{
				reader.get_seethrough_data(g_gear, g_speed, g_steering_angle,g_door);
				// AVM_LOGI("Seethrough gear{%d} angle{%f} speed{%f} door{%d}\n", g_gear, g_steering_angle, g_speed,g_door);
				// std::cout << "Data received and verified successfully. Data: " << std::hex << result << std::endl;
				if(g_gear == 0 || g_gear == 1)
				{
					//canBus.speed = 0;
					//canBus.meterage = 0;
				}
				else
				{
					// {
					// 	gettimeofday(&canbus_end, NULL);
                    // 	float dtime = calculate_time(canbus_start, canbus_end);
                    	// canBus.speed = (double)g_speed;

                    // 	meterageEach = (beforeSpeed + (double)canBus.speed) * dtime / (2 * 3.6) * 0.95;

		            //     canBus.meterage = canBus.meterage + meterageEach;
                    // 	beforeSpeed = (double)canBus.speed;
                    // 	gettimeofday(&canbus_start, NULL);
					// }
				}
			}
			else
			{
				//std::cerr << "Failed to verify data." << std::endl;
			}
			// _IVI_set_CONTRL_info.data[0] = status;
			// ioctl(fd_autosys, IVI_set_CONTRL_info, &_IVI_set_CONTRL_info);

			// _IVI_set_SYSTEM_info.data[0] = speed;
			// _IVI_set_SYSTEM_info.data[1] = gear;
			// ioctl(fd_autosys, IVI_set_SYSTEM_info, &_IVI_set_SYSTEM_info);
		}
	}
	#endif
	pthread_exit(NULL);
	delete serial;
	return;
	#endif
}

void redraw(void *data, struct wl_callback *callback, uint32_t time)
{
		// 
	struct window *window = (struct window *)data;
	struct buffer *buffer;
	int index, num_busy = 0;
	float diff_time;

	if (SetLayerDisable == 1)
	{
		avm_layer_enable(disable, window->display);
		// Disable MIPI-CSI or V656
		myInstance->ADAS_Cam_Video_Input_Ctrl_API(disable);
		// Disable IW0
		myInstance->ADAS_Cam_IW_Ctrl_API(0, disable);
		SetLayerDisable = 0;
		CurrentLayerStatus = 0;
	}

	/* Pause AVM render. */
	while (paused == 1)
	{
		usleep(50000);
	}

	/* Enable iw0, mipi csi/v656, avm layer when SetLayerEnable event is
	 * triggered.
	 */
	if (SetLayerEnable == 1)
	{
		// Enable IW0
		myInstance->ADAS_Cam_IW_Ctrl_API(0, enable);
		// Enable MIPI-CSI or V656
		myInstance->ADAS_Cam_Video_Input_Ctrl_API(enable);
		avm_layer_enable(enable, window->display);
		SetLayerEnable = 0;
		CurrentLayerStatus = 1;
	}

	for (index = 0; index < NUM_BUFFERS; ++index) {
		if (window->buffers[index].busy) {
			++num_busy;
		}
	}

	assert(num_busy < NUM_BUFFERS);
	dequeue();

	V4L2Buf_VAF = V4L2_Frame_Param.V4L2_Frame_Addr_cur[0].Luma_VA;
	V4L2Buf_VAB = V4L2_Frame_Param.V4L2_Frame_Addr_cur[1].Luma_VA;
	V4L2Buf_VAL = V4L2_Frame_Param.V4L2_Frame_Addr_cur[2].Luma_VA;
	V4L2Buf_VAR = V4L2_Frame_Param.V4L2_Frame_Addr_cur[3].Luma_VA;
	V4L2Buf_FdF = V4L2_Frame_Param.V4L2_Frame_Fd_cur[1];
	V4L2Buf_FdB = V4L2_Frame_Param.V4L2_Frame_Fd_cur[0];
	V4L2Buf_FdL = V4L2_Frame_Param.V4L2_Frame_Fd_cur[2];
	V4L2Buf_FdR = V4L2_Frame_Param.V4L2_Frame_Fd_cur[3];

	V4L2Buf_4to1_Luma = V4L2_Frame_Param.V4L2_Frame_Addr_cur[4].Luma_VA;
	V4L2Buf_4to1_Chroma = V4L2_Frame_Param.V4L2_Frame_Addr_cur[4].Chroma_VA;

	

	g_avm_detect_camera[front_camera] = (V4L2_Frame_Param.V4L2_Frame_Calc_Val[front_camera].Y > 5)?normal:failure;
	g_avm_detect_camera[back_camera]  = (V4L2_Frame_Param.V4L2_Frame_Calc_Val[back_camera].Y  > 5)?normal:failure;
	g_avm_detect_camera[left_camera]  = (V4L2_Frame_Param.V4L2_Frame_Calc_Val[left_camera].Y  > 5)?normal:failure;
	g_avm_detect_camera[right_camera] = (V4L2_Frame_Param.V4L2_Frame_Calc_Val[right_camera].Y > 5)?normal:failure;

	if(touch_write_flag == 1 && _autosys_avm_info.avm_view_page == 16)
	{
		cv::Mat rgb;
		cv::Mat yuv_yuyv(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAF);
		cv::cvtColor(yuv_yuyv, rgb, cv::COLOR_YUV2BGR_YUYV);
		cv::imwrite("/media/flash/avm/EVSTable/input_img/front.png", rgb);

		cv::Mat yuv_yuyv1(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAB);
		cv::cvtColor(yuv_yuyv1, rgb, cv::COLOR_YUV2BGR_YUYV);
		cv::imwrite("/media/flash/avm/EVSTable/input_img/back.png", rgb);

		cv::Mat yuv_yuyv2(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAL);
		cv::cvtColor(yuv_yuyv2, rgb, cv::COLOR_YUV2BGR_YUYV);
		cv::imwrite("/media/flash/avm/EVSTable/input_img/left.png", rgb);

		cv::Mat yuv_yuyv3(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAR);
		cv::cvtColor(yuv_yuyv3, rgb, cv::COLOR_YUV2BGR_YUYV);
		cv::imwrite("/media/flash/avm/EVSTable/input_img/right.png", rgb);
		Fisheye_file_count = 0;
		touch_write_flag = 0;
		AVM_view_page2.data[3] = 0x01;
		AVM_view_page2.data[4] = 0x03;
		printf("Is touch panel to write the picture\n");
		system("sync\n");
	}

	if(Fisheye_capture == 1)
	{

		char directory_path_buffer[200]; // check EVSTable
		char usb_path_buffer[200]; // check sda1'sdb1'sdc1'
		char usbID[][10] = {"sda1","sda", "sdb1", "sdc1"};
		int checkflag = 0;
		// for (auto id : usbID)
		// {
		// 	sprintf(directory_path_buffer, "/media/%s/EVSTable", id);
		// 	if (access(directory_path_buffer, F_OK) == 0)
		// 	{
		// 		printf("The %s exists.\n", directory_path_buffer);
		// 		checkflag = 1;
		// 		break;
		// 	}
		// }
		for (auto id : usbID)
		{
			sprintf(usb_path_buffer, "/media/%s/", id);
			if (access(usb_path_buffer, F_OK) == 0)
			{
				printf("The %s exists.\n", usb_path_buffer);
			sprintf(directory_path_buffer, "/media/%s/EVSTable", id);
				
				if(access(directory_path_buffer, F_OK)==0){
					checkflag = 1;
				}else{
					std::string MKDIR_common_EVSTable = std::string("mkdir -p") +" "+ usb_path_buffer + "EVSTable/";
					system(MKDIR_common_EVSTable.c_str());
					printf("Create Dir EVSTable in %s.\n", id);
				checkflag = 1;
				}
				break;
			}
		}

		if(access(directory_path_buffer, F_OK) == 0 && (checkflag = 1))
		{
		USB_EVSTABLE_LOCATION = directory_path_buffer;
		std::string input_img = USB_EVSTABLE_LOCATION + "/input_img/";
			std::string MKDIR_common_front = std::string("mkdir -p") +" "+ input_img + "front/";
		system(MKDIR_common_front.c_str());

			std::string MKDIR_common_back = std::string("mkdir -p") +" " + input_img + "back/";
		system(MKDIR_common_back.c_str());

			std::string MKDIR_common_left = std::string("mkdir -p") +" " + input_img + "left/";
		system(MKDIR_common_left.c_str());

			std::string MKDIR_common_right = std::string("mkdir -p") +" " + input_img + "right/";
		system(MKDIR_common_right.c_str());

		char fisheye_output_name[1024];
			std::time_t currentTime = std::time(nullptr);
			std::tm* localTime = std::localtime(&currentTime);
			char timeString[80]; 
			std::strftime(timeString, sizeof(timeString), "%m%d_%H%M%S", localTime);

		cv::Mat rgb;
		cv::Mat yuv_yuyv(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAF);
		cv::cvtColor(yuv_yuyv, rgb, cv::COLOR_YUV2BGR_YUYV);
				sprintf(fisheye_output_name,"%sfront/front_%s_%d.png",input_img.c_str(),timeString,Fisheye_file_count);
		cv::imwrite(fisheye_output_name, rgb);
			printf("Image Save = %s\n", fisheye_output_name);

		cv::Mat yuv_yuyv1(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAB);
		cv::cvtColor(yuv_yuyv1, rgb, cv::COLOR_YUV2BGR_YUYV);
				sprintf(fisheye_output_name,"%sback/back_%s_%d.png",input_img.c_str(),timeString,Fisheye_file_count);
		cv::imwrite(fisheye_output_name, rgb);
			printf("Image Save = %s\n", fisheye_output_name);

		cv::Mat yuv_yuyv2(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAL);
		cv::cvtColor(yuv_yuyv2, rgb, cv::COLOR_YUV2BGR_YUYV);
				sprintf(fisheye_output_name,"%sleft/left_%s_%d.png",input_img.c_str(),timeString,Fisheye_file_count);
		cv::imwrite(fisheye_output_name, rgb);
			printf("Image Save = %s\n", fisheye_output_name);

		cv::Mat yuv_yuyv3(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAR);
		cv::cvtColor(yuv_yuyv3, rgb, cv::COLOR_YUV2BGR_YUYV);
				sprintf(fisheye_output_name,"%sright/right_%s_%d.png",input_img.c_str(),timeString,Fisheye_file_count);
		cv::imwrite(fisheye_output_name, rgb);
			printf("Image Save = %s\n", fisheye_output_name);

		Fisheye_capture = 0;
		Fisheye_file_count+=1;
		AVM_view_page2.data[3] = 0x02;
		AVM_view_page2.data[4] = 0x04;
		printf("Fisheye_capture to write the picture\n");
			printf("Sucessfully to oneshoot 4 to 1.\n");
		system("sync\n");
	}
		else
		{
			printf("The folder does not exist.\n");
			printf("Fail to oneshoot.\n");
			Fisheye_capture = 0;
		}	
	}


	buffer = window_next_buffer(window);

	if (!buffer) {
		fprintf(stderr,
				!callback ? "Failed to create the first buffer.\n" : "All buffers busy at redraw(). Server bug?\n");
		abort();
	}

	if (buffer->release_fence_fd >= 0) {
		wait_for_buffer_release_fence(buffer);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->gl_fbo);


		pass_campa(CamInfo.V4L2Buf_phyAddr[V4L2_Frame_Param.V4L2_curIDX][0], CamInfo.V4L2Buf_phyAddr[V4L2_Frame_Param.V4L2_curIDX][1], CamInfo.V4L2Buf_phyAddr[V4L2_Frame_Param.V4L2_curIDX][2], CamInfo.V4L2Buf_phyAddr[V4L2_Frame_Param.V4L2_curIDX][3]);
		panel_fbo(buffer->gl_fbo);
		panel_depth(buffer->gl_depth);
		get_cam_va_from_8368_p(V4L2Buf_VAF, V4L2Buf_VAB, V4L2Buf_VAL, V4L2Buf_VAR);
		routine_avm(&user_command, &canBus, _autosys_avm_info,&_AVM_set_MOD_info);
		__IVI_SET_WRITECUR_FRAMEBUFFER__(_IVI_get_STATUS_info.data[7]);
		// printf("_AVM_set_MOD_info.data[1] = %d\n",_AVM_set_MOD_info.data[1]);
		// printf("_AVM_set_MOD_info.data[2] = %d\n",_AVM_set_MOD_info.data[2]);
		// printf("_AVM_set_MOD_info.data[3] = %d\n",_AVM_set_MOD_info.data[3]);
		// printf("_AVM_set_MOD_info.data[4] = %d\n",_AVM_set_MOD_info.data[4]);
	

	if (window->display->use_explicit_sync)
	{
		int fence_fd = create_egl_fence_fd(window);
		zwp_linux_surface_synchronization_v1_set_acquire_fence(window->surface_sync, fence_fd);
		close(fence_fd);
		buffer->buffer_release =zwp_linux_surface_synchronization_v1_get_release(window->surface_sync);
		zwp_linux_buffer_release_v1_add_listener(buffer->buffer_release, &buffer_release_listener, buffer);
	}
	else
	{
		glFinish();
	}

	wl_surface_attach(window->surface, buffer->buffer, 0, 0);
	wl_surface_damage(window->surface, 0, 0, window->width, window->height);

	if (callback) {
		wl_callback_destroy(callback);
	}

	window->callback = wl_surface_frame(window->surface);
	wl_callback_add_listener(window->callback, &frame_listener, window);
	wl_surface_commit(window->surface);
	buffer->busy = 1;

	if (!_queue(window->buffers)) {
		fprintf(stderr, "Failed to _queue buffers\n");
		abort();
	}
}

void view_4to1(void)
{
	printf("4to1\n");
	VIEW = VIEW_RECORD_4_TO_1;
}
void view_front(void)
{
	printf("front\n");
	VIEW = FRONT_VIEW;
}
void view_back(void)
{
	printf("back\n");
	VIEW = BACK_VIEW;
}
void view_left(void)
{
	printf("left\n");
	VIEW = LEFT_VIEW;
}
void view_right(void)
{
	printf("right\n");
	VIEW = RIGHT_VIEW;
}
struct window *global_win;

/* Enable iw0, mipi csi/v656, avm layer
*
*/
void layer_enabled(void)
{
	int ret;
	printf("layer status == %d\n", CurrentLayerStatus);
	if(CurrentLayerStatus==1)
	{
		printf("layer already enabled\ndo nothing\n");
		return;
	}

	ret = sem_wait(layer_sem);
	// Enable IW0
	myInstance->ADAS_Cam_IW_Ctrl_API(0, enable);
	// Enable MIPI-CSI or V656
	myInstance->ADAS_Cam_Video_Input_Ctrl_API(enable);
	avm_layer_enable(enable, global_win->display);

	CurrentLayerStatus = 1;
	ret = sem_post(layer_sem);

	printf("layer enable done\n");

}

/* Disble avm layer, mipi csi/v656, iw0. When mipi csi/v656 is disabled, there is no camera input
* data to all iw(0/1/2).
*/
void layer_disabled(void)
{
	int ret;
	printf("layer status == %d\n", CurrentLayerStatus);
	if(CurrentLayerStatus==0)
	{
		printf("layer already disabled\ndo nothing\n");
		return ;

	}

	ret = sem_wait(layer_sem);
	avm_layer_enable(disable, global_win->display);
	// Disable MIPI-CSI or V656
	myInstance->ADAS_Cam_Video_Input_Ctrl_API(disable);
	// Disable IW0
	myInstance->ADAS_Cam_IW_Ctrl_API(0, disable);
	CurrentLayerStatus = 0;
	ret = sem_post(layer_sem);

	printf("layer disabled done\n");

}

void Decode_IVI_AVM_common()
{
	//ioctl(fd_autosys, AVM_get_AVM_view_page, &AVM_view_page);
	// printf("_AVM_get_CONTRL_info.data[3] = %d\n",_AVM_get_CONTRL_info.data[3]);
	// printf("touch_write_flag = %d\n",touch_write_flag);
	if (_AVM_get_CONTRL_info.data[3] == 0x01 && touch_write_flag == 0)
	{
		if (AVM_view_page2.data[3] == 0x00)
		{
			_AVM_get_CONTRL_info.data[3] = 0x00;
			ioctl(fd_autosys, IVI_set_CONTRL_info, &_AVM_get_CONTRL_info);
			touch_write_flag = 1;
		}
	}
	if (AVM_view_page2.data[3] == 0x01)
	{
		if (AVM_view_page2.data[4] == 0x03)
		{
			if (touch_write_flag == 0)
			{
				_AVM_get_CONTRL_info.data[3] = 0x02;
				ioctl(fd_autosys, AVM_set_CONTRL_info, &_AVM_get_CONTRL_info);
				AVM_view_page2.data[3] = 0x00;
			}
		}
	}

	if (_AVM_get_CONTRL_info.data[4] == 0x01 && Fisheye_capture == 0)
	{
		if (AVM_view_page2.data[3] == 0x00)
		{
			_AVM_get_CONTRL_info.data[4] = 0x00;
			ioctl(fd_autosys, IVI_set_CONTRL_info, &_AVM_get_CONTRL_info);
			Fisheye_capture = 1;
		}
	}

	if (AVM_view_page2.data[3] == 0x02)
	{
		if (AVM_view_page2.data[4] == 0x04)
		{
			if (Fisheye_capture == 0)
			{
				_AVM_get_CONTRL_info.data[4] = 0x02;
				ioctl(fd_autosys, AVM_set_CONTRL_info, &_AVM_get_CONTRL_info);
				AVM_view_page2.data[3] = 0x00;
			}
		}
	}


	// if (AVM_view_page.data[1] == 0x02)
	{
		if(_AVM_get_CONTRL_info.data[0] == 0x01)
		{
			_AVM_get_CONTRL_info.data[0] = 0x00;
			 ioctl(fd_autosys, IVI_set_CONTRL_info, &_AVM_get_CONTRL_info);
			GO_to_QT_flag = 1;
		}
	}

	if (AVM_view_page.data[1] == 0x02)
	{
		if(AVM_view_page.data[2] == 0x04)
		{
			AVM_view_page.data[1] = 0x00;
			AVM_view_page.data[2] = 0x00;
			AVM_view_page.data[3] = 0x02;
			AVM_view_page.data[4] = 0x05;
			ioctl(fd_autosys, IVI_set_AVM_view_page, &AVM_view_page);
			GO_to_QT_flag = 1;
		}
	}
}

void __AVM_CALL_CONTORL_IVI__(int index, int value)
{
	_AVM_get_CONTRL_info.data[index] = value;
	ioctl(fd_autosys, AVM_set_CONTRL_info, &_AVM_get_CONTRL_info);
}

int get_layer_status(void)
{
	return current_avm_status;
}
int get_avm_status(void)
{
	return running;
}

int exit_avm()
{
	if(CurrentLayerStatus==0)
	{
		printf("Can't not exit AVM,due to layer is disabled.\n");
		printf("pleas enable layer\n");
		return -1;

	}
	printf("exit avm\n");
	running = 0;
	return 0;
}

float calculate_time(struct timeval start, struct timeval end)
{
	float dtime;
	dtime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)* 0.000001;
	// printf("dtime %f\n", dtime);
	return dtime;
}

double START,END; 
#if AVM_IVI_INTERFACE

void*avm_EN_DIS_func(void* ptr)
{
	// Init_Panel_monitor();
	while(1)
	{
		ioctl(fd_autosys, AVM_get_AVM_view_page, &AVM_view_page1);
		// printf("(AVM_view_page1.data[0] = %d\n",AVM_view_page1.data[0]);
		if (AVM_view_page1.data[0] > 0)
		{
			_autosys_avm_info.avm_view_page = (AVM_view_page1.data[0]) - 1;
		}
		// if(AVM_view_page1.data[0] == 0 && amv_layer_enable_flag == 0)
		if(AVM_view_page1.data[0] == 0)
		{
			if (CurrentLayerStatus == 1)
			{
				paused = 1;
				SetLayerDisable = 1;
			}
			amv_layer_enable_flag = 1;
			rolate_360_flag  = 0;
			lock_page_roate = 0;
		}
		// else if(AVM_view_page1.data[0] != 0 && amv_layer_enable_flag == 1)		
		else if(AVM_view_page1.data[0] != 0 && AVM_view_page1.data[0] !=99 && AVM_view_page1.data[0] !=100)
		{		
			if (CurrentLayerStatus == 0)
			{
				paused = 0;
				SetLayerEnable = 1;
			}
			amv_layer_enable_flag = 0;
		}
		usleep(50000);
	}
};
#endif

#if (Board == board6702 || Board == board8368)
#include "../../application/sdk/include/SPPlatformControl.h"
//#include "../../application/sdk/include/SPReverseControl.h"
using namespace spplfctrl;
//using namespace reversectrl;
#endif
//#include "../../appSPPlatformControl.h"
//#include "SPReverseControl.h"
// using namespace spappframework;

char *EVSTable_path_write_png[4];
char front_png[1024];
char back_png[1024];
char left_png[1024];
char right_png[1024];


void init_read_config_setting(const char dir[])
{
	FILE *fp_EVSTable;
	size_t len = 0;
	ssize_t read;
	int read_EVStable_path_count = 0;
	fp_EVSTable = fopen(dir, "r");
	if (fp_EVSTable == NULL)
	{
		printf("EVStable_path.config Load error!!!\n");
		char *path2 = "/tmp/";
		EVSTable_path_write_png[2] = path2;
	}
	else
	{

#ifdef CLOCK_LOGGIN
		clock_gettime(CLOCK_MONOTONIC, &adas_cam_start);
#else
		gettimeofday(&adas_cam_start, NULL);
#endif

		while ((getline(&EVSTable_path_write_png[read_EVStable_path_count], &len, fp_EVSTable)) != -1)
		{
			char *temp1 = EVSTable_path_write_png[read_EVStable_path_count];
			temp1[strcspn(EVSTable_path_write_png[read_EVStable_path_count], "\n\r")] = 0;
			printf("EVSTable_path[%d] = %s!!!!!!!!\n", read_EVStable_path_count, EVSTable_path_write_png[read_EVStable_path_count]);
			read_EVStable_path_count++;
#ifdef CLOCK_LOGGIN
			clock_gettime(CLOCK_MONOTONIC, &adas_cam_end);
			float through_time = (adas_cam_end.tv_sec - adas_cam_start.tv_sec) +
								 (adas_cam_end.tv_nsec - adas_cam_start.tv_nsec) / 1e9f;
#else
			gettimeofday(&adas_cam_end, NULL);
			float through_time = calculate_time(adas_cam_start, adas_cam_end);
#endif
			if (through_time >= 3.0f)
			{
				printf("read EVSTable_path overtime %f s\n", through_time);
				break;
			}
		}

		fclose(fp_EVSTable);
	}

	char mkdir_flash_cmd[1024];
	sprintf(mkdir_flash_cmd, "mkdir %sEVSTable/", EVSTable_path_write_png[2]);
	system(mkdir_flash_cmd);
	sprintf(mkdir_flash_cmd, "mkdir %sEVSTable/input_img/", EVSTable_path_write_png[2]);
	system(mkdir_flash_cmd);


	sprintf(front_png, "%sEVSTable/input_img/front.png", EVSTable_path_write_png[2]);
	sprintf(back_png, "%sEVSTable/input_img/back.png",   EVSTable_path_write_png[2]);
	sprintf(left_png, "%sEVSTable/input_img/left.png",   EVSTable_path_write_png[2]);
	sprintf(right_png, "%sEVSTable/input_img/right.png", EVSTable_path_write_png[2]);

}

char * avm_main_version()
{
	#if fasle
	static char avm_version_with_datetime[50];
	//char *avm_version = "T-mb04-01";
	snprintf(avm_version_with_datetime, sizeof(avm_version_with_datetime), 
             "v10-00-00  %s", __DATE__);
	
	return avm_version_with_datetime;
	#else
		return usr_get_libautosys_version();
	#endif
}

vector<string> spliteStr(const string& str,char delimiter){
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream,token,delimiter)){
        tokens.push_back(token);
    }
    return tokens;
}
void paeseDataToAvmInfo(const string& key,const string& value,avmJsonData& d){
    char* charKey = new char[key.length()+1];
    strcpy(charKey,key.c_str());
    if(strcmp(charKey,"avm.carColor")==0)
    {
        d.color = stoi(value.c_str());
    }
    else if (strcmp(charKey,"avm.mod")==0)
    {
        istringstream(value)>>boolalpha>>d.mod;
    }
    else if (strcmp(charKey,"avm.page")==0)
    {
        d.page = std::stoi(value.c_str());
    }
    else
    {
        istringstream(value)>>boolalpha>>d.turn;
    }
}
int openAvmJsonFile(string& str){
    string result;
    ifstream file("/media/flash/userdata/avmInfo.json");
    if(!file.is_open()){
        cerr<< "Error !! Cant Open File"<<endl;
        return -1;
    }else{
        while (getline(file,str)){}
        file.close();
        return 0;
    }
}
avmJsonData getAvmJsonData(){
    string str;
	avmJsonData result;
	//開檔失敗
	if(openAvmJsonFile(str)!=0){
		cout<< "Open Json Error !!!!!"<< endl;
		cout << "use default result!!!"<<endl;
		cout <<"result.color = " <<result.color<<endl;
		return result;
	}
    
	
    //去掉頭尾 {} 符號
    string formatString = str.substr(1,str.length()-2);
	cout << "formatString" << formatString << endl;
    //切割字串存入 patrs
    vector<string> parts = spliteStr(formatString,',');
    
    for(const auto& part : parts){
        //切割自串成 key value
        vector<string> keyValue = spliteStr(part,':');
        if(keyValue.size() == 2){
            //去掉頭尾 "" 符號
            string key = keyValue[0].substr(1,keyValue[0].length()-2);
            string value = keyValue[1];
            //把 value 存進 avmInfo
            paeseDataToAvmInfo(key,value,result);
        }
    }
	cout << "color" << result.color << endl;
	cout << "mod" << result.mod << endl;
	cout << "page" << result.page << endl;
	cout << "turn" << result.turn << endl;
	return result;
}

void ensure_data_written(const char *file_path) {
    int fd = open(file_path, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    if (fsync(fd) == -1) {
        perror("fsync");
    } else {
        printf("Data has been synchronized to disk for file: %s\n", file_path);
    }

    close(fd);
}
void *auto_update(void *ptr)
{
	#ifdef AUTOSYS_CAN_SERVICE
	#if (VEHICLE_TYPE == __URX__)
		printf("[###] echo 3 > /dev/ttyS9\n");
		system("echo 3 > /dev/ttyS9");
	#else
		printf("[###] echo 3 > /dev/ttyUSB0\n");
		sleep(10);
		system("stty -F /dev/ttyUSB0 speed 921600");
	#endif
	read_ttySData();
	return NULL;
	#endif
}


#if (Board == board6702)
int run_avm(int argc, char **argv)
#else if (Board == board8368)
int main(int argc, char **argv)
#endif
{

	#ifdef USB_LOGGIN
		init_read_config_setting("./EVStable_path.config");
	#else
		init_read_config_setting("application/avm_usb/EVStable_path.config");
	#endif
	// Andy Add 05/03
	// 打開 Avm Json檔 把參數傳入 _avm_json_data
	#if (Board == board6702)
	system("cat /media/flash/userdata/avmInfo.json");
	#endif
	_avm_json_data = getAvmJsonData();
	printf("\n_avm_json_data.color = %d!!!!!!!\n",_avm_json_data.color);
	current_avm_status = 1;
	bool adas_cam_getservice_status = false;

	#if userself2defineAVMPAGE
	pthread_t user_set_avm_page;
	pthread_create(&user_set_avm_page, NULL, user_set_AVMPAGE_fun, (void*)getAVMPAGEsharememory() ); // 建立子執行緒
	pthread_detach(user_set_avm_page); 
	#endif
/*canBus*/
#ifdef AUTOSYS_CAN_SERVICE
	pthread_t auto_update_thread = (pthread_t)0;
	pthread_create(&auto_update_thread, NULL, auto_update, NULL);
	pthread_detach(auto_update_thread);
#endif


#if AVM_IVI_INTERFACE 
	// system("mkdir ./avm_img/");
	fd_autosys = open("/dev/autosys_avm_cmd", O_RDWR);
	_IVI_set_SYSTEM_info.data[2] = _avm_json_data.color;
	ioctl(fd_autosys, IVI_set_SYSTEM_info, &_IVI_set_SYSTEM_info);

	ioctl(fd_autosys, IVI_get_CONTRL_info, &_AVM_get_CONTRL_info);
	if (_AVM_get_CONTRL_info.data[5] == 0x00)
	{
		printf("!!!!!!!!!!!!!!!!!_AVM_get_CONTRL_info.data[5] = %d\n", _AVM_get_CONTRL_info.data[5]);
		printf("Start run_avm!!!!!!\n");
#if (Board == board6702)
		// system("preparatory&"); //開機logo 
		printf("Start simple-AppReady2CtrlReverse!!!!!!\n");
		
		// system("/application/bin/simple-AppReady2CtrlReverse&"); //執行換手
		printf("End simple-AppReady2CtrlReverse!!!!!!\n");
#endif
		_AVM_get_CONTRL_info.data[3] = 0x00;
		_AVM_get_CONTRL_info.data[5] = 0x01;
		ioctl(fd_autosys, AVM_set_CONTRL_info, &_AVM_get_CONTRL_info);
	}
	ioctl(fd_autosys, AVM_get_AVM_view_page, &AVM_view_page);
	ioctl(fd_autosys, AVM_get_SYSTEM_info, &_AVM_get_SYSTEM_info);
	ioctl(fd_autosys, AVM_get_turn_signal_info, &_AVM_get_turn_signal_info);
	ioctl(fd_autosys, AVM_get_PGL_style_info, &_AVM_get_PGL_style_info);
	if (AVM_view_page.data[0] > 0 && AVM_view_page.data[0]!=99 && AVM_view_page.data[0]!=100)
	{
		_autosys_avm_info.avm_view_page = (AVM_view_page.data[0]) - 1;
	}
	pthread_t avm_enable_or_disable;
	pthread_create(&avm_enable_or_disable, NULL, avm_EN_DIS_func, NULL); // 建立子執行緒
#endif
#if 1
	
#endif

	if(running == 0)
		running = 1;
	else if(running == 1)
	{
		printf("avm is already running\n");
		return -1;
	}
	
/*---------------------------*/
/*--read avm system config---*/
	#ifdef USB_LOGGIN
		read_avm_sysyem_file("./EVStable_path.config");
	#else
		read_avm_sysyem_file("application/avm_usb/EVStable_path.config");
	#endif
	int panel_width, panel_height;
	usr_get_current_panel_resolution(&panel_width, &panel_height);
	ifstream file("/media/flash/userdata/avmInfo.json");
    if(!file.is_open()){
        cerr<< "Error !! Cant Open File"<<endl;
		usr_set_default_color(-1);
    }else{
		usr_set_default_color(_avm_json_data.color);
	}
	file.close();
	
/*---------------------------*/


	struct display *display;
	struct window *window;
	int opts = 0;
	int c, option_index, ret = 0;
	char v4l2FMT[8];
	char const *drm_render_node = "tako";
	layer_sem = sem_open("layer_sem", O_CREAT, 0644, 1);
	Cam_PixelFMT = 1;
	display = create_display(drm_render_node, opts);
	
	if (!display) {
		return 1;
	}
	display->format.format = 1448695129;

	
	window = create_window(display, panel_width, panel_height, opts);
	if (!window) {
		return 1;
	}

	GLuint *fbo_data = (GLuint *)malloc(3 * sizeof(GLuint));
	fbo_data[0] = window->buffers->gl_fbo;
	fbo_data[1] = window->buffers->gl_texture;
	fbo_data[2] = window->buffers->gl_depth;
	get_panel_4to1_fbo(fbo_data);
	#ifdef USB_LOGGIN
		usr_seethrough_resoultion_level(SET_INFO, 0.0f);
		init_avm("./EVStable_path.config");
	#else
		init_avm("application/avm_usb/EVStable_path.config");
	#endif

	#if userself2defineAVMPAGE
	int setlimitbuffer[4] = {-90,90,-180,180};
	usr_set_freetouch_limit(0,setlimitbuffer);
	#endif

	// #if (Board == board6702)
	// SPPlatformControl *spPfcCtrl;
	// spPfcCtrl = SPPlatformControl::GetInstance();
	// if (spPfcCtrl)
	// {
	// 	PFCResult_e eRet = spPfcCtrl->PFC_AppReady2CtrlReverse(true);

	// 	if (eRet != PFC_SUCCESS)
	// 	{
	// 		printf("PFC_AppReady2CtrlReverse() failed.Plz check.");
	// 	}
	// 	else
	// 	{
	// 		printf("PFC_AppReady2CtrlReverse() success.");
	// 	}
	// }else{
	// 		printf("SPPlatformControl::GetInstance() FAIL!!!!!!!!!!!!!!!!!!!");
	// }
	// //spPfcCtrl->Stop_Animation_Logo(0);
	// spPfcCtrl->Stop_Logo(0);
	// SPPlatformControl::ReleaseInstance(spPfcCtrl);
	// #endif
	global_win=window;

	if (!v4l_init(display, window->buffers)) {
		return 1;
	}	
	wl_display_roundtrip(display->display);
	
	//3 s
	#ifdef CLOCK_LOGGIN
		clock_gettime(CLOCK_MONOTONIC, &adas_cam_start);
	#else
		gettimeofday(&adas_cam_start, NULL);
	#endif
	while (adas_cam_getservice_status == false)
	{
		myInstance = SP_adas_cam::GetInstance();
		if (!myInstance)
		{
			printf("4to1 failed to get adas_cam instance, retry...\n");
			usleep(100);
		}
		else
		{
			printf("4to1 get adas_cam instance successfully.\n");
			adas_cam_getservice_status = true;
		}

		#ifdef CLOCK_LOGGIN
			clock_gettime(CLOCK_MONOTONIC, &adas_cam_end);
    		float through_time = (adas_cam_end.tv_sec - adas_cam_start.tv_sec) + 
                         (adas_cam_end.tv_nsec - adas_cam_start.tv_nsec) / 1e9f;
		#else
			gettimeofday(&adas_cam_end, NULL);
			float through_time = calculate_time(adas_cam_start, adas_cam_end);
		#endif
		if(through_time >= 3.0f)
		{
			printf("4to1 get adas_cam instance failure. Overtime %f s\n", through_time);
			break;
		}
	}
	/* User setting */
	// int static_car_Page[2] = {12, 13};
	// usr_carModel_static_Draw_Page(SET_INFO, static_car_Page, 2);
	usr_seethrough_2D_car_alpha_value(SET_INFO, 0.1f);
	usr_seethrough_3D_car_alpha_value(SET_INFO, 0.1f);
	usr_seethrough_3D_car_tire_alpha_value(SET_INFO, 0.1f);
	usr_seethrough_3D_car_windows_alpha_value(SET_INFO, 0.1f);
	usr_set_3DcarModel_doorStatus(1,1,0,0,0);
	float opendoor_color[] = {0.5f, 0.5f, 1.0f};
	usr_set_avm_opendoor_view(1, opendoor_color);
	usr_set_carModel_shadow_drawMode(1);
	usr_3dcarModel_lamp_flowing_para(SET_INFO, FW_REAR, 1, 5);
	/* end */
	v4l_connect();
	window->initialized = true;
	if (!window->wait_for_configure) {
		_autosys_avm_info.car_color = _avm_json_data.color;
		redraw(window, NULL, 0);
	}

	bool do_once_time = false;
	stTouchData touchdata_main;
	touchdata_main.x = 0.0f;
	touchdata_main.y = 0.0f;

	stTouchData touchdata_main_before = {0};
	int car_color = 0;
	int prinf_flag = 1;
	int frame_count11 =0;
	int change_flag = 0;
	while (running && ret != -1)
	{
		// if (frame_count11 % 500 == 0)
		// {
		// 	switch (change_flag)
		// 	{
		// 	case 0:
		// 		usr_seethrough_frame_TuneArea(SET_INFO, 0.00f, 0.95f);
		// 		change_flag = 1;
		// 		break;

		// 	case 1:
		// 		usr_seethrough_frame_TuneArea(SET_INFO, 0.00f, 1.0f);
		// 		change_flag = 2;
		// 		break;

		// 	case 2:
		// 		usr_seethrough_frame_TuneArea(SET_INFO, 0.00f, 1.05f);
		// 		change_flag = 0;
		// 		break;
		// 	}
		// }
		// frame_count11++;
		ret = wl_display_dispatch(display->display);

		ioctl(fd_autosys, AVM_get_AVM_view_page, &AVM_view_page);
		ioctl(fd_autosys, AVM_get_SYSTEM_info, &_AVM_get_SYSTEM_info);
        ioctl(fd_autosys, AVM_get_turn_signal_info, &_AVM_get_turn_signal_info);
        ioctl(fd_autosys, AVM_get_PGL_style_info, &_AVM_get_PGL_style_info);
		ioctl(fd_autosys, AVM_get_CONTRL_info, &_AVM_get_CONTRL_info);
		ioctl(fd_autosys, AVM_get_MOD_info, &_AVM_get_MOD_info);
		ioctl(fd_autosys, IVI_get_STATUS_info, &_IVI_get_STATUS_info);
		ioctl(fd_autosys, IVI_get_CONTRL_CarModel_info, &_IVI_get_CONTRL_CarModel_info);
		ioctl(fd_autosys, IVI_get_CONTRL_touch_info, &_IVI_get_CONTRL_touch_info);
		Decode_IVI_AVM_common();
		if (AVM_view_page.data[0] > 0 &&AVM_view_page.data[0] !=99 && AVM_view_page.data[0] !=100 )
		{
			if (cam_ch_change_count < 1 && (_autosys_avm_info.avm_view_page != ((AVM_view_page.data[0]) - 1))){
				cam_ch_change_count++;
			}	
			else if (cam_ch_change_count >= 1)
			{
				_autosys_avm_info.avm_view_page = (AVM_view_page.data[0]) - 1;
				cam_ch_change_count = 0;
			}
		}
		#ifndef AUTOSYS_CAN_SERVICE
		_autosys_avm_info.vehicle_speed         = (float)_AVM_get_SYSTEM_info.data[0]; 
		_autosys_avm_info.gear                  = (_AVM_get_SYSTEM_info.data[1] & 0xF);
		_autosys_avm_info.steering_angle        = _AVM_get_SYSTEM_info.data[8];
		#else
		g_speed = 10;
		_autosys_avm_info.vehicle_speed         = g_speed; 
		_autosys_avm_info.gear                  = g_gear;
		_autosys_avm_info.steering_angle        = g_steering_angle;
		#endif
		_autosys_avm_info.speed_decide          = (_AVM_get_SYSTEM_info.data[1] & 0xF0) >> 4 ;
		_autosys_avm_info.car_color             = _AVM_get_SYSTEM_info.data[2];
		_autosys_avm_info.touch_x_H             = _AVM_get_SYSTEM_info.data[3];
		_autosys_avm_info.touch_x_L             = _AVM_get_SYSTEM_info.data[4];
		_autosys_avm_info.touch_y_H             = _AVM_get_SYSTEM_info.data[5];
		_autosys_avm_info.touch_y_L             = _AVM_get_SYSTEM_info.data[6];
		_autosys_avm_info.vehicle_alpha         = 0;//_AVM_get_SYSTEM_info.data[7];
		//_autosys_avm_info.steering_angle        = _AVM_get_SYSTEM_info.data[8];
		_autosys_avm_info.door_signal           = _AVM_get_SYSTEM_info.data[9];
		_autosys_avm_info.light_signal          = _AVM_get_SYSTEM_info.data[10];
		_autosys_avm_info.turn_signal_flag      = _AVM_get_turn_signal_info.data[0];
		_autosys_avm_info.PGL_style_flag        = _AVM_get_PGL_style_info.data[0];
		_autosys_avm_info.TRN_ON_OFF            = _AVM_get_CONTRL_info.data[2];
		_autosys_avm_info.mod_ON_OFF            = _AVM_get_MOD_info.data[0];
		_autosys_avm_info.AVM_PID_Latancy       = _AVM_get_CONTRL_info.data[6];///
		_autosys_avm_info.cam_status.front[0]   = _IVI_get_STATUS_info.data[0] | g_avm_detect_camera[front_camera];
		_autosys_avm_info.cam_status.back[0]    = _IVI_get_STATUS_info.data[1] | g_avm_detect_camera[back_camera];
		_autosys_avm_info.cam_status.left[0]    = _IVI_get_STATUS_info.data[2] | g_avm_detect_camera[left_camera];
		_autosys_avm_info.cam_status.right[0]   = _IVI_get_STATUS_info.data[3] | g_avm_detect_camera[right_camera];
		_autosys_avm_info.mod_recall            = 1;
		_autosys_avm_info.muti_touchIndex       = _IVI_get_CONTRL_touch_info.data[0];
		_autosys_avm_info.ZOOM_ON_OFF           = _IVI_get_STATUS_info.data[6];
#ifdef AUTOSYS_CAN_SERVICE

		uint8_t rf_dr = (g_door & DOOR_FRONT_RIGHT_MASK) != 0;
		uint8_t lf_dr = (g_door & DOOR_FRONT_LEFT_MASK) != 0;
		uint8_t lb_dr = (g_door & DOOR_REAR_LEFT_MASK) != 0;
		uint8_t rb_dr = (g_door & DOOR_REAR_RIGHT_MASK) != 0;
		uint8_t trunk = (g_door & DOOR_TAILGATE_MASK) == DOOR_TAILGATE_MASK;

 		usr_set_2DcarModel_doorStatus(rb_dr, lb_dr, rf_dr, lf_dr, trunk);
		usr_set_3DcarModel_doorStatus(rb_dr, lb_dr, rf_dr, lf_dr, trunk);
#else
		uint8_t rb_dr = (_IVI_get_CONTRL_CarModel_info.data[4] > 0) ? 1 : 0;
		uint8_t lb_dr = (_IVI_get_CONTRL_CarModel_info.data[5] > 0) ? 1 : 0;
		uint8_t rf_dr = (_IVI_get_CONTRL_CarModel_info.data[6] > 0) ? 1 : 0;
		uint8_t lf_dr = (_IVI_get_CONTRL_CarModel_info.data[7] > 0) ? 1 : 0;
		uint8_t trunk = (_IVI_get_CONTRL_CarModel_info.data[8] > 0) ? 1 : 0;
		usr_set_2DcarModel_doorStatus(rb_dr, lb_dr, rf_dr, lf_dr, trunk);
#endif

		if (AVM_view_page1.data[0]==99) {
			bWhiteBalance = true;
		};
		if (AVM_view_page1.data[0]==100) {
			bWhiteBalance = false;
		};
		_autosys_avm_info.brightness_adj = bWhiteBalance;
		// printf("_autosys_avm_info.brightness_adj = %d\n",_autosys_avm_info.brightness_adj);
		// _autosys_avm_info.brightness_adj = 0;
		_autosys_avm_info.rotate_540_reset_flag = _AVM_get_CONTRL_info.data[5];
		// _autosys_avm_info.car_color = 0;

		_AVM_set_MOD_info.can_id = _AVM_get_MOD_info.can_id;
		_AVM_set_MOD_info.data[0] = _AVM_get_MOD_info.data[0];

		ioctl(fd_autosys, AVM_set_MOD_info, &_AVM_set_MOD_info);
		usr_set_current_3D_carModel_alpha(_autosys_avm_info.vehicle_alpha,0);
		touchdata_main_before.x = touchdata_main.x;
		touchdata_main_before.y = touchdata_main.y;

		//ioctl(fd_autosys, IVI_get_360_status, &_IVI_get_360_status);
		//printf("_IVI_get_STATUS_info.data[0] = %d\n",_IVI_get_STATUS_info.data[4]);
		DEBUG_CHECK_NECESSARY_IVI_STATUS(_IVI_get_STATUS_info.data[5]);
		//__IVI_SET_WRITECUR_FRAMEBUFFER__(_IVI_get_STATUS_info.data[7]);
		/* seethrough alhpa setting */

		// if(prinf_flag == 1)
		// {
		// 	clock_gettime(CLOCK_MONOTONIC, &adas_cam_end);
		// 	float through_time = (adas_cam_end.tv_sec - adas_cam_start.tv_sec) + 
        // 	                 (adas_cam_end.tv_nsec - adas_cam_start.tv_nsec) / 1e9f;
		// 	if(through_time > 0.3f)
		// 	{
		// 		float process = usr_get_seethroughprossValue();
		// 		AVM_LOGI("CURRENT SEETHROUGH VALUE %f\n", process);
		// 		clock_gettime(CLOCK_MONOTONIC, &adas_cam_start);
		// 		if(process >= 0.8f)
		// 		{
		// 			prinf_flag = 0;
		// 		}
		// 	}
		// }
#if 0
		usr_seethrough_2D_car_alpha_value(SET_INFO, (float)_IVI_get_CONTRL_CarModel_info.data[0] / 100);
		usr_seethrough_3D_car_alpha_value(SET_INFO, (float)_IVI_get_CONTRL_CarModel_info.data[1] / 100);
		usr_seethrough_3D_car_tire_alpha_value(SET_INFO, (float)_IVI_get_CONTRL_CarModel_info.data[2] / 100);
		usr_seethrough_3D_car_windows_alpha_value(SET_INFO, (float)_IVI_get_CONTRL_CarModel_info.data[3] / 100);
#endif

		usr_seethrough_2D_car_alpha_value(SET_INFO, (float) 50 / 100);
		usr_seethrough_3D_car_alpha_value(SET_INFO, (float)50 / 100);
		usr_seethrough_3D_car_tire_alpha_value(SET_INFO, (float)50 / 100);
		usr_seethrough_3D_car_windows_alpha_value(SET_INFO, (float)50 / 100);

		/* end */
	}
	mod_exit();
	close(fd_autosys);
	fprintf(stderr, "simple-dmabuf-4to1 exiting\n");


	cv::Mat rgb;
	cv::Mat yuv_yuyv(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAF);
	cv::cvtColor(yuv_yuyv, rgb, cv::COLOR_YUV2BGR_YUYV);
	cv::imwrite(front_png, rgb);

	cv::Mat yuv_yuyv1(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAB);
	cv::cvtColor(yuv_yuyv1, rgb, cv::COLOR_YUV2BGR_YUYV);
	cv::imwrite(back_png, rgb);

	cv::Mat yuv_yuyv2(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAL);
	cv::cvtColor(yuv_yuyv2, rgb, cv::COLOR_YUV2BGR_YUYV);
	cv::imwrite(left_png, rgb);

	cv::Mat yuv_yuyv3(imageHeight, imageWidth, CV_8UC2, V4L2Buf_VAR);
	cv::cvtColor(yuv_yuyv3, rgb, cv::COLOR_YUV2BGR_YUYV);
	cv::imwrite(right_png, rgb);

	printf("Is touch panel to write the picture\n");
	system("sync\n");


	printf("\033[35mFront camera png path = %s\033[0m\n",front_png);
	printf("\033[35mBack camera png path  = %s\033[0m\n",back_png);
	printf("\033[35mLeft camera png path  = %s\033[0m\n",left_png);
	printf("\033[35mRight camera png path = %s\033[0m\n",right_png);


	destroy_window(window);
	destroy_display(display);
	myInstance->ReleaseInstance(myInstance);
	running = 0;
	sem_unlink("layer_sem");
	sem_close(layer_sem);
	global_win=NULL;
	current_avm_status = 0;
	return 0;
}