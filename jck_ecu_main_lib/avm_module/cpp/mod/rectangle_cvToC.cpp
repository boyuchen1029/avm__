
#include "../../system.h"
#include "mod/rectangle_cvToC.h"

#include <cstdlib>
#include <cstdio>
#include <algorithm>

using namespace std;

extern float calculate_iou(mod_rect_int_t r0, mod_rect_int_t r1)
{
	float iou = 0.f;
    float epsilon = 1e-5; //Small value to prevent division by zero

    if (r0.x > r1.x + r1.width) return 0.f;
    if (r0.y > r1.y + r1.height) return 0.f;
    if (r0.x + r0.width < r1.x) return 0.f;
    if (r0.y + r0.height < r1.y) return 0.f;

    // Must Union
    float overlapWidth = min(r0.x + r0.width, r1.x + r1.width) - max(r0.x, r1.x);
    float overlapHeight = min(r0.y + r0.height, r1.y + r1.height) - max(r0.y, r1.y);
    float overlapArea = (float)overlapWidth * overlapHeight;
    // maybe overflow
    return overlapArea / (float)((r0.width * r0.height) + (r1.width * r1.height) - overlapArea + epsilon);
}