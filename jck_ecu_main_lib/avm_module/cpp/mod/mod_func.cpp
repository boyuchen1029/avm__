#define _CRT_SECURE_NO_WARNINGS
// #define READ_VIDEO
// #define USING_CV
// #define DEBUG_MOD
#define MOD_2

#include "../../system.h"
#include "../../include/avm/fp_source.h"
//#include "./avm/fp_source.h"
#include "../../include/mod/mod_func.h"
#include "../../include/mod/mod_para.h"
#include "../../include/mod/tracking.h"
#include "../../include/mod/autosys_tracker.h"

#include <iterator>
using namespace std;

#ifdef USING_CV
#include <opencv2/core/mat.hpp>
#include "opencv2/opencv.hpp"
using namespace cv;
#endif

std::vector<tracking_box_t_mod> tbHits;

static int frameCount = -1;
static int regionWarning[4] = { 0, 0, 0, 0 };

#if defined(READ_VIDEO)
FILE * simulationLog = NULL;
#endif
static unsigned char colors_draw[][3] = {
	{255, 127, 0},
	{255, 255, 0},
	{0, 255, 0},
	{0, 0, 255},
	{75, 0, 130},
	{148, 0, 211},
	{255, 0, 153 },
	{100, 0, 13},
	{40, 0 , 40},
	{47, 0, 100}
};

static void cv_display_img(int w, int h, uint8 * img, const char * windowName, int channels)
{
#ifdef USING_CV
	Mat cvFrame;
	if (channels == 1)
	{
		cvFrame.create(w, h, CV_8UC1);
	}
	else if (channels == 3)
	{
		cvFrame.create(w, h, CV_8UC3);
	}
	else
	{
		return;
	}
	
	memcpy(cvFrame.data, img, w * h * channels);
	imshow(windowName, cvFrame);
#endif
}

static void gaussLPF(unsigned char* dataOut, unsigned char* dataIn, int w, int h, double gauss[5])
{
	uint8* tmp = (uint8*)calloc(w * h, 1);
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			double sum = 0.0;
			for (int x = -2; x <= 2; x++)
			{
				if ((i + x) < 0 || (i + x) >= w)
				{
					continue;
				}
				sum = sum + dataIn[(i + x) + (j)*w] * gauss[x + 2];
			}
			tmp[i + j * w] = (uint8)sum;
		}
	}

	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			double sum = 0.0;
			for (int y = -2; y <= 2; y++)
			{
				{
					if ((j + y) < 0 || (j + y) >= h)
					{
						continue;
					}
					sum = sum + tmp[(i)+(j + y) * w] * gauss[y + 2];
				}
			}
			dataOut[i + j * w] = (uint8)sum;
		}
	}

	free(tmp);
}

static void gaussLPF_Mask(double* blur, double sigma)
{
	double sum = 0.0;

	blur[0] = 4.0;
	blur[1] = 1.0;
	blur[2] = 0.0;
	blur[3] = 1.0;
	blur[4] = 4.0;

	for (int i = 0; i < 5; i++)
	{
		blur[i] = exp(-blur[i] / (2 * sigma * sigma));
		sum = sum + blur[i];
	}

	for (int i = 0; i < 5; i++)
	{
		blur[i] = blur[i] / sum;
	}
}

static void cvtColor(uint8* gray, uint8* rgb, int w, int h)
{
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int ptrGray = x + y * w;
			int ptrRGB = 3 * ptrGray;

			int y = 306 * (int)rgb[ptrRGB] + 601 * (int)rgb[ptrRGB + 1] + 117 * (int)rgb[ptrRGB + 2];
			gray[ptrGray] = y >> 10;
		}
	}
}

static void convertTo(float64* dataOut, uint8* dataIn, int w, int h)
{
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			int ptr = i + j * w;
			dataOut[ptr] = (float64)dataIn[ptr];
		}
	}
}

static void absdiff(uint8* dataOut, uint8* src1, uint8* src2, int w, int h)
{

	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			int ptr = i + j * w;

			//int diff = (int)src1[ptr] - (int)src2[ptr];
			//dataOut[ptr] = (diff > 255) ? 255 : ((diff < 0) ? 0 : diff);
			//dataOut[ptr] = abs(dataOut[ptr]);

			int diff = abs((int)src1[ptr] - (int)src2[ptr]);
			if (diff > 255)
			{
				diff = 255;
			}
			dataOut[ptr] = (uint8)diff;
		}
	}
}

static void threshold(uint8* dataOut, uint8* dataIn, int thresh, int maxval, int w, int h)
{
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			int ptr = i + j * w;
			if (dataIn[ptr] > thresh)
			{
				dataOut[ptr] = maxval;
			}
			else
			{
				dataOut[ptr] = 0;
			}
		}
	}
}

static void accumulateWeighted(uint8* src, float64* dst, double alpha, int w, int h)
{
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			int ptr = i + j * w;
			dst[ptr] = (1.0 - alpha) * dst[ptr] + alpha * src[ptr];
		}
	}
}

static void img_preservation(uint8* src, uint8* dst, double alpha, int w, int h)
{
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			int ptr = i + j * w;
			dst[ptr] = uint8((1.0 - alpha) * dst[ptr] + alpha * src[ptr]);
		}
	}
}

static void convertScaleAbs(uint8* dataOut, float64* dataIn, int w, int h)
{
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			int ptr = i + j * w;
			if (dataIn[ptr] > 255)
			{
				dataOut[ptr] = 255;
			}
			else if (dataIn[ptr] < 0)
			{
				dataOut[ptr] = 0;
			}
			else
			{
				dataOut[ptr] = (uint8)dataIn[ptr];
			}
		}
	}
}

static void find_extreme_points(short x[], short y[], int area, extreme_point_t * extremePoint)
{
	// init
	extremePoint->up = y[0];
	extremePoint->down = y[0];
	extremePoint->left = x[0];
	extremePoint->right = x[0];

	for (int i = 1; i < area; i++)
	{
		// extreme points: y
		if (y[i] < extremePoint->up)
		{
			extremePoint->up = y[i];
		}
		else
		{
			if (y[i] > extremePoint->down)
			{
				extremePoint->down = y[i];
			}
		}

		// extreme points: x
		if (x[i] < extremePoint->left)
		{
			extremePoint->left = x[i];
		}
		else
		{
			if (x[i] > extremePoint->right)
			{
				extremePoint->right = x[i];
			}
		}
	}
}

static int mark_surrounding_points(
	mod_t* mod_, uint8 * _thresh_, int groupAmount, int startX, int startY, int w, int h)
{
	mod_group_t * group = mod_->group;
	uint8 * dataIn = mod_->imgThresh;

	unsigned int index = 0;
	int flagGPValid = 0;

	mod_->groupX[0] = startX;
	mod_->groupY[0] = startY;

	do
	{
		int x_tmp = mod_->groupX[index];
		int y_tmp = mod_->groupY[index];
		for (int windowY = -1; windowY <= 1; windowY++)
		{
			for (int windowX = -1; windowX <= 1; windowX++)
			{
				if ((windowX == 0 && windowY == 0) || \
					((x_tmp + windowX) >= w || (x_tmp + windowX) < 0 || (y_tmp + windowY) >= h || (y_tmp + windowY) < 0))
					continue;

				unsigned int ptr_ = (x_tmp + windowX) + (y_tmp + windowY) * w;

				if (dataIn[ptr_] == 255 &&	// �P�򹳯����ƭ�
					_thresh_[ptr_] == 127)	// �B�|���Q�O��
				{
					_thresh_[ptr_] = 255;

					mod_->groupX[group[groupAmount].area] = x_tmp + windowX;
					mod_->groupY[group[groupAmount].area] = y_tmp + windowY;

					group[groupAmount].area++;
				}
			}
		}
		index++;
	} while (index < (unsigned int)group[groupAmount].area);

	/* find extreme points */
	// printf("mod_->groupAreaThresh + mod_recall_flag = %d\n",mod_->groupAreaThresh + mod_recall_flag);
	if (mod_->group[groupAmount].area > (mod_->groupAreaThresh + mod_recall_flag) &&
		mod_->group[groupAmount].area < GROUP_AREA_THRESH_MAX)
	{
		flagGPValid = 1;

		find_extreme_points(mod_->groupX, mod_->groupY, group[groupAmount].area, &mod_->extremePoint[groupAmount]);


	}

#if defined(DEBUG_MOD)
	if (mod_->group[groupAmount].area > mod_->groupAreaThresh)
	{
		if (simulationLog)
		{
			fprintf(simulationLog, "\nframeCount: %d\n", frameCount);
			fprintf(simulationLog, "mod_->group[%d].area  = %d\n", groupAmount, mod_->group[groupAmount].area);

			fprintf(simulationLog, "up %d, down %d, left %d, right %d\n",
				mod_->extremePoint[groupAmount].up, mod_->extremePoint[groupAmount].down,
				mod_->extremePoint[groupAmount].left, mod_->extremePoint[groupAmount].right);

			fprintf(simulationLog, "Center of bd box: (%d, %d)\n", 
				(mod_->extremePoint[groupAmount].up + mod_->extremePoint[groupAmount].down) / 2,
				(mod_->extremePoint[groupAmount].left + mod_->extremePoint[groupAmount].down) / 2);
		}
		/*printf("mod_->group[%d].area  = %d\n", groupAmount, mod_->group[groupAmount].area);*/

		int sumX = 0;
		int sumY = 0;
		int luma = 0;
		int sumLumaLast = 0;
		for (int area = 0; area < mod_->group[groupAmount].area; area++)
		{
			int x = mod_->groupX[area];
			int y = mod_->groupY[area];

			// for displaying and debug
			int pos = (y * mod_->frameW + x) * 3;
			*(mod_->input + pos + 0) = 255 - 0 + groupAmount * 20;
			*(mod_->input + pos + 1) = 255 - 0 + groupAmount * 10;
			*(mod_->input + pos + 2) = 255 - 100 + groupAmount * 7;
		}
	}
#endif

	return flagGPValid;
}
uint8* _thresh_ = NULL;
//static int find_groups(GROUP* group, uint8* dataIn, int groupNumMax, int w, int h, extreme_point_t extremePoint[], int exIndex[])
static int find_groups(mod_t * mod_, int w, int h)
{
	uint8 * dataIn = mod_->imgThresh;
	
	int groupAmount = 0;
	
	// if (_thresh_)
	{
		memset(_thresh_, 127, w * h * sizeof(uint8)); // 127 -> ���Q�O��, 255 -> �P�򹳯����ƭ�
	}

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int ptr = x + y * w;
			if (dataIn[ptr] == 255 && _thresh_[ptr] == 127)
			{
				// /*printf("x %d\n", x);*/
				mod_->group[groupAmount].flag = 0;
				mod_->group[groupAmount].area = 1;
				int flagValid = mark_surrounding_points(mod_, _thresh_, groupAmount, x, y, w, h);

				if (flagValid == 1)
				{
					groupAmount++;
				}

				if (groupAmount >= mod_->groupNumMax)
				{
#if defined(READ_VIDEO)
					if (simulationLog)
					{
						fprintf(simulationLog, "\nframeCount: %d\n", frameCount);
						fprintf(simulationLog, "reach groupNumMax\n");
					}
#endif
					goto EXIT;
				}
			}
		}
	}
EXIT:
	// free(_thresh_);

	return groupAmount;
}

static int isPointInPolygon(int polygon[4][2], int point[2])
{
	int _polygon_[4][2] = {};
	memcpy(_polygon_, polygon, sizeof(_polygon_));
	for (int i = 0; i < 4; i++)
	{
		_polygon_[i][0] = _polygon_[i][0] - point[0];
		_polygon_[i][1] = _polygon_[i][1] - point[1];
	}
	int line[2][2];
	int pointInPolygon = 0;
	int crossFlag = 0;
	int cct = 0;

	for (int i = 0; i < 4; i++)
	{
		if (i < 3)
		{
			line[0][0] = _polygon_[i][0];
			line[0][1] = _polygon_[i][1];
			line[1][0] = _polygon_[i + 1][0];
			line[1][1] = _polygon_[i + 1][1];
		}
		else
		{
			line[0][0] = _polygon_[3][0];
			line[0][1] = _polygon_[3][1];
			line[1][0] = _polygon_[0][0];
			line[1][1] = _polygon_[0][1];
		}
		crossFlag = line[1][1] * line[0][1];
		if (crossFlag > 0)
		{
			continue;
		}
		else
		{
			if ((line[0][1] == 0 || line[1][1] == 0) && (line[0][0] == line[1][0]))
			{
				continue;
			}

			double x = line[0][0] - line[0][1] * (line[1][0] - line[0][0]) / (double)(line[1][1] - line[0][1]);

			if (x > 0)
			{
				
				pointInPolygon = !pointInPolygon;

				cct++;
			}
		}
	}
	

	return pointInPolygon;
}

static int find_center(mod_t * mod_, int groupNum, int detectSW[])
{
	extreme_point_t * extremePoint = mod_->extremePoint;
	memset(extremePoint, -1, sizeof(extreme_point_t) * GROUP_NUM_THRESH);

	int validGroups = 0;

	for (int n = 0; n < groupNum; n++)
	{
		if (mod_->group[n].area > (mod_->groupAreaThresh +mod_recall_flag)&&
			mod_->group[n].area < GROUP_AREA_THRESH_MAX)
		{
			find_extreme_points(mod_->group[n].x, mod_->group[n].y, mod_->group[n].area,
				&mod_->extremePoint[validGroups]);

			// printf("up %d, down %d, left %d, right %d\n",
				// mod_->extremePoint[validGroups].up, mod_->extremePoint[validGroups].down,
				// mod_->extremePoint[validGroups].left, mod_->extremePoint[validGroups].right);

			validGroups++;
		}

#if defined(DEBUG_MOD)
		if (mod_->group[n].area > mod_->groupAreaThresh)
		{
			printf("mod_->group[%d].area  = %d\n", n, mod_->group[n].area);

			int sumX = 0;
			int sumY = 0;
			int luma = 0;
			int sumLumaLast = 0;
			for (int area = 0; area < mod_->group[n].area; area++)
			{
				int x = mod_->group[n].x[area];
				int y = mod_->group[n].y[area];

				// for displaying and debug
				int pos = (y * mod_->frameW + x) * 3;
				*(mod_->input + pos + 0) = 255 - 0 + validGroups * 20;
				*(mod_->input + pos + 1) = 255 - 0 + validGroups * 10;
				*(mod_->input + pos + 2) = 255 - 100 + validGroups * 7;

				// summary x and y
				//sumX += x;
				//sumY += y;

				//int posLuma = (y * mod_->frameW + x);
				//luma += *(mod_->imgBlur + posLuma);

				//if (mod_->flagImgSaving == 0)
				//{
				//	sumLumaLast += *(mod_->imgEven + posLuma);
				//}
				//else
				//{
				//	sumLumaLast += *(mod_->imgOdd + posLuma);
				//}
			}
		}
#endif

	}
	return validGroups;
}


void mod_func_init(mod_t * mod_, int carSizeW, int carSizeH)
{
	_thresh_ = (uint8*)calloc(mod_->frameW * mod_->frameH, sizeof(uint8));
	mod_->imgAvg_float = (float64*)calloc(mod_->frameW * mod_->frameH, sizeof(float64));
	mod_->imgAvg = (uint8*)calloc(mod_->frameW * mod_->frameH, sizeof(uint8));
	mod_->imgGray = (uint8*)calloc(mod_->frameW * mod_->frameH, sizeof(uint8));
	mod_->imgBlur = (uint8*)calloc(mod_->frameW * mod_->frameH, sizeof(uint8));
	mod_->imgDiff = (uint8*)calloc(mod_->frameW * mod_->frameH, sizeof(uint8));
	mod_->imgThresh = (uint8*)calloc(mod_->frameW * mod_->frameH, sizeof(uint8));
	mod_->imgGroup = (uint8*)calloc(mod_->frameW * mod_->frameH * 3, sizeof(uint8));
	mod_->imgEven = (uint8*)calloc(mod_->frameW * mod_->frameH * 3, sizeof(uint8));
	mod_->imgOdd = (uint8*)calloc(mod_->frameW * mod_->frameH * 3, sizeof(uint8));

	gaussLPF_Mask(mod_->blurMask, 5.0);

	mod_->motionThresh = MOTION_TRESH;
	mod_->groupAreaThresh = GROUP_AREA_THRESH_MIN;

	mod_->groupNumMax = GROUP_NUM_THRESH;
	mod_->group = (mod_group_t *)calloc(mod_->groupNumMax, sizeof(mod_group_t));
	mod_->groupX = (short*)calloc(mod_->frameW * mod_->frameH, sizeof(short));
	mod_->groupY = (short*)calloc(mod_->frameW * mod_->frameH, sizeof(short));


	mod_->flagImgSaving = 0;

	mod_->glRect = NULL;

	// mod_->carWidth = carSizeW;
	// mod_->carHeight = carSizeH;
	int carLF[2] = { (mod_->frameW - mod_->carWidth) / 2, (mod_->frameH - mod_->carHeight) / 2 };
	int carRF[2] = { (mod_->frameW + mod_->carWidth) / 2, (mod_->frameH - mod_->carHeight) / 2 };
	int carLB[2] = { (mod_->frameW - mod_->carWidth) / 2, (mod_->frameH + mod_->carHeight) / 2 };
	int carRB[2] = { (mod_->frameW + mod_->carWidth) / 2, (mod_->frameH + mod_->carHeight) / 2 };

	int detectPoly[32] =
	{
		/* 1st detection area */
		0, 0,
		carLF[0], carLF[1],
		carLB[0], carLB[1],
		0, mod_->frameH - 1,

		/* 2nd detection area */
		mod_->frameW - 1, 0 ,
		carRF[0], carRF[1],
		carRB[0], carRB[1],
		mod_->frameW - 1, mod_->frameH - 1 ,

		/* 3th detection area */
		0, 0 ,
		carLF[0], carLF[1],
		carRF[0], carRF[1],
		mod_->frameW - 1, 0,

		/* 4th detection area */
		mod_->frameW - 1, mod_->frameH - 1 ,
		carRB[0], carRB[1],
		carLB[0], carLB[1],
		0, mod_->frameH - 1
	};

	memset(mod_->modDetectSW, 0, sizeof(mod_->modDetectSW));
	memcpy(mod_->detectPoly, detectPoly, sizeof(detectPoly));
	memset(mod_->extremePoint, 0, sizeof(mod_->extremePoint));
}

static void alert(int detectSW[], int detectSW_[], int detectFlag[], int edgeTrigger[])
{
	static int num = CONTINUOUS_DETECTION_FRAMES;
	for (int i = 0; i < 4; i++)
	{
		if (detectSW_[i] == 1)
		{
			detectFlag[i]++;
		}
		else
		{
			detectFlag[i]--;
			if (detectFlag[i] < 0)
			{
				detectFlag[i] = 0;
			}
		}

		if (detectFlag[i] > num)
		{
			detectSW[i] = 1;
			edgeTrigger[i] = 1;
		}
		//else if ((modDetectFlag[i] < (num - 2)) && edgeTrigger[i] == 1)
		else if ((detectFlag[i] < (num / 2)) && edgeTrigger[i] == 1)
		{
			detectSW[i] = 0;
			detectFlag[i] = 0;
			edgeTrigger[i] = 0;
		}

		if (detectFlag[i] > 10)
			detectFlag[i] = 10;
	}
}

static void make_bounding_info(mod_rect_int_t bdInfo[], extreme_point_t extremeInfo[], int * groups)
{
	int unvalidNum = 0;
	for (int i = 0; i < *groups; i++)
	{
		int left = extremeInfo[i].left;
		int up = extremeInfo[i].up;
		int width = extremeInfo[i].right - extremeInfo[i].left;
		int height = extremeInfo[i].down - extremeInfo[i].up;

		// filter out unusual objects
		if ((width / height > 7) || (height / width > 7))
		{
			// printf("unvalidNum");
			unvalidNum++;
			continue;
		}
#if defined(DEBUG_MOD)
		else if ((width / height > 5) || (height / width > 5))
		{
			// printf("width / height = %.2f\n", (float)width / height);
			// printf("height / width = %.2f\n", (float)height / width);
		}
#endif

		int index = i - unvalidNum;

		bdInfo[index].x = left;
		bdInfo[index].y = up;
		bdInfo[index].width = width;
		bdInfo[index].height = height;

		// printf("bdInfo[index].x  %d\n", bdInfo[index].x);
		// printf("bdInfo[index].y  %d\n", bdInfo[index].y);
		// printf("bdInfo[index].width  %d\n", bdInfo[index].width);
		// printf("bdInfo[index].height  %d\n", bdInfo[index].height);
	}

	*groups -= unvalidNum;
}

static void get_color(unsigned char color[], int count)
{
	int index = count % (sizeof(colors_draw) / 3);
	color[0] = colors_draw[index][0];
	color[1] = colors_draw[index][1];
	color[2] = colors_draw[index][2];
}

#if defined(USING_CV)
static void draw_tracking(mod_rect_int_t * bdInfo, int groups, std::vector<tracking_box_t_mod> & bdPredict, unsigned char * img)
{
	Mat frame(1024, 1024, CV_8UC3);
	frame.data = img;

	// group box
	for (int i = 0; i < groups; i++)
	{
		cv::Point pt1(bdInfo[i].x, bdInfo[i].y);
		cv::Point pt2(bdInfo[i].x + bdInfo[i].width, bdInfo[i].y + bdInfo[i].height);
		cv::rectangle(frame, pt1, pt2, cv::Scalar(255, 127, 80), 1);
	}

	// predic box
	if (bdPredict.size() > 0)
	{
		int idx = 0;
		for (auto & it : bdPredict)
		{
			unsigned char color[3] = { 0 };
			get_color(color, it.id);

			// draw rectangle
			int thickness = 3;
			cv::Point pt1(it.box.x, it.box.y);
			cv::Point pt2(it.box.x + it.box.width, it.box.y + it.box.height);
			cv::rectangle(frame, pt1, pt2, cv::Scalar(color[2], color[1], color[0]), thickness);

			// draw text
			cv::putText(frame,								// target image
				std::to_string(it.id),					// text
				cv::Point(it.box.x + 3, it.box.y + 20),		// top-left position
				cv::FONT_HERSHEY_DUPLEX,
				0.6,
				CV_RGB(color[0], color[1], color[2]),			// font color
				1);

			idx++;
		}
	}
}
#endif

static void calculate_hits(std::vector<tracking_box_t_mod> & tbTracking,
	std::vector<tracking_box_t_mod> & tbResult)
{
	for (auto & ix : tbTracking)
	{
		uint8 flagNewObject = 1;
		for (auto & iy : tbResult)
		{
			if (ix.id == iy.id)
			{
				iy.frame = ix.frame;
				iy.box = ix.box;
				iy.flagHit = 1;
				flagNewObject = 0;
			}
		}

		if (flagNewObject == 1)
		{
			tbResult.push_back(ix);
			tbResult.back().flagHit = 1;
			tbResult.back().hitNum = 0;
		}
	}

	// todo: threshold
	int idx = 0;
	while(idx < tbResult.size())
	{
		auto & iy = tbResult[idx];

		if (iy.flagHit == 1)
		{
			iy.hitNum++;
			iy.flagHit = 0;

			if (iy.hitNum > 10)
			{
				iy.hitNum = 10;
			}
			
		}
		else
		{
			iy.hitNum--;
			if (iy.hitNum < 0)
			{
				tbResult.erase(tbResult.begin() + idx);
				idx--;
			}
			
		}
		//printf("obj id: %d, hitNum: %d\n", iy.id, iy.hitNum);
		idx++;
	}
}

static void alert_hits(mod_t * mod_, std::vector<tracking_box_t_mod> & tb)
{
	int detect_temp[4] = { 0 };
	for (auto & it : tb)
	{
		if (it.hitNum >= CONTINUOUS_DETECTION_FRAMES)
		{
			// if area and object not move, delete 
			// old  - new < 

			int center[2] = { 0 };
			center[0] = int(it.box.x + 0.5 * it.box.width);
			center[1] = int(it.box.y + 0.5 * it.box.height);

			for (int ix = 0; ix < 4; ix++)
			{
				//printf("ix = %d\n", ix);
				if (isPointInPolygon(mod_->detectPoly[ix], center))
				{
					detect_temp[ix] = 1;
				}
			}

			

#if defined(READ_VIDEO)
			if (simulationLog)
			{
				fprintf(simulationLog, "\nframeCount: %d\n", frameCount);
				fprintf(simulationLog, "id: %d, hits: %d\n", it.id, it.hitNum);
			}
#endif
		}
	}

	// set warrning frames
	for (int ix = 0; ix < 4; ix++)
	{
		if (detect_temp[ix] == 1)
		{
			regionWarning[ix] = WARNING_FRAMES;
		}
		else
		{
			regionWarning[ix]--;
			if (regionWarning[ix] < 0)
			{
				regionWarning[ix] = 0;
			}
		}
	}

	// decide warrning
	for (int iy = 0; iy < 4; iy++)
	{
		if (regionWarning[iy] > 0)
		{
			mod_->modDetectSW[iy] = 1;
		}
		else
		{
			mod_->modDetectSW[iy] = 0;
		}
	}
}

static void make_gl_rect(mod_t* mod_, std::vector<tracking_box_t_mod> & bdMatched)
{
	if (bdMatched.size() > 0)
	{
		free(mod_->glRect);
		mod_->glRectNum = bdMatched.size();
		mod_->glRect = (gl_rect_t*)calloc(mod_->glRectNum, sizeof(gl_rect_t));

		int idx = 0;
		for (auto& it : bdMatched)
		{
			unsigned char color[3] = { 0 };
			if (mod_->glRect)
			{
				mod_->glRect->id = it.id;

				// setting rectangle
				mod_->glRect->x1 = it.box.x;
				mod_->glRect->y1 = it.box.y;
				mod_->glRect->x2 = it.box.x + it.box.width;
				mod_->glRect->y2 = it.box.y + it.box.height;
			}
		}
	}
	else
	{
		mod_->glRectNum = 0;
	}
}
	std::vector<tracking_box_t_mod> trackingResult_mod;
	std::vector<tracking_box_t_mod> detFrameData_mod;

extern void mod_func(mod_t * mod_)
{
	int w = mod_->frameW;
	int h = mod_->frameH;

	float64* avg_float = mod_->imgAvg_float;
	uint8* avg = mod_->imgAvg;
	uint8* gray = mod_->imgGray;
	uint8* blur = mod_->imgBlur;
	uint8* diff = mod_->imgDiff;
	uint8* thresh = mod_->imgThresh;
	uint8* group = mod_->imgGroup;
	uint8* imgEven = mod_->imgEven;
	int modDetectSW_[4] = { 0 };

	memset(modDetectSW_, 0, sizeof(modDetectSW_));
	mod_->flagTarget = 1;
	frameCount++;
	// printf("\nframeCount: %d\n", frameCount);
#if defined(DEBUG_MOD)
	cvtColor(gray, mod_->input, w, h); // rgb to gray
#else
	gray = mod_->input;
#endif
	//fwrite_rgba(mod_->imgGray, mod_->frameW , mod_->frameH, "func_mod.rgba");
	gaussLPF(blur, gray, w, h, mod_->blurMask);



#ifdef MOD_2
	/*printf("mod_->flagImgSaving %d\n", mod_->flagImgSaving);*/
	if (mod_->flagImgSaving == 0)
	{
		absdiff(mod_->imgDiff, blur, mod_->imgEven, w, h); // even frames
	}
	else
	{
		absdiff(mod_->imgDiff, blur, mod_->imgOdd, w, h); // odd frames
	}
#else
	absdiff(diff, avg, blur, w, h); // SAD
#endif


	threshold(thresh, mod_->imgDiff, (mod_->motionThresh+mod_recall_flag), 255, w, h);

// 

	//cv_display_img(w, h, blur, "blur_gray", 1);
	//*cv_display_img(w, h, thresh, "2-0", 1); // debug


	int validGroups = find_groups(mod_, w, h);
	

	mod_rect_int_t * bdInfo = (mod_rect_int_t*)malloc(sizeof(mod_rect_int_t) * validGroups);
	make_bounding_info(bdInfo, mod_->extremePoint, &validGroups);
	mod_->exPNum = validGroups;
	// mod_tracking(bdInfo, validGroups, trackingResult);

	detFrameData_mod.clear();
	for (int i = 0; i < validGroups; i++)
	{
		tracking_box_t_mod tb_mod = {};
		tb_mod.frame = frameCount;
		tb_mod.id = 0;
		tb_mod.class_id = 0; //
		tb_mod.box = bdInfo[i];
		// res.area = (*it).area; // un
		detFrameData_mod.push_back(tb_mod);
	}
	trackingResult_mod = autosys_tracker_func(
		detFrameData_mod,
		frameCount,
		1);
#ifdef DEBUG_MOD
#ifdef USING_CV
	draw_tracking(bdInfo, validGroups, trackingResult, mod_->input);
#else
	
#endif //USING_CV
#endif // DEBUG
	// make_gl_rect(mod_, trackingResult);
	alert_hits(mod_, trackingResult_mod);
#ifdef MOD_2
	if (mod_->flagImgSaving == 0)
	{
		memcpy(mod_->imgEven, blur, w * h * sizeof(uint8));
	}
	else
	{
		memcpy(mod_->imgOdd, blur, w * h * sizeof(uint8));
	}
	if (mod_->flagImgSaving == 0)
	{
		mod_->flagImgSaving = 1;
	}
	else
	{
		mod_->flagImgSaving = 0;
	}
#endif
	free(bdInfo);
}

void mod_release(mod_t *mod_)
{
	free(mod_->imgAvg_float);
	free(mod_->imgAvg);
	free(mod_->imgGray);
	free(mod_->imgBlur);
	free(mod_->imgDiff);
	free(mod_->imgThresh);
	free(mod_->imgGroup);
	free(mod_->imgEven);
	free(mod_->imgOdd);
	free(mod_->group);
	free(_thresh_);
	free(mod_->groupX);
	free(mod_->groupY);

#if defined(READ_VIDEO)
	if (simulationLog != NULL)
	{
		fclose(simulationLog);
	}
#endif
}

extern void open_log(char * folder)
{
#if defined(READ_VIDEO)
	// simulation log
	char dirSimulation[255];
	snprintf(dirSimulation, sizeof dirSimulation, "%s%s%sdetect_result_%d_%d_%d_%d_%d%%.txt",
		SOURCE_ROOT, folder, SIMULATION_FOLDER, MOTION_TRESH, GROUP_AREA_THRESH_MIN,
		GROUP_NUM_THRESH, CONTINUOUS_DETECTION_FRAMES, int(PRESERVATION_RATE * 100));

	errno_t error_code;
	error_code = fopen_s(&simulationLog, dirSimulation, "w");
	if (error_code != 0)
	{
		perror("create simulation log error");
	}
#endif
}
