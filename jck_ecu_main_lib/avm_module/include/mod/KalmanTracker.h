///////////////////////////////////////////////////////////////////////////////
// KalmanTracker.h: KalmanTracker Class Declaration

#ifndef KALMAN_H
#define KALMAN_H

#include "rectangle_cvToC.h"

// This class represents the internel state of individual tracked objects observed as bounding box.
typedef struct 
{
	int m_time_since_update; // = flagMatched
	int m_hit_streak;
	int m_id;
	int class_id; //info need to be carried
	mod_rect_int_t matchedBox;
}KalmanTracker_t;

#endif // KALMAN_H