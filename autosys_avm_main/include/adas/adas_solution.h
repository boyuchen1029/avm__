#pragma once
#ifndef ADAS_SOLUTION_H_
#define ADAS_SOLUTION_H_
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

#include "omp.h"
#include "poly_yolo.h"
#include <deque>
#include <numeric>
#include "obj_tracking.h"
#include "LaneSortTracking.h"

struct adas_function_roi;

typedef struct objInfo_s
{
	int id;
    int forwardCount;
    int alert;
    double distance;
    double distanceWidth;
    double motionSum;
    double motionSumW;
    double kalman_p_last_N;
    double kalman_x_last_N; 
    double preLocal;
    double preLocalWidth;
    float confidAvg;
    int liveTime;
    int firstLocaldis;
    int firstLocalwid;

    float motionRecord[31];
    float distanceRecord[31];

    int LR; // BSD: left 1  right 2, MOD: front 1 back 2 left 3 right 4
    int coordx;
    int coordy;
    int boxWidth;
    int boxHeight;
    int turnCount;
} objInfo_t;

bool lane_compare(const Laneline_params_t lane_1, const Laneline_params_t lane_2);

class LaneDetector {
public:
    void lane_detect(std::vector<Laneline_params_t> lane_params, std::vector<cv::Point> car_status, int * LDWS_FLAG);
    void lane_detect(std::vector<TrackingLane> &lane_params, const std::vector<cv::Point> &car_status, int * LDWS_FLAG);

    std::deque<int> ldws_flag_left;
    std::deque<int> ldws_flag_right;
    int lane_alarmExtender[2];

    std::vector<cv::Point> car_stat;

private:
    float detect_line_position_RL(Laneline_params_t lane_func, cv::Point car_point);
    float detect_line_position_RL(const TrackingLane lane_func, const cv::Point car_point);
    
    
};

class ObjectDetector{
    public:
        double CalculateIOU(const cv::Rect_<float> bb_test, const cv::Rect_<float> bb_gt);
        void box_scaling(Object &box);
        void box_scaling(TrackingBox &box);
        void roi_detect(std::vector<Object> detections, std::vector<Object> &detections_roi, adas_function_roi adas_rois);
        void object_jitter_filter(Object det, std::vector<Object> &detections_roi, std::vector<Object> &detection_previous, bool no_obj_last_frame, float jitter_threshold_upper, float jitter_threshold_lower, float jitter_threshold_iou);
        void object_track(std::vector<Object> &detections, std::vector<TrackingBox> &tracking_results);
        double KalmanF(const double ResrcData, double ProcessNiose_Q, double MeasureNoise_R, objInfo_t obj);
        void alarmDecision_BSD(int alertFlag[], std::vector<TrackingBox>* trackingResult, const adas_function_roi &adas_rois, int car_speed_info);
        void alarmDecision_MOD(int alertFlag[], std::vector<TrackingBox> &trackingResult, const adas_function_roi &adas_rois, int car_speed_info, float motionThreshold, float carL, float carW, float rangeExtend[2]);
        void alarmDecision_BSD_pixel(int alertFlag[], std::vector<TrackingBox>* trackingResult, const adas_function_roi &adas_rois, int car_speed_info);
        void alarmDecision_MOD_pixel(int alertFlag[], std::vector<TrackingBox> &trackingResult, const adas_function_roi &adas_rois, int car_speed_info, float motionThreshold, float carL);

        IMS_Tracking ims_tracking;
        std::vector<objInfo_s> objs;
        std::vector<objInfo_s> objs_MOD;
        std::vector<TrackingBox> tracking_results;

        std::vector<Object> detection_draw;
        int motionCount[8] = {0};

        // const vector<cv::Point> right_roi = {cv::Point(961, 1079), cv::Point(961, 951), cv::Point(1067, 817), cv::Point(1141, 752), cv::Point(1351, 724), \
        //                                                                         cv::Point(1486, 724), cv::Point(1628, 736), cv::Point(1711, 748), cv::Point(1755, 757),cv::Point(1814, 809), \
        //                                                                         cv::Point(1919, 952), cv::Point(1919, 1079)};
        // const vector<cv::Point> left_roi = {cv::Point(0, 1079), cv::Point(59, 939), cv::Point(115, 859), cv::Point(163, 809), cv::Point(201, 796), \
        //                                                                         cv::Point(299, 770), cv::Point(379, 751), cv::Point(526, 727), cv::Point(757, 717),cv::Point(850, 773), \
        //                                                                         cv::Point(960, 883), cv::Point(960, 1079)};
        // const vector<cv::Point> front_15_roi = {cv::Point(0, 540), cv::Point(0, 370), cv::Point(204, 245), cv::Point(367, 190), cv::Point(392, 189), \
        //                                                                                 cv::Point(410, 188), cv::Point(425, 188), cv::Point(447, 187), cv::Point(470, 186),cv::Point(492, 186), \
        //                                                                                 cv::Point(549, 186), cv::Point(652, 202), cv::Point(960, 327), cv::Point(960, 540)};
        // const vector<cv::Point> front_3_roi = {cv::Point(0, 540), cv::Point(0, 370), cv::Point(204, 245), cv::Point(295, 237), cv::Point(464, 229), \
        //                                                                                 cv::Point(651, 231), cv::Point(732, 232), cv::Point(960, 327), cv::Point(960, 540)};
        // const vector<cv::Point> back_15_roi = {cv::Point(961, 540), cv::Point(961, 354), cv::Point(1115, 217), cv::Point(1199, 146), cv::Point(1262, 105), \
        //                                                                                 cv::Point(1344, 68), cv::Point(1379, 56), cv::Point(1399, 54), cv::Point(1411, 53), cv::Point(1428, 52), \
        //                                                                                 cv::Point(1476, 52), cv::Point(1504, 53), cv::Point(1524, 54), cv::Point(1550, 59), cv::Point(1593, 75), \
        //                                                                                 cv::Point(1678, 120), cv::Point(1746, 172), cv::Point(1813, 245), cv::Point(1919, 366), cv::Point(1919, 540)};
        // const vector<cv::Point> back_10_roi = {cv::Point(961, 540), cv::Point(961, 354), cv::Point(1115, 217), cv::Point(1199, 146), cv::Point(1262, 105), \
        //                                                                                 cv::Point(1344, 68), cv::Point(1391, 62), cv::Point(1443, 60), cv::Point(1468, 61), cv::Point(1497, 61), \
        //                                                                                 cv::Point(1553, 68), cv::Point(1592, 72), cv::Point(1678, 120), cv::Point(1746, 172), cv::Point(1813, 245), \
        //                                                                                 cv::Point(1919, 366), cv::Point(1919, 540)};
        // const vector<cv::Point> back_5_roi = {cv::Point(961, 540), cv::Point(961, 354), cv::Point(1115, 217), cv::Point(1199, 146), cv::Point(1262, 105), \
        //                                                                                 cv::Point(1331, 92), cv::Point(1425, 85), cv::Point(1477, 84), cv::Point(1525, 88), cv::Point(1620, 103), \
        //                                                                                 cv::Point(1678, 120), cv::Point(1746, 172), cv::Point(1813, 245), cv::Point(1919, 366), cv::Point(1919, 540)};
        // const vector<cv::Point> back_3_roi = {cv::Point(961, 540), cv::Point(961, 354), cv::Point(1115, 217), cv::Point(1199, 146), cv::Point(1281, 126), \
        //                                                                                 cv::Point(1410, 112), cv::Point(1486, 114), cv::Point(1559, 121), cv::Point(1682, 147), cv::Point(1746, 172), \
        //                                                                                 cv::Point(1813, 245), cv::Point(1919, 366), cv::Point(1919, 540)};
    private:
        const cv::Point front_center = cv::Point(479, 540);
        const cv::Point back_center = cv::Point(1439, 540);
        const cv::Point right_center = cv::Point(1439, 1079);
        const cv::Point left_center = cv::Point(479, 1079);
};

#endif //ADAS_SOLUTION_H_