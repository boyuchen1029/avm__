///////////////////////////////////////////////////////////////////////////////
// LANEKALMANTracker.h: LANEKALMANTracker Class Declaration

#ifndef LANEKALMAN_H
#define LANEKALMAN_H 2

#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <cmath>
#include <iostream>

#include "poly_yolo.h"

#define LaneStateType LaneKalman

struct LaneKalman {
	float a;
	float b;
	float c;
	float d;
	float lower;
	float upper;
};

// This class represents the internel state of individual tracked objects observed as bounding box.
class LaneKalmanTracker
{
public:
	LaneKalmanTracker()
	{
		init_kf(LaneStateType());
		m_time_since_update = 0;
		m_hits = 0;
		m_hit_streak = 0;
		m_age = 0;
		m_class = -1;
		m_id = kf_count;
		//kf_count++;
	}
	LaneKalmanTracker(LaneStateType init, float lower, float upper)
	{
		init_kf(init);
		m_time_since_update = 0;
		m_hits = 0;
		m_hit_streak = 0;
		m_age = 0;
		m_id = kf_count;
		m_class = -1;
		kf_count++;

		this->lower = lower;
		this->upper = upper;
	}

	~LaneKalmanTracker()
	{
		m_history.clear();
	}

	std::vector<cv::Point> predict(float lower, float upper);
	void update(LaneStateType stateMat);
	
	// std::vector<cv::Point> get_state();
	cv::Mat get_state();
	std::vector<cv::Point> get_lane_pts(float a, float b, float c, float d);
	std::vector<cv::Point> get_lane_pts_new(float a, float b, float c, float d, float lower, float upper);

	static int kf_count;

	int m_time_since_update;
	int m_hits;
	int m_hit_streak;
	int m_age;
	int m_id;
	int m_class;

	float lower;//info need to be carried
	float upper;//info need to be carried

private:
	void init_kf(LaneStateType stateMat);

	cv::KalmanFilter kf;
	cv::Mat measurement;

	std::vector<std::vector<cv::Point>> m_history;
};

#endif
