#pragma once
#ifndef ADAS_MAIN_H_
#define ADAS_MAIN_H_

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "poly_yolo.h"
#include "adas_solution.h"
#include "LaneSortTracking.h"

typedef struct laneline_s{
    int pt1_x;
    int pt1_y;
    int pt2_x;
    int pt2_y;
} laneline_t;

typedef struct adas_output_param_s{
    int BSDalert[2];
    int ODAalert[2];
    int MODalert[4];
    int LDWalert[2];
    std::vector<TrackingBox> objs_tracking;
    std::vector<laneline_t> lanes_tracking;
} adas_output_param_t;

struct adas_function_roi{
    std::vector<cv::Point> rear_15_roi;
    std::vector<cv::Point> rear_10_roi;
    std::vector<cv::Point> rear_5_roi;
    std::vector<cv::Point> rear_3_roi;

    std::vector<cv::Point> front_3_roi;
    std::vector<cv::Point> right_3_roi;
    std::vector<cv::Point> left_3_roi;

    std::vector<std::vector<cv::Point2f>> adasLUT_rear; //LUT from bin: (720, 1280)
    std::vector<std::vector<cv::Point2f>> adasLUT_front;
    std::vector<std::vector<cv::Point2f>> adasLUT_left;
    std::vector<std::vector<cv::Point2f>> adasLUT_right;
    bool LUT_trigger; /*0: use defult roi by corners, 1: use loo-up table from avm*/
};

typedef struct car_param_info_s{
		float carW;
		float carL;
		cv::Point2f lf_headlight;
		cv::Point2f rf_headlight;
}car_param_info_t;

typedef struct adas_function_trigger_s{
    int MOD_trigger;
    int BSD_trigger;
    int ODA_trigger;
    int LDW_trigger;
} adas_function_trigger_t;

typedef struct car_status_s{
    std::atomic<int> car_speed_info;
    car_param_info_t car_LR_infos;
    adas_function_trigger_t adas_func_triggers;
    std::atomic<float> MODmotionThreshold;
    float MODrangeExtend[2];
} car_status_t;

typedef struct adas_input_param_s{
    const char *model_path;
    car_status_t car_info;
    adas_function_roi adas_func_rois;
    std::atomic<unsigned int> adas_progress_counter;
} adas_input_param_t;

typedef struct AI_arg_s{
   POLY_YOLO poly_yolo;
    cv::Mat ADAS_src_img;
} AI_arg_t;

class AutoSysADAD{
public :
    AutoSysADAD(){

    }
    void AutoSysADASsolutionInit();
    void AutoSysADASsolutionRuntime(adas_output_param_t *adas_outputs);
    void AutoSysADASsolutionClose();
    adas_input_param_t adas_inputs;
    AI_arg_t AI_args;

private:
    
    LANESORTTRACKING lanesorttracking;
    LaneDetector lanedetector;
    ObjectDetector blindspotdetection;

    std::vector<Object> det_obj_roi;
    std::vector<TrackingLane> LaneTrackingResult;

};

// #ifdef __cplusplus
// }
// #endif

#endif // !ADAS_MAIN_H_