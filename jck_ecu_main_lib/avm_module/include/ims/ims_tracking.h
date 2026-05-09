#ifndef IMS_TRACKING_H
#define IMS_TRACKING_H 2

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

#define StateType Rect_<float>

typedef struct IMSTrackingBox
{
    int frame;
    int id;
    cv::Rect_<float> box;
}IMSTrackingBox;

class HungarianAlgorithm
{
public:
	HungarianAlgorithm();
	~HungarianAlgorithm();
	double Solve(std::vector<std::vector<double>>& DistMatrix, std::vector<int>& Assignment);

private:
	void assignmentoptimal(int *assignment, double *cost, double *distMatrix, int nOfRows, int nOfColumns);
	void buildassignmentvector(int *assignment, bool *starMatrix, int nOfRows, int nOfColumns);
	void computeassignmentcost(int *assignment, double *cost, double *distMatrix, int nOfRows);
	void step2a(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
	void step2b(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
	void step3(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
	void step4(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim, int row, int col);
	void step5(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
};

class IMSKalmanTracker
{
public:
	IMSKalmanTracker()
	{
		init_kf(cv::StateType());
		m_time_since_update = 0;
		m_hits = 0;
		m_hit_streak = 0;
		m_age = 0;
		m_id = kf_count;
		//kf_count++;
	}
	IMSKalmanTracker(cv::StateType initRect)
	{
		init_kf(initRect);
		m_time_since_update = 0;
		m_hits = 0;
		m_hit_streak = 0;
		m_age = 0;
		m_id = kf_count;
		kf_count++;
	}

	~IMSKalmanTracker()
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

private:
	void init_kf(cv::StateType stateMat);
	cv::KalmanFilter kf;
	cv::Mat measurement;

	std::vector<cv::StateType> m_history;
};

class IMS_tracking
{
public:
    bool track_state = false;
    int max_age = 10;
    int min_hits = 1;
    double iouThreshold = 0.2;

    std::vector<IMSTrackingBox> detData;
    std::vector<IMSTrackingBox> frameTrackingResult;

    void sort_DataReload(int x1, int y1, int x2, int y2, int i);
    std::vector<IMSTrackingBox> sort();

	// global variables for counting
	int total_frames = 0;
	double total_time = 0.0;
	int frame_count = 0;
	std::vector<IMSKalmanTracker> trackers;

	// variables used in the for-loop
	std::vector<cv::Rect_<float>> predictedBoxes;
	std::vector<std::vector<double>> iouMatrix;
	std::vector<int> assignment;
	std::set<int> unmatchedDetections;
	std::set<int> unmatchedTrajectories;
	std::set<int> allItems;
	std::set<int> matchedItems;
	std::vector<cv::Point> matchedPairs;

	unsigned int trkNum = 0;
	unsigned int detNum = 0;
private:

};

#endif