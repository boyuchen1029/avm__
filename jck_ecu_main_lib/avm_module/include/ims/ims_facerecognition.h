#ifndef IMS_FACERECOGNITION_H_
#define IMS_FACERECOGNITION_H_

#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <exception>
#include <dirent.h>

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

typedef struct userinfo
{
    std::vector<std::vector<float>> gradient_feature;
    std::string user_name;
} userinfo;

class FACERECOGNITION_TFLITE
{
public:
   std::string user_information_dir_path = "./User-information/";
    std::vector<userinfo> total_userinfo;
    std::string final_user_name;
    int database_idx = 0;
    bool user_name_prepare = false;

    void init(const char *model_file);
    ~FACERECOGNITION_TFLITE();
    void data_memset();
    void preprocess(cv::Mat &src, cv::Point pt1, cv::Point pt2, cv::Point &s_point, cv::Size &croped_wh);
    void Invoke();
    void postprocess(std::vector<float> &results);
    void calculSimilar(std::vector<float>& results);
    int readFileList(std::string basePath,std::vector<std::string> &files);
    int readDirList(std::string basePath,std::vector<std::string> &dirs, std::vector<std::string> &names);
    float factor = 1.f;
    int dir_file_num = 0;
private:
    int num_thread = 1;
    float threshold = 0.65;

    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    std::unique_ptr<tflite::ExternalCpuBackendContext> external_context_;
    DelegateProviders delegate_providers;
    TfLiteDelegate *delegate_;
    TfLiteContext *context_;
    Settings tflite_setting;

    int input, input_batch, input_height, input_width, input_channels;
    int output_batch, output_channels;
    float *input_data;
    float *pred_data;
};

#endif