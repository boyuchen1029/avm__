#ifndef IMS_TFLITE_H_
#define IMS_TFLITE_H_

// #define print_log

#include <iostream>
#include <sys/stat.h>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/tools/command_line_flags.h"
#include "tensorflow/lite/tools/delegates/delegate_provider.h"

#include <getopt.h> // NOLINT(build/include_order)
#include "absl/memory/memory.h"
#include "tensorflow/lite/examples/label_image/log.h"
#include "tensorflow/lite/profiling/profiler.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/kernels/cpu_backend_context.h"
#include "tensorflow/lite/kernels/register.h"

#include "omp.h"

#include "tfdevice.h"
#include "ims_gesture.h"
#include "ims_pfld.h"
#include "ims_facerecognition.h"
#include "ims_tracking.h"

#define btoa(x) ((x)?"true":"false")

#define PI 3.141592654

typedef struct BoxInfo_
{
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
} BoxInfo;

struct Vector3D {
    double x, y, z;
};

class IMS_TFLITE
{
public:
    int DMS_turn_head_func = 1;
    int DMS_close_eye_func = 1;
    int DMS_phone_func = 1;
    int DMS_smoke_func = 1;
    int IMS_person_id_func = 1;
    int IMS_gesture_func = 1;
    int IMS_seat_belt_func = 1;
    int IMS_passenger_deection_func = 1;

    bool driver_passenger = false;
    bool copilot_passenger = false;
    bool left_passenger = false;
    bool mid_passenger = false;
    bool right_passenger = false;

    bool ims_do_once_time = false;

    volatile bool add_member_flag;
    volatile int  add_member_state;

    bool draw_bbox = true;
    char face_recognition_text[50] = "Face Recognition ...";
    char passengers_text[50] = "Number of Passengers: detect...";
    std::vector<int> number_of_passengers_count;
    char seatbelt_driver_text[50] = "Driver Seatbelt State: detect...";
    char seatbelt_copilot_text[50] = "Copilot Seatbelt State: detect...";
    std::vector<int> seatbelt_driver_count;
    std::vector<int> seatbelt_copilot_count;

    /*  Driver Seat and Hand Gesture ROI  */
    int drivers_seat_roi = 830; // 670
    int copilot_seat_roi = 360; // 260
    int other_driver_roi = (830-360)/3;

    int image_width = 1280;     // 960
    cv::Rect hand_gesture_roi = cv::Rect(360, 155, 560, 425);    // (220, 90, 510, 370)
    float offset_yaw = 0.f;
    float offset_pitch = 0.f;
    int see_front_count = 0;

    /*  Seatbelt Detect ROI  */
    cv::Point2d  driver_seatbelt_roi = cv::Point2d(830, 430);   // (560, 240, 399, 299)
    cv::Point2d co_pilot_seatbelt_roi = cv::Point2d(460, 440);    // (0, 270, 350, 269)
    bool driver_seatbelt_flag = false;
    bool copilot_seatbelt_flag = false;

    /*  Hand Pose Estimation ID Survival Time Variable  */
    bool count_hand_init = true;
    int old_hand_id = 0;
    int count_hand_time = 0;

    /*  Hand Pose Estimation Variable */
    bool hand_estimation_init = true;
    float old_hand_x = 0;
    float old_hand_y = 0;
    std::vector<float> x_movement;
    std::vector<float> y_movement;
    float x_movement_final = 0;
    float y_movement_final = 0;
    int hand_pose_result = 16;

    /*  Smoke, Phone, Head, Wink Flag */
    int  phone_alert_count = 0;
    int  smoke_alert_count = 0;
    int  wink_alert_count = 0;
    int  head_alert_count = 0;
    std::vector<int> smoke_count;
    std::vector<int> phone_count;
    std::vector<int> wink_count;
    std::vector<int> head_count;

    int wink_sum_threhold = 15;

    bool smoke_flag = false;
    bool phone_flag = false;
    bool head_flag = false;
    bool wink_flag = false;

    bool left_flag = false;
    bool right_flag = false;
    bool down_flag = false;
    bool up_flag = false;

    /* Save User Information Variable */
    int face_angle = 0;
    std::vector<bool> save_userinfo_state {false};
    std::vector<std::vector<float>> gradient_feature;

    IMS_GESTURE ims_gesture;
    PFLD_TFLITE pfld_tflite;
    FACERECOGNITION_TFLITE facerecognition_tflite;
    IMS_tracking hand_tracking;
    
    void init(const char *model_file);
    ~IMS_TFLITE();
    void data_memset();
    void preprocess(cv::Mat &src);
    void Invoke();
    void postprocess(cv::Mat &img, std::vector<BoxInfo> &result_list);
    cv::Mat pfld_draw_result(cv::Mat &img, std::vector<BoxInfo> &result_list);
    cv::Mat add_member(cv::Mat image, std::vector<BoxInfo> &result_list);
    void decode_infer(const float *cls_pred, const float *dis_pred, int stride, const float *threshold, std::vector<std::vector<BoxInfo>> &results);
    BoxInfo disPred2Bbox(const float *&dfl_det, int label, float score, int x, int y, int stride);
    void nms(std::vector<BoxInfo> &input_boxes, float NMS_THRESH);
private:
    int hand_estimation_movement = 150;
    int alert_vector_count = 10;
    /* IMS Model Setting */
    int num_class = 5;
    int reg_max = 7;
    const float mean[3] = {0.f, 0.f, 0.f};
    const float scale[3] = {0.003921, 0.003921, 0.003921};
    const float score_threshold[5] = {0.6, 0.6, 0.6, 0.4, 0.6};
    float nms_threshold = 0.7;
    int num_thread = 1;
    int in_w = 320;
    int in_h = 320;

    int cls_pred_stride_8_pos = 3;              // 1x1600x2
    int cls_pred_stride_16_pos = 4;           // 1x400x2
    int cls_pred_stride_32_pos = 1;           // 1x100x2
    int dis_pred_stride_8_pos = 0;             // 1x1600x32
    int dis_pred_stride_16_pos = 2;          // 1x400x32
    int dis_pred_stride_32_pos = 5;          // 1x100x32

    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    std::unique_ptr<tflite::ExternalCpuBackendContext> external_context_;
    DelegateProviders delegate_providers;
    TfLiteDelegate *delegate_;
    TfLiteContext *context_;
    Settings tflite_setting;

    int input, input_batch, input_height, input_width, input_channels;

    int cls_pred_stride_8_batch, cls_pred_stride_8_channels, cls_pred_stride_8_score;
    int cls_pred_stride_16_batch, cls_pred_stride_16_channels, cls_pred_stride_16_score;
    int cls_pred_stride_32_batch, cls_pred_stride_32_channels, cls_pred_stride_32_score;

    int dis_pred_stride_8_batch, dis_pred_stride_8_channels, dis_pred_stride_8_box;
    int dis_pred_stride_16_batch, dis_pred_stride_16_channels, dis_pred_stride_16_box;
    int dis_pred_stride_32_batch, dis_pred_stride_32_channels, dis_pred_stride_32_box;

    float *input_data;
    float *cls_pred_stride_8_data;
    float *cls_pred_stride_16_data;
    float *cls_pred_stride_32_data;

    float *dis_pred_stride_8_data;
    float *dis_pred_stride_16_data;
    float *dis_pred_stride_32_data;

    cv::Mat icon_head0, icon_head1;
    cv::Mat icon_smoke0, icon_smoke1;
    cv::Mat icon_phone0, icon_phone1;
    cv::Mat icon_sleep0, icon_sleep1;
    cv::Mat icon_head0_alpha, icon_head1_alpha;
    cv::Mat icon_smoke0_alpha, icon_smoke1_alpha;
    cv::Mat icon_phone0_alpha, icon_phone1_alpha;
    cv::Mat icon_sleep0_alpha, icon_sleep1_alpha;

    cv::Mat icon_left0, icon_left1;
    cv::Mat icon_right0, icon_right1;
    cv::Mat icon_up0, icon_up1;
    cv::Mat icon_down0, icon_down1;
    cv::Mat icon_left0_alpha, icon_left1_alpha;
    cv::Mat icon_right0_alpha, icon_right1_alpha;
    cv::Mat icon_up0_alpha, icon_up1_alpha;
    cv::Mat icon_down0_alpha, icon_down1_alpha;

    cv::Mat icon_driver_seatbelt0, icon_driver_seatbelt1;
    cv::Mat icon_driver_seatbelt0_alpha, icon_driver_seatbelt1_alpha;
    cv::Mat icon_copilot_seatbelt0, icon_copilot_seatbelt1;
    cv::Mat icon_copilot_seatbelt0_alpha, icon_copilot_seatbelt1_alpha;
};

class KalmHeading
{
	public:
		KalmHeading();
		~KalmHeading();
        void KalmReset();
		void KalmInit(float x, float y);        //完成对x(0)的初始化
		std::tuple<float, float> kalmPredict(float x, float y);
		std::tuple<float, float> run(float center_x, float center_y);

		//global v
		std::vector<float> offHeading;

	private:
		cv::KalmanFilter kf;
		cv::Mat measurement;
		std::vector<float> d_history;

};

template <typename _Tp>
int activation_function_softmax(const _Tp *src, _Tp *dst, int length);
inline float fast_exp(float x);
inline float sigmoid(float x);
double get_us(struct timeval t);
int vector_mode(std::vector<int>input_vector);
bool dirExists(const std::string &path);
std::tuple<float, float, float> get_pitch_yaw_roll(std::vector<cv::Point2d> landmark_points, BoxInfo face);
float getAngelOfTwoVector(cv::Point2d &pt1, cv::Point2d &pt2, cv::Point2d&c);
void calculateFaceAngles(double pitch, double yaw, double roll);
#endif