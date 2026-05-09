#pragma once
#ifndef LANESORTTRACKING_H_
#define LANESORTTRACKING_H_

#include "opencv2/opencv.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <iomanip> // to format image names using setw() and setfill()
//#include <io.h> // to check file existence using POSIX function access(). On Linux include <unistd.h>.
#include <unistd.h>
#include <set>

#include "poly_yolo.h"
#include "Hungarian.h"
#include "lanekalman.h"

typedef struct TrackingLane
{
	int frame;
	int id;
	float a;
	float b;
	float c;
	float d;
	std::vector<cv::Point> pts;
	float lower;
	float upper;

}TrackingLane;

class LANESORTTRACKING {
public:

    // global variables for counting
    #define CNUM 20

    LANESORTTRACKING();
    ~LANESORTTRACKING();
    std::vector<TrackingLane> TrackingResult(std::vector<Laneline_params_t> &lanes, cv::Mat result_img);
    cv::Mat DarwTrackingResult(cv::Mat& image, std::vector<TrackingLane> frameTrackingResult);
    std::vector<LaneKalmanTracker> trackers;
private:

    int frame_count = 0;
	int max_age = 30;
	int min_hits = 2;
	double iouThreshold = 0.3;
	
	// variables used in the for-loop
	std::vector<std::vector<cv::Point>> predictedLanes;
	std::vector<std::vector<double>> iouMatrix;
	std::vector<int> assignment;
	std::set<int> unmatchedDetections;
	std::set<int> unmatchedTrajectories;
	std::set<int> allItems;
	std::set<int> matchedItems;
	std::vector<cv::Point> matchedPairs;
	std::vector<TrackingLane> frameTrackingResult;
	unsigned int trkNum = 0;
	unsigned int detNum = 0;
    std::vector<TrackingLane> detData;
    double GetIOU(cv::Rect_<float> bb_test, cv::Rect_<float> bb_gt);
	std::vector<cv::Point> Get_Lane_pts(float a, float b, float c, float d);
	std::vector<cv::Point> Get_Lane_pts_new(Laneline_params_t lane_param);
	double Get_Euclident_Distance(std::vector<cv::Point> pts_test, std::vector<cv::Point> pts_gt);

	float x_value(float y, float m, float b);
	float m_value(float start_x, float start_y, float end_x, float end_y);
	float b_value(float start_x, float start_y, float end_x, float end_y);
	std::vector<std::vector<cv::Point>> main_lane(std::vector<std::vector<cv::Point>> lane_pts);
};

#endif // !MOBILEFACENET_H_