#ifndef IMS_H_
#define IMS_H_
#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include "opencv2/opencv.hpp"
#include <unistd.h>
#include "ims_tflite.h"

using namespace std;

cv::Mat ims(cv::Mat img, IMS_TFLITE &ims_tflite, bool draw_bbox_flag);

#endif