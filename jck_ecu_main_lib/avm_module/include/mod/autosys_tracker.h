#ifndef AUTOSYS_TRACKER_H
#define AUTOSYS_TRACKER_H

#include "rectangle_cvToC.h"

#include <vector>
using namespace std;

typedef struct TrackingBox_mod
{
    int frame;
    int id;
    int class_id;
    mod_rect_int_t box;

    unsigned char flagHit;
    int hitNum;

    unsigned char flagMatched;
}tracking_box_t_mod;

extern std::vector<TrackingBox_mod> autosys_tracker_func(
    std::vector<TrackingBox_mod> detFrameData,
    int frame_count, int detectionFlage);

#endif // AUTOSYS_TRACKER_H