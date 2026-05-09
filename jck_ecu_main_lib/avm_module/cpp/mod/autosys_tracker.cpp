// Tracking
#include "../../system.h"
#include "mod/mod_para.h"
#include "mod/autosys_tracker.h"
#include "mod/KalmanTracker.h"
#include "mod/Hungarian_mod.h"
#include "mod/rectangle_cvToC.h"

#define MOD_WINDOWS 0
#define MOD_LINUX 1
#define MOD_SYSTEM MOD_WINDOWS

#if MOD_SYSTEM == MOD_LINUX
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sched.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#endif

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <numeric>
#include <limits>
#include <list>
#include <sstream>
#include <chrono>
#include <thread>
#include <stdbool.h>
#include <time.h>
#include <ctime>
#include <stdlib.h>
#include <errno.h>
#include <fstream>          // for std::ofstream
#include <errno.h>

using namespace std;

// Tracking
std::vector<KalmanTracker_t> trackers_mod;
std::vector<mod_rect_int_t> predictedBoxes_mod;
std::vector<vector<double>> iouMatrix_mod;
std::vector<int> assignment_mod;
std::vector<mod_point_t> matchedPairs_mod;
std::vector<TrackingBox_mod> frameTrackingResult_mod;

static int trkNum_mod = 0;
static int detNum_mod = 0;

static double iouThreshold = 0.1;// MOD
static int minHits_mod = 1;
static int maxHits_mod = CONTINUOUS_DETECTION_FRAMES + 4;
static int punishFrames_mod = 2;
static int idCount_mod = 0;

static int check_cover(mod_rect_int_t boxA, mod_rect_int_t boxB)
{
    int pointA[2][2] = {};
    int pointB[2][2] = {};

    // left and up point
    pointA[0][0] = boxA.x;
    pointA[0][1] = boxA.y;
    // right and down point
    pointA[1][0] = boxA.x + boxA.width;
    pointA[1][1] = boxA.y + boxA.height;

    pointB[0][0] = boxB.x;
    pointB[0][1] = boxB.y;
    pointB[1][0] = boxB.x + boxB.width;
    pointB[1][1] = boxB.y + boxB.height;

    if (pointA[0][0] < pointB[0][0] && pointA[0][1] < pointB[0][1])
    {
        if (pointA[1][0] > pointB[1][0] && pointA[1][1] > pointB[1][1])
        {
            //retrun A > B and all cover
            return 1;
        }
    }
    else if (pointA[0][0] > pointB[0][0] && pointA[0][1] > pointB[0][1])
    {
        if (pointA[1][0] < pointB[1][0] && pointA[1][1] < pointB[1][1])
        {
            //retrun B > A and all cover
            return 2;
        }
    }

    return 0;
}

extern std::vector<TrackingBox_mod> autosys_tracker_func(
    std::vector<TrackingBox_mod> detFrameData,
    int frame_count, int detectionFlage)
{
    // initialization for matches
    for (auto & it : detFrameData)
    {
        it.flagMatched = 0;
    }

    for (auto & it : trackers_mod)
    {
        it.m_time_since_update = 1; // init as unmatched
    }

    ///////////////////////////////////////
    // 2. associate detections to tracked object (both represented as bounding boxes)
    // dets : detFrameData
    trkNum_mod = predictedBoxes_mod.size();
    detNum_mod = detFrameData.size();
    //printf("detNum_mod %d\n", detNum_mod);
    //if (detNum_mod > 0)
    //{
    //    printf("detNum_mod %d, x = %d, y = %d\n", detNum_mod, detFrameData[0].box.x, detFrameData[0].box.y);
    //}
        
    iouMatrix_mod.clear();
    iouMatrix_mod.resize(trkNum_mod, vector<double>(detNum_mod, 0));

    for (unsigned int i = 0; i < trkNum_mod; i++) // compute iou matrix as a distance matrix
    {
        for (unsigned int j = 0; j < detNum_mod; j++)
        {
            // use 1-iou because the hungarian algorithm computes a minimum-cost assignment_mod.
            //float iou = GetIOU(predictedBoxes_mod[i], detFrameData[j].box);
            float iou = calculate_iou(predictedBoxes_mod[i], detFrameData[j].box);
            iouMatrix_mod[i][j] = 1 - iou;
            //printf("trkNum_mod %d, x = %d, y = %d\n", i, predictedBoxes_mod[i].x, predictedBoxes_mod[i].y);
            //printf("detNum_mod %d, x = %d, y = %d\n", j, detFrameData[j].box.x, detFrameData[j].box.y);
            //printf("IOU %.2f%%\n", iou * 100);
        }
    }

    // solve the assignment_mod problem using hungarian algorithm.
    // the resulting assignment_mod is [track(prediction) : detection], with len=preNum
    HungarianAlgorithm_mod HungAlgo;
    assignment_mod.clear();
    HungAlgo.Solve_mod(iouMatrix_mod, assignment_mod);

    // filter out matched with low IOU
    matchedPairs_mod.clear();
    for (int i = 0; i < trkNum_mod; ++i)
    {
        if (assignment_mod[i] == -1) // pass over invalid values
        {
            continue;
        }

        if (1 - iouMatrix_mod[i][assignment_mod[i]] < iouThreshold)
        {
            //unmatchedTrajectories.insert(i);
            //unmatchedDetections.insert(assignment_mod[i]);
        }
        else
        {
            //matchedPairs_mod.push_back(cv::Point(i, assignment_mod[i]));
            mod_point_t match = {i, assignment_mod[i] };
            matchedPairs_mod.push_back(match);
            //printf("assignment_mod[%d] = %d\n", i, assignment_mod[i]);
        }
    }

    ///////////////////////////////////////
    // 3. updating trackers_mod

    // update matched trackers_mod with assigned detections.
    // each prediction is corresponding to a tracker
    int detIdx, trkIdx;
    for (unsigned int i = 0; i < matchedPairs_mod.size(); i++)
    {
        trkIdx = matchedPairs_mod[i].x;
        detIdx = matchedPairs_mod[i].y;

        // check cover
        int a = check_cover(detFrameData[detIdx].box, trackers_mod[trkIdx].matchedBox);
        if (a == 1)
        {
            // printf("det cover trackers_mod: det.box = %d, %d, %d,%d\n",
            //     detFrameData[detIdx].box.x, detFrameData[detIdx].box.y,
            //     detFrameData[detIdx].box.width, detFrameData[detIdx].box.height);

            // printf("trackers_mod.box = %d, %d, %d,%d\n",
            //     trackers_mod[trkIdx].matchedBox.x, trackers_mod[trkIdx].matchedBox.y,
            //     trackers_mod[trkIdx].matchedBox.width, trackers_mod[trkIdx].matchedBox.height);
        }
        else if (a == 2)
        {
            // printf("trackers_mod cover det: trackers_mod.box = %d, %d, %d,%d\n",
            //     trackers_mod[trkIdx].matchedBox.x, trackers_mod[trkIdx].matchedBox.y,
            //     trackers_mod[trkIdx].matchedBox.width, trackers_mod[trkIdx].matchedBox.height);

            // printf("trackers_mod: det.box = %d, %d, %d,%d\n",
            //     detFrameData[detIdx].box.x, detFrameData[detIdx].box.y,
            //     detFrameData[detIdx].box.width, detFrameData[detIdx].box.height);
        }


        detFrameData[detIdx].flagMatched = 1;
        trackers_mod[trkIdx].m_time_since_update = 0;
        trackers_mod[trkIdx].m_hit_streak ++;
        trackers_mod[trkIdx].matchedBox = detFrameData[detIdx].box;
        if (trackers_mod[trkIdx].m_hit_streak > maxHits_mod)
        {
            trackers_mod[trkIdx].m_hit_streak = maxHits_mod;
        }

        //printf("update detIdx = %d\n", detIdx);
        //printf("update to trkIdx = %d\n", trkIdx);
        //printf("update det (%d,%d)\n", detFrameData[detIdx].box.x, detFrameData[detIdx].box.y);
    }

    // minus m_hit_streak number for unmatched trackers_mod
    for (auto & it : trackers_mod)
    {
        if (it.m_time_since_update == 1)
        {
            it.m_hit_streak -= punishFrames_mod; // init as unmatched
        }
    }

    // create and initialise new trackers_mod for unmatched detections
    for (auto & it : detFrameData)
    {
        if (it.flagMatched == 0)
        {
            //printf("unmatched x = %d, y = %d\n", it.box.x, it.box.y);
            KalmanTracker_t tkTmp = {};
            tkTmp.matchedBox = it.box;
            tkTmp.m_time_since_update = 0;
            tkTmp.m_hit_streak = 0;
            tkTmp.m_id = 0;
            trackers_mod.push_back(tkTmp);
        }
    }

    // get trackers_mod' output
    frameTrackingResult_mod.clear();
    for (auto it = trackers_mod.begin(); it != trackers_mod.end();)
    {
        if ((*it).m_hit_streak < 0)
        {
            //printf("erase id = %d\n", (*it).m_id);
            it = trackers_mod.erase(it);
            if (it == trackers_mod.end())
            {
                break;
            }
            else
            {
                continue;
            }
        }

        //puts("");
        //printf("m_id = %d\n", (*it).m_id);
        //printf("m_time_since_update %d\n", (*it).m_time_since_update);
        //printf("m_hit_streak %d\n", (*it).m_hit_streak);
        if (((*it).m_time_since_update == 0) && (*it).m_hit_streak >= minHits_mod)
		{
            TrackingBox_mod res = {};
            res.box = (*it).matchedBox;
			res.frame = frame_count;
            res.class_id = (*it).class_id; //
            res.hitNum = (*it).m_hit_streak;
            // res.area = (*it).area; // un

            if ((*it).m_id == 0)
            {
                idCount_mod++;
                res.id = idCount_mod;
                (*it).m_id = idCount_mod;
            }
            else
            {
                res.id = (*it).m_id;
            }

			frameTrackingResult_mod.push_back(res);
         /*   printf("res.id %d\n", res.id);
            printf("res.box x %d\n", res.box.x);
            printf("res.box y %d\n", res.box.y);*/
		}
            
        it++;
    }

    predictedBoxes_mod.clear();
    if (trackers_mod.size() >= 1)
    {
        for (auto& it : trackers_mod)
        {
            //printf("trackers_mod size = %llu\n", trackers_mod.size());
            //printf("push trackers_mod id = %d\n", it.m_id);
            //printf("push trackers_mod x = %f\n", it.matchedBox.x);
            //printf("push trackers_mod y = %f\n", it.matchedBox.y);
            //printf("push trackers_mod w = %f\n", it.matchedBox.width);
            //printf("push trackers_mod h = %f\n", it.matchedBox.height);
            predictedBoxes_mod.push_back(it.matchedBox);
        }
    }

    return frameTrackingResult_mod;
}