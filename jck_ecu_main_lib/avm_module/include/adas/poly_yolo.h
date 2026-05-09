#pragma once
#ifndef POLY_YOLO_H_
#define POLY_YOLO_H_
#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/tools/command_line_flags.h"
#include "tensorflow/lite/tools/delegates/delegate_provider.h"

#include <sys/time.h>
#include "tfdevice.h"

#include <getopt.h>     // NOLINT(build/include_order)
#include "absl/memory/memory.h"
#include "tensorflow/lite/examples/label_image/log.h"
#include "tensorflow/lite/profiling/profiler.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/kernels/cpu_backend_context.h"
#include "tensorflow/lite/kernels/register.h"

#include "omp.h"

#define INPUT_IMAGE_W 1920//1280
#define INPUT_IMAGE_H 1080//720

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

typedef struct Laneline_params_s
{
    float lane_params[4];
    float upper;
    float lower;
    float prob;
} Laneline_params_t;

std::vector<double> linspace(float start_in, float end_in, int num_in);
double get_us(struct timeval t);

class POLY_YOLO {
public:

    void init(const char* model_file);
	~POLY_YOLO();
    void data_memset();
    void preprocess(cv::Mat &img);
    void Invoke();
    void postprocess(cv::Mat &img, std::vector<Object> &objects, std::vector<cv::Point> &lane_pts);
    void postprocess_LD(std::vector<Object> &objects, std::vector<Laneline_params_t> &lane_params);
    void draw_objects(cv::Mat& bgr, const std::vector<Object>& objects);

    // std::vector<double> linspace(float start_in, float end_in, int num_in);

private:
    void get_input_rgb_data(cv::Mat &sample, float* input_data, int img_h, int img_w, int img_c, const float* mean, const float* scale);
    void get_input_rgb_letterbox_data(cv::Mat &sample, float* input_data, int img_h, int img_w, int img_c, const float* mean, const float* scale);
    void get_input_data_focus(cv::Mat &sample, float* input_data, int letterbox_rows, int letterbox_cols, const float* mean, const float* scale);
    void get_input_data_focus_letterbox(cv::Mat &sample, float* input_data, int letterbox_rows, int letterbox_cols, const float* mean, const float* scale);
    void generate_proposals(int stride, const float* feat, float prob_threshold, std::vector<Object>& objects,
                               int letterbox_cols, int letterbox_rows);
    void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold);
    void qsort_descent_inplace(std::vector<Object>& faceobjects);
    void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right);
    float intersection_area(const Object& a, const Object& b);
    float sigmoid(float x);
    // template<typename T>
    // std::vector<double> linspace(T start_in, T end_in, int num_in);

    // load model
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    std::unique_ptr<tflite::ExternalCpuBackendContext> external_context_;
    DelegateProviders delegate_providers;
    TfLiteDelegate* delegate_;
    TfLiteContext* context_;
    Settings s;
    int input;
    int input_batch;
    int input_height;
    int input_width;
    int input_channels;

    int output_8_batch ;
    int output_8_height;
    int output_8_width;
    int output_8_channels;
    int output_8_channels2 ;

    int output_16_batch ;
    int output_16_height;
    int output_16_width;
    int output_16_channels;
    int output_16_channels2 ;

    int output_32_batch ;
    int output_32_height;
    int output_32_width;
    int output_32_channels;
    int output_32_channels2 ;

    int output_lane_batch ;
    int output_lane_height;
    int output_lane_width;
    int output_lane_channels;

    float* input_data;
    float* p8_data;
    float* p16_data;
    float* p32_data;
    float* lane_data;

    // const float anchors[18] = {5,5, 6,9, 10,7, 9,15, 16,13, 20,22, 15,51, 32,29, 53,60};
    const float anchors[18] = {10,13, 16,30, 33,23, 30,61, 62,45, 59,119, 116,90, 156,198, 373,326};
    
    // int cls_num = 1;
    int cls_num = 3;
    const float mean[3] = {0, 0, 0};
    const float scale[3] = {0.003921, 0.003921, 0.003921};
    // const float scale[3] = { 1, 1, 1};
    float prob_threshold = 0.3;
    float nms_threshold = 0.25;
    // int letterbox_rows = 288;
    // int letterbox_cols = 512;
    int letterbox_rows = 192;
    int letterbox_cols = 320;

    int lane_coeff = 7;
    int max_lane = 2;
};

#endif // !MOBILEFACENET_H_