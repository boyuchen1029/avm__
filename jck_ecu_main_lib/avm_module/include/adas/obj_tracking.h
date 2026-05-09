#ifndef OBJ_TRACKING_H
#define OBJ_TRACKING_H 2

#include <iostream>
#include <fstream>
#include <iomanip> 
#include <unistd.h>
#include <set>
#include <numeric>
#include <vector>
#include <float.h>
#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <cmath>

#include "Hungarian.h"

#define StateType Rect_<float>

typedef struct TrackingBox
{
    int frame;
    int id;
	int class_id;
    cv::Rect_<float> box;
}TrackingBox;

class IMS_Tracking
{
public:
    bool track_state = false;
    int max_age = 20;
    int min_hits = 2;
    double iouThreshold = 0.3;

    std::vector<TrackingBox> detData;
    std::vector<TrackingBox> frameTrackingResult;

    void sort_DataReload(int x1, int y1, int x2, int y2, int i, int class_id);
    std::vector<TrackingBox> sort();
private:

};

class KalmanTracker
{
public:
	KalmanTracker()
	{
		init_kf(cv::StateType());
		m_time_since_update = 0;
		m_hits = 0;                //successful hits
		m_hit_streak = 0;  //consecutive successful hits
		m_age = 0;
		m_id = kf_count;
		//kf_count++;
	}
	KalmanTracker(cv::StateType initRect, int class_id)
	{
		init_kf(initRect);
		m_time_since_update = 0;
		m_hits = 0;
		m_hit_streak = 0;
		m_age = 0;
		m_id = kf_count;
		kf_count++;
		this->class_id = class_id;
	}

	~KalmanTracker()
	{
		m_history.clear();
	}

	cv::StateType predict();
	void update(cv::StateType stateMat);
	
	cv::StateType get_state();
	cv::StateType get_rect_xysr(float cx, float cy, float s, float r);

	static int kf_count;

	int m_time_since_update;
	int m_hits;
	int m_hit_streak;
	int m_age;
	int m_id;

	int class_id; //info need to be carried

private:
	void init_kf(cv::StateType stateMat);

	cv::KalmanFilter kf;
	cv::Mat measurement;

	std::vector<cv::StateType> m_history;
};

#endif