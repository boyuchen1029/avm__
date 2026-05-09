#ifndef IMS_PFLD_H_
#define IMS_PFLD_H_

#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <exception>

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

#include <getopt.h> // NOLINT(build/include_order)
#include "absl/memory/memory.h"
#include "tensorflow/lite/examples/label_image/log.h"
#include "tensorflow/lite/profiling/profiler.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/kernels/cpu_backend_context.h"
#include "tensorflow/lite/kernels/register.h"

#include "omp.h"

class PFLD_TFLITE
{
public:
    void init(const char *model_file);
    ~PFLD_TFLITE();
    void data_memset();
    void preprocess(cv::Mat &src, cv::Point pt1, cv::Point pt2, cv::Point &s_point, cv::Size &croped_wh);
    void Invoke();
    void postprocess(cv::Mat &img, cv::Size &croped_wh, cv::Point &s_point, std::vector<cv::Point2d> &results, bool draw_bbox);
    void face_pose_estimation(std::vector<cv::Point2d> &face_kpt_results, bool &head_turn, bool &eyes_wink,  cv::Rect &bbox_top, bool phone_flag, bool smoke_flag);
    float factor = 1.f;
    float left_head_ratio_thre = 7.0;
    float right_head_ratio_thre = 0.6;
    float up_head_ratio_thre = 0.6;
    float down_head_ratio_thre = 1.6;
    float left_wink_ratio_thre = 0.18;
    float right_wink_ratio_thre = 0.19;
private:
    int num_thread = 1;
    int face_kpt_num = 28;
    const float mean[3] = {123.f, 123.f, 123.f};
    const float scale[3] = {0.017241, 0.017241, 0.017241};

    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    std::unique_ptr<tflite::ExternalCpuBackendContext> external_context_;
    DelegateProviders delegate_providers;
    TfLiteDelegate *delegate_;
    TfLiteContext *context_;
    Settings tflite_setting;

    int input, input_batch, input_height, input_width, input_channels;
    int output_batch, output_height, output_width, output_channels;
    float *input_data;
    float *pred_data;
};
#endif