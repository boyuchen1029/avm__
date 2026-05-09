
// #include "../../system.h"
// #include "mod/tracking.h"
// #include "mod/rectangle_cvToC.h"

// using namespace std;

// static int idCount = 0;
// static int frameCount = -1;
// vector<tracking_box_t> detFrameData;
// extern void mod_tracking(mod_rect_int_t* bdInfo, int groups, std::vector<tracking_box_t> & trackingResult)
// {
// 	printf("%s --- %d\n",__func__,__LINE__);

// 	frameCount++;
// 	printf("groups = %d\n",groups);
// 	// make bounding box info as tracker input

// 	printf("%s --- %d\n",__func__,__LINE__);
// 	detFrameData.clear();
// 	for (int i = 0; i < groups; i++)
// 	{
// 		tracking_box_t tb = {};
// 		tb.frame = frameCount;
// 		tb.id = idCount;
// 		tb.class_id = 0; //
// 		tb.box = bdInfo[i];
// 		// res.area = (*it).area; // un

// 		detFrameData.push_back(tb);
// 	}
// printf("%s --- %d\n",__func__,__LINE__);

// 	// do tracker function
// 	int detectionFlag = 1;
// 	// trackingResult = autosys_tracker_func(
// 	// 	detFrameData,
// 	// 	frameCount,
// 	// 	detectionFlag);
// 	// 	printf("%s --- %d\n",__func__,__LINE__);

// }