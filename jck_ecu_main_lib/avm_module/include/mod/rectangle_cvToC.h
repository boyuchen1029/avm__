#pragma once
#ifndef RECTANGLE_CVTOC_H
#define RECTANGLE_CVTOC_H

typedef struct
{
	int x;
	int y;
}mod_point_t;

typedef struct
{
	int x;
	int y;
	int width;
	int height;
}mod_rect_int_t;

extern float calculate_iou(mod_rect_int_t r0, mod_rect_int_t r1);

#endif