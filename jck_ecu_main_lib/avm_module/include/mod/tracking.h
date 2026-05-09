#pragma once
#ifndef TRACKING_H
#define TRACKING_H

#include "autosys_tracker.h"

extern void mod_tracking(mod_rect_int_t* bdInfo, int groups, std::vector<tracking_box_t_mod>& trackingResult);

#endif