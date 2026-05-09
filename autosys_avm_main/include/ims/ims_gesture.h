#ifndef IMS_GESTURE_H_
#define IMS_GESTURE_H_

#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <cmath>
#include <tuple>
#include <opencv2/opencv.hpp>

class IMS_GESTURE
{
    public:
        int vector_2d_angle(const std::vector<cv::Point2d> &vector);
        std::vector<int> hand_angle(const std::vector<cv::Point2f> &landmarks);
        std::string hand_pos(std::vector<int> finger_angle);
};

#endif