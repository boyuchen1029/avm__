#include "../../autosys/autosys.h"
#include "./include/avm/bb_func.h"
#include "avm/maths.h"
#include "../../system.h"
#include "avm/fp_source.h"
#include <math.h>
/*============================================================

		AVM_Brightness_balance

==============================================================*/
AvmBrightnessBalanceConfig g_avmBBConfig;
unsigned char *m_cameraFileFrontBuf;
unsigned char *m_cameraFileBackBuf;
unsigned char *m_cameraFileLeftBuf;
unsigned char *m_cameraFileRightBuf;
unsigned char *m_cameraFileFront;
unsigned char *m_cameraFileBack;
unsigned char *m_cameraFileLeft;
unsigned char *m_cameraFileRight;

static int m_total_ov_area;
static int *m_colorblk0;
static int *m_colorblk1;
static int *m_colorblk2;
static int *m_colorblk3;
static int *m_colorblk4;
static int *m_colorblk5;
static int *m_colorblk6;
static int *m_colorblk7;
static int *m_colorblk_mid0;
static int *m_colorblk_mid1;
static int *m_colorblk_mid2;
static int *m_colorblk_mid3;

int dark_count = 0;

/**
 * @brief :  Adjustment avm brightness
 * @author : Boyuchen
 * @date : 2024/03/26
 */
int g_avm_BB_PID_Latency = 6;
int m_avm_BB_GAMMA_FLAG = 0;
int m_HOMO_Rside_flag = 0;
int m_HOMO_Lside_flag = 0;
BrignessBlance brignessblance;
colorObj colorData;
vignetteConfig vignetteData[MAX_CAM_NUM];

static int *y_list, *g_u_list, *r_v_list, *g_v_list, *b_u_list;

static float m_tip_gamma_top[2];
static float m_tip_gamma_down[2];
static float m_tip_homo_top[2];
static float m_tip_homo_down[2];


static float rgb_diff13[3];
static float rgb_diff24[3];
static float rgb_diff1_13[3];
static float rgb_diff2_24[3];
static float rgb_diff1_13_negative[3];
static float rgb_diff2_24_negative[3];

static VEC3 SUM_LF, SUM_LB;
static VEC3 SUM_FL, SUM_BL;
static VEC3 SUM_RF, SUM_RB;
static VEC3 SUM_FR, SUM_BR;
static VEC3 SUM_FM, SUM_BM;
static VEC3 SUM_RM, SUM_LM;


static VEC3 SUM_Diff_LF_Minus_FL;
static VEC3 SUM_Diff_RF_Minus_FR;
static VEC3 SUM_Diff_LB_Minus_BL;
static VEC3 SUM_Diff_RB_Minus_BR;

static VEC3 RESULT_FR;
static VEC3 RESULT_FL;
static VEC3 RESULT_R;
static VEC3 RESULT_L;

static float diff_L_y;
static float diff_R_y;
static float diff_FL_y;
static float diff_FR_y;
static float diff_BL_y;
static float diff_BR_y;


static float avg_L_y;
static float avg_R_y;
static float avg_FL_y;
static float avg_FR_y;
static float avg_BL_y;
static float avg_BR_y;
static float avg_LF_y;
static float avg_LB_y;
static float avg_RF_y;
static float avg_RB_y;

#pragma region Init
/*============================================================

		INIT

==============================================================*/
void read_vignette_table()
{
    FILE *fpBin = fp_source_app("/EVSTable/avm/vignette.dat", "r");
    if(fpBin != NULL)
    {
        for(int i = 0; i < MAX_CAM_NUM; i++)
        {
            fp_read_data_safty(&vignetteData[i].sampler_point[0], 12, 1 * sizeof(int), fpBin);
            fp_read_data_safty(&vignetteData[i].nor_value, 1, 1 * sizeof(float), fpBin);
            fp_read_data_safty(&vignetteData[i].angle, 1, 1 * sizeof(float), fpBin);
            AVM_LOGI(LOG_RED("[DEBUG]")"cam[] angle = %f, nor_value = %f, sampler_point = (%d,%d),(%d,%d)\n",
                    vignetteData[i].angle, vignetteData[i].nor_value,
                    vignetteData[i].sampler_point[0].X, vignetteData[i].sampler_point[0].Y,
                    vignetteData[i].sampler_point[1].X, vignetteData[i].sampler_point[1].X);
        }
    }
    else
    {
        vignetteData[0].angle = 0.0f;
        vignetteData[1].angle = 0.0f;
        vignetteData[2].angle = 0.0f;
        vignetteData[3].angle = 0.0f;
        vignetteData[3].nor_value = 0.0f;
        vignetteData[3].nor_value = 0.0f;
        vignetteData[3].nor_value = 0.0f;
        vignetteData[3].nor_value = 0.0f;
    }
}

#pragma endregion

#pragma region  IMG_FUNC
/*============================================================

		IMG_FUNC main

==============================================================*/
static void BY_yuvlist(int *y_list, int *g_u_list, int *g_v_list, int *r_v_list, int *b_u_list)
{
	int i;
	for (i = 0; i < 256; i++)
	{
		y_list[i]   =  (i-16 < 0) ? 0 : (1192 * (i - 16))  >> 10;
		r_v_list[i] =  (i-128 < 0) ? 0: (1634 * (i - 128)) >> 10;
		g_u_list[i] =  (i-128 < 0) ? 0: (833 *  (i - 128)) >> 10;
		g_v_list[i] =  (i-128 < 0) ? 0: (400 *  (i - 128)) >> 10;
		b_u_list[i] =  (i-128 < 0) ? 0: (2066 * (i - 128)) >> 10;
	}
}
static void YUV2RGBnew(unsigned char tmp_y, unsigned char tmp_u, unsigned char tmp_v, unsigned char *input_r, unsigned char *input_g, unsigned char *input_b, int *y_list, int *r_v_list, int *g_u_list, int *g_v_list, int *b_u_list)
{
	int r, g, b;
	r = 0;
	g = 0;
	b = 0;
	//  AVM_LOGI("%d \n", tmp_y);
	r = (y_list[tmp_y] + r_v_list[tmp_v]);
	g = (y_list[tmp_y] - g_u_list[tmp_u] - g_v_list[tmp_v]);
	b = y_list[tmp_y] + b_u_list[tmp_u];
	*input_r = r > 255 ? 255 : r < 0 ? 0
									 : r;
	*input_g = g > 255 ? 255 : g < 0 ? 0
									 : g;
	*input_b = b > 255 ? 255 : b < 0 ? 0
									 : b;
}

float rgb2yuvv(VEC3 color)
{
	return 0.299f*(float)color.X+0.587f*(float)color.Y+0.114f*(float)color.Z;
}
#pragma endregion 

#pragma region Vignette
/*============================================================

		Calculate_Vignette_Value

==============================================================*/
void generate_circle_point_byRadius(Point* center, int radius, int* outCount, Point* pts)
{
	if (pts == NULL)
		return ;

    int i = 0;
    for (int dy = -radius; dy <= radius; dy++)
    {
        for (int dx = -radius; dx <= radius; dx++)
        {
            if (dx * dx + dy * dy <= radius * radius)
            {
                pts[i].X = center->X + dx;
                pts[i++].Y = center->Y + dy;
            }
        }
    }

    if (outCount)
        *outCount = i;  
}
void Calculate_Vignette_Value(unsigned char * avmcamera, int view ,int side)
{
	if(side == 2) return;
	Point center_h, center_l, center_o;
    center_h = vignetteData[view].sampler_point[(side * 3) + 0];
    center_l = vignetteData[view].sampler_point[(side * 3) + 1];
    center_o = vignetteData[view].sampler_point[(side * 3) + 2];

	int radius = 10;
	int lh_effectCount = 0;
	int ll_effectCount = 0;
	int lo_effectCount = 0;
	float (*output)[3] = (side == 0) ? colorData.vignette.Y_vignette_Cam_1
                                 	 : colorData.vignette.Y_vignette_Cam_2;
	Point output_lh[1000], output_ll[1000], output_lo[1000];				 
	generate_circle_point_byRadius(&center_h, radius, &lh_effectCount, output_lh);
	generate_circle_point_byRadius(&center_l, radius, &ll_effectCount, output_ll);
	generate_circle_point_byRadius(&center_o, radius, &lo_effectCount, output_lo);
	//return;
	Point *list[3] = {output_lh, output_ll, output_lo};
	int ov_area[3] = {lh_effectCount, ll_effectCount, lo_effectCount};
	int sum_yuv_list = 0;

	for (int idx = 0; idx < 3; idx++)
	{
		int total_tmp_y = 0;
		int total_count = 0;
		for (int ptr = 0; ptr < ov_area[idx]; ptr++)
		{
			unsigned char tmpy;
			int px, py, ptry;
			px = list[idx][ptr].X;
			py = list[idx][ptr].Y;

			if (px > imageWidth - 5) px = imageWidth - 5;
			if (px < 0) px = 0;
			if (py > imageHeight - 5) py = imageHeight - 5;
			if (py < 0) py = 0;
			ptry = 2 * (px + py * imageWidth);
			tmpy = avmcamera[ptry];

			if (tmpy >= 10 && tmpy <= 240)
			{
				total_tmp_y += (int)tmpy;
				total_count++;
			}
		}

        output[view][idx] = (total_count != 0)
            ? (float)total_tmp_y / (float)total_count
            : 0.0f;
	}
	
	// for (int idx = 0; idx < 3; idx++)
	// {
	// 	if(list[idx] != NULL)
	// 	{
	// 		free(list[idx]);
	// 	}
	// }
}

float RatioLimitSmooth(float x)
{
    if (x >= 125.0f) return 0.30f;
    if (x <= 60.0f) return 0.70f;

    float t = (x - 60.0f) / 65.0f; // 0..1 對應 60..125
    float p = (x >= 100.0f) ? 1.0f : 0.415f;
    return 0.70f - 0.40f * (float)powf(t, p);
}

float vignette_confidence(float x)
{
	if (x >= 125.0f) return 1.0f;
	if (x <= 60.0f)  return 0.70f;
	return (0.00461538f * x + 0.4230769231f);
}
float calculate_vignette_ratio(float *srcY, float basic, float nor_value, float preValue)
{
	float pY_1 = srcY[0];
	float pY_2 = srcY[1];
	float ratio1 = 0.0f;
	float ratio2 = 0.0f;
	ratio1 = 1.0f - pY_1 / basic;
	ratio2 = 1.0f - pY_2 / basic;

    float v1 = 0.0f;
    float v2 = 0.0f;
    bool flag = false;
    float standard = 1.0f;
    float confidence = 1.0f;
    standard   = RatioLimitSmooth(basic);
	confidence = vignette_confidence(basic);
    if (basic > 50.0f)
	{
        if (basic > pY_1 && ratio1 < standard)
        {
            v1 = (basic - pY_1) / 255.0f * nor_value * confidence;
            flag |= true;
        }

        if (basic > pY_2 && ratio2 < standard)
        {
            v2 = (basic - pY_2) / 255.0f * nor_value * confidence;
            flag |= true;
        }
    }


	return (flag)?((v1 > v2)?v1:v2):(preValue * 0.8f);
}
#pragma endregion

#pragma region AVM BB PID
/*============================================================

		PID main

==============================================================*/
void init_BrignessBlance()
{
	//(int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	brignessblance.is_first_record           = (int*)calloc(PARAM_AVM_BB_PID_BRIGNESS_CAM_NUM, sizeof(int));
	brignessblance.each_record_current_count = (int*)calloc(PARAM_AVM_BB_PID_BRIGNESS_CAM_NUM, sizeof(int));


	brignessblance.frame_list = (float**)malloc(sizeof(float*) * PARAM_AVM_BB_PID_BRIGNESS_CAM_NUM);
	for(int ix = 0; ix < PARAM_AVM_BB_PID_BRIGNESS_CAM_NUM; ix++)
	{
		brignessblance.frame_list[ix] = (float*)calloc(PARAM_AVM_BB_PID_BRIGNESS_LIST_BUFFER_LENGTH, sizeof(float));
	}

}

void AVM_BB_RESET()
{
	VEC3_RESET(SUM_LF);
	VEC3_RESET(SUM_FL);
	VEC3_RESET(SUM_RF);
	VEC3_RESET(SUM_FR);
	VEC3_RESET(SUM_LB);
	VEC3_RESET(SUM_BL);
	VEC3_RESET(SUM_RB);
	VEC3_RESET(SUM_BR);
}

void AVM_Brightness_first_count_reset(BrignessBlance* BB, int current_select)
{
	BB->is_first_record[current_select] = 0;
}

float AVM_Brightness_balance_PID(float* frame_list_brigness, int sampler_count, int type, BrignessBlanceAlgorithm *algorithm)
{
	float total_brigness = 0;
	switch(type)
	{
		case BRIGNESS_BLANCES_LINEAR:
			for(int frame = 0; frame < sampler_count; frame++)
			{
				if (!isnan(frame_list_brigness[frame]))
				{
					total_brigness += frame_list_brigness[frame];
				}
			}
			total_brigness /= (float)sampler_count;
			return total_brigness;
			break;
		default:
			break;
		
	}
	return total_brigness;
	
}

void AVM_Brightness_DATA_Loading(BrignessBlance* BB, int current_select, int sampler_count, VEC3* PIXEL, float PIXEL_Y)
{
	float Brigness = 0.0f;
	if(PIXEL != NULL)
	{
		float r = (float)PIXEL->X / 255.0f;
		float g = (float)PIXEL->Y / 255.0f;
		float b = (float)PIXEL->Z / 255.0f;
	  	Brigness = 0.299f * r   + 0.587f * g   + 0.114f * b;
	}
	else
	{
		Brigness = PIXEL_Y;
	}

	if(BB->is_first_record[current_select] != 0)
	{
		int cur_num = BB->each_record_current_count[current_select];
		cur_num = (cur_num + 1) % (sampler_count);
		BB->frame_list[current_select][cur_num] = Brigness;
		BB->each_record_current_count[current_select] = cur_num;
	}
	else
	{
		BB->is_first_record[current_select] = 1;
		for(int ix = 0; ix < sampler_count; ix++)
		{
			BB->frame_list[current_select][ix] = Brigness;
		}
	}
}

void AVM_Brightness_DATA_Update(BrignessBlance* BB, int current_select, int sampler_count, float Brigness)
{
	int cur_num = BB->each_record_current_count[current_select];
	BB->frame_list[current_select][cur_num] = Brigness;
}

void AVM_Brightness_DATA_Clear(BrignessBlance* BB, int current_select, int sampler_count)
{
	//AVM_Brightness_first_count_reset(BB, current_select);
	BB->is_first_record[current_select] = 1;
	BB->each_record_current_count[current_select] = 0;
	for(int ix = 0; ix < sampler_count; ix++)
	{
		BB->frame_list[current_select][ix] = 0.0f;
	}
}
void AVM_PID_GROUP_DATA_Clear()
{
	AVM_Brightness_DATA_Clear(&brignessblance, 0, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME);
	AVM_Brightness_DATA_Clear(&brignessblance, 1, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME);
	AVM_Brightness_DATA_Clear(&brignessblance, 2, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME);
	AVM_Brightness_DATA_Clear(&brignessblance, 3, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME);
	AVM_BB_RESET();
}
void PID_BB_FLOW()
{
	#if PARAM_AVM_BB_PID_FUNCTION

	// VEC3  *SrcVec3buffer[] = {&RESULT_L, &RESULT_R, &RESULT_FL, &RESULT_FR};
	// float *tmpVbuffer[] = {
	// 	&diff_L_y, &diff_R_y, &diff_FL_y, &diff_FR_y, 
	// 	&m_tip_homo_top, &m_tip_homo_top,
	// 	&colorData.diff_pid_value.diffFL, &colorData.diff_pid_value.diffFR, &colorData.diff_pid_value.diffBL, &colorData.diff_pid_value.diffBR,
	// 	&colorData.diff_pid_value.diffLF, &colorData.diff_pid_value.diffLB, &colorData.diff_pid_value.diffRF, &colorData.diff_pid_value.diffRB,
	// 	&colorData.gamma_value.Y_gamma_FL, &colorData.gamma_value.Y_gamma_BL, &colorData.gamma_value.Y_gamma_LL, &colorData.gamma_value.Y_gamma_RL,
	// 	&colorData.gamma_value.Y_gamma_FR, &colorData.gamma_value.Y_gamma_BR, &colorData.gamma_value.Y_gamma_LR, &colorData.gamma_value.Y_gamma_RR
	// };

	// float srcfloatbuffer[] = {
	// 	0, 0 ,0 ,0,
	// 	m_tip_homo_top[0], m_tip_homo_top[1],
	// 	colorData.diff_value.diffFL, colorData.diff_value.diffFR, colorData.diff_value.diffBL, colorData.diff_value.diffBR,
	// 	colorData.diff_value.diffLF, colorData.diff_value.diffLB, colorData.diff_value.diffRF, colorData.diff_value.diffRB,
	// 	colorData.gamma_value.Y_gamma_FL, colorData.gamma_value.Y_gamma_BL, colorData.gamma_value.Y_gamma_LL, colorData.gamma_value.Y_gamma_RL,
	// 	colorData.gamma_value.Y_gamma_FR, colorData.gamma_value.Y_gamma_BR, colorData.gamma_value.Y_gamma_LR, colorData.gamma_value.Y_gamma_RR
	// };

	// for(int ix = 0; ix < 4; ix++)
	// {
	// 	AVM_Brightness_DATA_Loading(&brignessblance, ix, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, SrcVec3buffer[ix], 0.0f);
	// 	*(tmpVbuffer[ix]) = AVM_Brightness_balance_PID(brignessblance.frame_list[ix],  PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	// 	AVM_Brightness_DATA_Update(&brignessblance, ix, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, *(tmpVbuffer[ix]));
	// }

	// for(int ix = 4; ix < sizeof(tmpVbuffer)/sizeof(tmpVbuffer[0]); ix++)
	// {
	// 	AVM_Brightness_DATA_Loading(&brignessblance, ix, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, NULL, srcfloatbuffer[ix]);
	// 	*(tmpVbuffer[ix]) = AVM_Brightness_balance_PID(brignessblance.frame_list[ix],  PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	// 	AVM_Brightness_DATA_Update(&brignessblance, ix, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, *(tmpVbuffer[ix]));
	// }


	//printf("PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME %d\n",PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME);
	AVM_Brightness_DATA_Loading(&brignessblance, 0, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, &RESULT_L, 0.0f);
	AVM_Brightness_DATA_Loading(&brignessblance, 1, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, &RESULT_R, 0.0f);
	AVM_Brightness_DATA_Loading(&brignessblance, 2, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, &RESULT_FL, 0.0f);
	AVM_Brightness_DATA_Loading(&brignessblance, 3, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, &RESULT_FR, 0.0f);
	AVM_Brightness_DATA_Loading(&brignessblance, 4, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, NULL, m_tip_homo_top[0]);
	AVM_Brightness_DATA_Loading(&brignessblance, 5, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, NULL, m_tip_homo_top[1]);

	AVM_Brightness_DATA_Loading(&brignessblance, 6, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.diff_value.diffFL);
	AVM_Brightness_DATA_Loading(&brignessblance, 7, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.diff_value.diffFR);
	AVM_Brightness_DATA_Loading(&brignessblance, 8, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.diff_value.diffBL);
	AVM_Brightness_DATA_Loading(&brignessblance, 9, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.diff_value.diffBR);
	AVM_Brightness_DATA_Loading(&brignessblance, 10, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.diff_value.diffLF);
	AVM_Brightness_DATA_Loading(&brignessblance, 11, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.diff_value.diffLB);
	AVM_Brightness_DATA_Loading(&brignessblance, 12, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.diff_value.diffRF);
	AVM_Brightness_DATA_Loading(&brignessblance, 13, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.diff_value.diffRB);

	AVM_Brightness_DATA_Loading(&brignessblance, 14, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.gamma_value.Y_gamma_FL);
	AVM_Brightness_DATA_Loading(&brignessblance, 15, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.gamma_value.Y_gamma_BL);
	AVM_Brightness_DATA_Loading(&brignessblance, 16, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.gamma_value.Y_gamma_LL);
	AVM_Brightness_DATA_Loading(&brignessblance, 17, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.gamma_value.Y_gamma_RL);
	AVM_Brightness_DATA_Loading(&brignessblance, 18, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.gamma_value.Y_gamma_FR);
	AVM_Brightness_DATA_Loading(&brignessblance, 19, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.gamma_value.Y_gamma_BR);
	AVM_Brightness_DATA_Loading(&brignessblance, 20, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.gamma_value.Y_gamma_LR);
	AVM_Brightness_DATA_Loading(&brignessblance, 21, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME , NULL, colorData.gamma_value.Y_gamma_RR);

	diff_L_y  = AVM_Brightness_balance_PID(brignessblance.frame_list[0], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	diff_R_y  = AVM_Brightness_balance_PID(brignessblance.frame_list[1], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	diff_FL_y = AVM_Brightness_balance_PID(brignessblance.frame_list[2], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	diff_FR_y = AVM_Brightness_balance_PID(brignessblance.frame_list[3], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	diff_BL_y = -diff_FL_y;
	diff_BR_y = -diff_FR_y;
	m_tip_homo_top[0]   = AVM_Brightness_balance_PID(brignessblance.frame_list[4],   PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME + 3, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	m_tip_homo_top[1]   = AVM_Brightness_balance_PID(brignessblance.frame_list[5],   PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME + 3, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);


	colorData.diff_pid_value.diffFL = AVM_Brightness_balance_PID(brignessblance.frame_list[6],  PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.diff_pid_value.diffFR = AVM_Brightness_balance_PID(brignessblance.frame_list[7],  PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.diff_pid_value.diffBL = AVM_Brightness_balance_PID(brignessblance.frame_list[8], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.diff_pid_value.diffBR = AVM_Brightness_balance_PID(brignessblance.frame_list[9], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.diff_pid_value.diffLF = AVM_Brightness_balance_PID(brignessblance.frame_list[10],  PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.diff_pid_value.diffLB = AVM_Brightness_balance_PID(brignessblance.frame_list[11],  PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.diff_pid_value.diffRF = AVM_Brightness_balance_PID(brignessblance.frame_list[12], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.diff_pid_value.diffRB = AVM_Brightness_balance_PID(brignessblance.frame_list[13], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);

	colorData.gamma_value.Y_gamma_FL = AVM_Brightness_balance_PID(brignessblance.frame_list[14], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.gamma_value.Y_gamma_BL = AVM_Brightness_balance_PID(brignessblance.frame_list[15], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
    colorData.gamma_value.Y_gamma_LL = AVM_Brightness_balance_PID(brignessblance.frame_list[16], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.gamma_value.Y_gamma_RL = AVM_Brightness_balance_PID(brignessblance.frame_list[17], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
    colorData.gamma_value.Y_gamma_FR = AVM_Brightness_balance_PID(brignessblance.frame_list[18], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.gamma_value.Y_gamma_BR = AVM_Brightness_balance_PID(brignessblance.frame_list[19], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
    colorData.gamma_value.Y_gamma_LR = AVM_Brightness_balance_PID(brignessblance.frame_list[20], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);
	colorData.gamma_value.Y_gamma_RR = AVM_Brightness_balance_PID(brignessblance.frame_list[21], PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, BRIGNESS_BLANCES_LINEAR, &brignessblance.algorithm);

	AVM_Brightness_DATA_Update(&brignessblance, 0, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, diff_L_y);
	AVM_Brightness_DATA_Update(&brignessblance, 1, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, diff_R_y);
	AVM_Brightness_DATA_Update(&brignessblance, 2, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, diff_FL_y);
	AVM_Brightness_DATA_Update(&brignessblance, 3, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, diff_FR_y);
	AVM_Brightness_DATA_Update(&brignessblance, 4, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, m_tip_homo_top[0]);
	AVM_Brightness_DATA_Update(&brignessblance, 5, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, m_tip_homo_top[1]);

	AVM_Brightness_DATA_Update(&brignessblance, 6, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME,  colorData.diff_pid_value.diffFL);
	AVM_Brightness_DATA_Update(&brignessblance, 7, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME,  colorData.diff_pid_value.diffFR);
	AVM_Brightness_DATA_Update(&brignessblance, 8, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME,  colorData.diff_pid_value.diffBL);
	AVM_Brightness_DATA_Update(&brignessblance, 9, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME,  colorData.diff_pid_value.diffBR);
	AVM_Brightness_DATA_Update(&brignessblance, 10, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.diff_pid_value.diffLF);
	AVM_Brightness_DATA_Update(&brignessblance, 11, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.diff_pid_value.diffLB);
	AVM_Brightness_DATA_Update(&brignessblance, 12, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.diff_pid_value.diffRF);
	AVM_Brightness_DATA_Update(&brignessblance, 13, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.diff_pid_value.diffRB);

    AVM_Brightness_DATA_Update(&brignessblance, 14, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.gamma_value.Y_gamma_FL);
	AVM_Brightness_DATA_Update(&brignessblance, 15, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.gamma_value.Y_gamma_BL);
	AVM_Brightness_DATA_Update(&brignessblance, 16, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.gamma_value.Y_gamma_LL);
	AVM_Brightness_DATA_Update(&brignessblance, 17, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.gamma_value.Y_gamma_RL);
    AVM_Brightness_DATA_Update(&brignessblance, 18, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.gamma_value.Y_gamma_FR);
	AVM_Brightness_DATA_Update(&brignessblance, 19, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.gamma_value.Y_gamma_BR);
	AVM_Brightness_DATA_Update(&brignessblance, 20, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.gamma_value.Y_gamma_LR);
	AVM_Brightness_DATA_Update(&brignessblance, 21, PARAM_AVM_BB_PID_BRIGNESS_GREP_FRAME, colorData.gamma_value.Y_gamma_RR);

	if(autosys.current_info.BB_VERSION == 1)
	{
		diff_FL_y = colorData.diff_pid_value.diffFL;
		diff_FR_y = colorData.diff_pid_value.diffFR;
		diff_BL_y = colorData.diff_pid_value.diffBL;
		diff_BR_y = colorData.diff_pid_value.diffBR;
	}
#else
	diff_L_y  = 0.299f * (float)RESULT_L.X / 255.0f  + 0.587f * (float)RESULT_L.Y/255.0f    + 0.114f * (float)RESULT_L.Z / 255.0f;
	diff_R_y  = 0.299f * (float)RESULT_R.X / 255.0f  + 0.587f * (float)RESULT_R.Y/255.0f    + 0.114f * (float)RESULT_R.Z / 255.0f;
	diff_FL_y = 0.299f * (float)RESULT_FL.X / 255.0f + 0.587f * (float)RESULT_FL.Y/255.0f   + 0.114f * (float)RESULT_FL.Z / 255.0f;
	diff_FR_y = 0.299f * (float)RESULT_FR.X / 255.0f + 0.587f * (float)RESULT_FR.Y/255.0f   + 0.114f * (float)RESULT_FR.Z / 255.0f;
	diff_BL_y = -diff_FL_y;
	diff_BR_y = -diff_FR_y;
#endif

	SAFE_VAL(m_tip_homo_top[0], 0.0f);
	SAFE_VAL(m_tip_homo_top[1], 0.0f);

    g_avmBBConfig.diff[0]      = 0.0f;
    g_avmBBConfig.diff[1]      = 0.0f;
    g_avmBBConfig.diff[2]      = diff_L_y;
    g_avmBBConfig.diff[3]      = diff_R_y;
    g_avmBBConfig.avg[0]       = 0.0f;
    g_avmBBConfig.avg[1]       = 0.0f;
    g_avmBBConfig.avg[2]       = avg_L_y;
    g_avmBBConfig.avg[3]       = avg_R_y;

    g_avmBBConfig.homo[0]      = m_tip_homo_top[0];
    g_avmBBConfig.homo[1]      = m_tip_homo_top[0];
    g_avmBBConfig.homo[2]      = m_tip_homo_top[0];
    g_avmBBConfig.homo[3]      = m_tip_homo_top[1];
    g_avmBBConfig.avg_part1[0] = diff_FL_y;
    g_avmBBConfig.avg_part1[1] = diff_BL_y;
    g_avmBBConfig.avg_part1[2] = colorData.diff_pid_value.diffLF;
    g_avmBBConfig.avg_part1[3] = colorData.diff_pid_value.diffRF;
    g_avmBBConfig.avg_part2[0] = diff_FR_y;
    g_avmBBConfig.avg_part2[1] = diff_BR_y;
    g_avmBBConfig.avg_part2[2] = colorData.diff_pid_value.diffLB;
    g_avmBBConfig.avg_part2[3] = colorData.diff_pid_value.diffRB;

    g_avmBBConfig.diff_part1[0] = colorData.diff_pid_value.diffFL;
    g_avmBBConfig.diff_part1[1] = colorData.diff_pid_value.diffBL;
    g_avmBBConfig.diff_part1[2] = colorData.diff_pid_value.diffLF;
    g_avmBBConfig.diff_part1[3] = colorData.diff_pid_value.diffRF;
    g_avmBBConfig.diff_part2[0] = colorData.diff_pid_value.diffFR;
    g_avmBBConfig.diff_part2[1] = colorData.diff_pid_value.diffBR;
    g_avmBBConfig.diff_part2[2] = colorData.diff_pid_value.diffLB;
    g_avmBBConfig.diff_part2[3] = colorData.diff_pid_value.diffRB;


    g_avmBBConfig.gammaL[0]    = colorData.gamma_value.Y_gamma_FL;
    g_avmBBConfig.gammaL[1]    = colorData.gamma_value.Y_gamma_BL;
    g_avmBBConfig.gammaL[2]    = colorData.gamma_value.Y_gamma_LL;
    g_avmBBConfig.gammaL[3]    = colorData.gamma_value.Y_gamma_RL;
    g_avmBBConfig.gammaR[0]    = colorData.gamma_value.Y_gamma_FR;
    g_avmBBConfig.gammaR[1]    = colorData.gamma_value.Y_gamma_BR;
    g_avmBBConfig.gammaR[2]    = colorData.gamma_value.Y_gamma_LR;
    g_avmBBConfig.gammaR[3]    = colorData.gamma_value.Y_gamma_RR;

	if(dark_count++ % 160 == 0)
	{
		// printf("--------------------------------------------------------------\n");
        // printf("avmBBConfig para LR: homo[%f,%f],\n",   g_avmBBConfig.homo[2],  g_avmBBConfig.homo[3]);
		// printf("--------------------------------------------------------------\n");
	}
}
#pragma endregion

#pragma region TransData
/*============================================================

		TransData main

==============================================================*/
void trans_cam_buffer_to_bb(unsigned char *cameraF, unsigned char *cameraB, unsigned char *cameraL, unsigned char *cameraR)
{
  m_cameraFileFrontBuf = cameraF;
  m_cameraFileBackBuf  = cameraB;
  m_cameraFileLeftBuf  = cameraL;
  m_cameraFileRightBuf = cameraR;
}

void trans_ReadFile_buffer_to_bb(unsigned char *cameraF, unsigned char *cameraB, unsigned char *cameraL, unsigned char *cameraR)
{
  m_cameraFileFront = cameraF;
  m_cameraFileBack  = cameraB;
  m_cameraFileLeft  = cameraL;
  m_cameraFileRight = cameraR;
}

void trans_color_block_to_bb(int total_ov_area, int *colorblk[12])
{
    m_total_ov_area = total_ov_area;
    int count = 0;
	m_colorblk0 =     colorblk[count++];
	m_colorblk1 =     colorblk[count++];
	m_colorblk2 =     colorblk[count++];
	m_colorblk3 =     colorblk[count++];
	m_colorblk4 =     colorblk[count++];
	m_colorblk5 =     colorblk[count++];
	m_colorblk6 =     colorblk[count++];
	m_colorblk7 =     colorblk[count++];
	m_colorblk_mid0 = colorblk[count++];
	m_colorblk_mid1 = colorblk[count++];
	m_colorblk_mid2 = colorblk[count++];
	m_colorblk_mid3 = colorblk[count++];
    
}
#pragma endregion

#pragma region calculate_color
/*============================================================

		calculate_color main

==============================================================*/
void clear_color_diff_nan()
{
	AVM_LOGI(LOG_RED("[WARNING]") "NaN appearance\n");
	AVM_Brightness_first_count_reset(&brignessblance, 0);
	AVM_Brightness_first_count_reset(&brignessblance, 1);
	AVM_Brightness_first_count_reset(&brignessblance, 2);
	AVM_Brightness_first_count_reset(&brignessblance, 3);
	rgb_diff13[0] = 0; // left
	rgb_diff13[1] = 0;
	rgb_diff13[2] = 0;

	rgb_diff24[0] = 0; // right
	rgb_diff24[1] = 0;
	rgb_diff24[2] = 0;

	rgb_diff1_13[0] = 0; // front
	rgb_diff1_13[1] = 0;
	rgb_diff1_13[2] = 0;

	rgb_diff2_24[0] = 0;
	rgb_diff2_24[1] = 0;
	rgb_diff2_24[2] = 0;

	diff_L_y = 0.0f;
	diff_R_y = 0.0f;
	avg_L_y = 0.0f;
	avg_R_y = 0.0f;
	avg_FL_y = 0.0f;
	avg_FR_y = 0.0f;
	diff_FL_y = 0.0f;
	diff_FR_y = 0.0f;
	avg_BL_y = 0.0f;
	avg_BR_y = 0.0f;
	diff_BL_y = 0.0f;
	diff_BR_y = 0.0f;

	g_avmBBConfig.diff_part1[0] = 0.0f;
	g_avmBBConfig.diff_part1[1] = 0.0f;
	g_avmBBConfig.diff_part1[2] = 0.0f;
	g_avmBBConfig.diff_part1[3] = 0.0f;
	g_avmBBConfig.diff_part2[0] = 0.0f;
	g_avmBBConfig.diff_part2[1] = 0.0f;
	g_avmBBConfig.diff_part2[2] = 0.0f;
	g_avmBBConfig.diff_part2[3] = 0.0f;

	g_avmBBConfig.diff[0] = 0.0f;
	g_avmBBConfig.diff[1] = 0.0f;
	g_avmBBConfig.diff[2] = 0.0f;
	g_avmBBConfig.diff[3] = 0.0f;
	g_avmBBConfig.avg[0] = 0.0f;
	g_avmBBConfig.avg[1] = 0.0f;
	g_avmBBConfig.avg[2] = 0.0f;
	g_avmBBConfig.avg[3] = 0.0f;

	g_avmBBConfig.homo[0] = 0.0f;
	g_avmBBConfig.homo[1] = 0.0f;
	g_avmBBConfig.homo[2] = 0.0f;
	g_avmBBConfig.homo[3] = 0.0f;
	g_avmBBConfig.avg_part1[0] = 0.0f;
	g_avmBBConfig.avg_part1[1] = 0.0f;
	g_avmBBConfig.avg_part1[2] = 0.0f;
	g_avmBBConfig.avg_part1[3] = 0.0f;
	g_avmBBConfig.avg_part2[0] = 0.0f;
	g_avmBBConfig.avg_part2[1] = 0.0f;
	g_avmBBConfig.avg_part2[2] = 0.0f;
	g_avmBBConfig.avg_part2[3] = 0.0f;
	g_avmBBConfig.gammaL[0] = 0.0f;
	g_avmBBConfig.gammaL[1] = 0.0f;
	g_avmBBConfig.gammaL[2] = 0.0f;
	g_avmBBConfig.gammaL[3] = 0.0f;
	g_avmBBConfig.gammaR[0] = 0.0f;
	g_avmBBConfig.gammaR[1] = 0.0f;
	g_avmBBConfig.gammaR[2] = 0.0f;
	g_avmBBConfig.gammaR[3] = 0.0f;
}

void clear_color_diff()
{
	AVM_Brightness_first_count_reset(&brignessblance, 0);
	AVM_Brightness_first_count_reset(&brignessblance, 1);
	AVM_Brightness_first_count_reset(&brignessblance, 2);
	AVM_Brightness_first_count_reset(&brignessblance, 3);
	rgb_diff13[0] = 0; // left
	rgb_diff13[1] = 0;
	rgb_diff13[2] = 0;

	rgb_diff24[0] = 0; // right
	rgb_diff24[1] = 0;
	rgb_diff24[2] = 0;

	rgb_diff1_13[0] = 0; // front
	rgb_diff1_13[1] = 0;
	rgb_diff1_13[2] = 0;

	rgb_diff2_24[0] = 0;
	rgb_diff2_24[1] = 0;
	rgb_diff2_24[2] = 0;

	diff_L_y = 0.0f;
	diff_R_y = 0.0f;
	avg_L_y = 0.0f;
	avg_R_y = 0.0f;
	avg_FL_y = 0.0f;
	avg_FR_y = 0.0f;
	diff_FL_y = 0.0f;
	diff_FR_y = 0.0f;
	avg_BL_y = 0.0f;
	avg_BR_y = 0.0f;
	diff_BL_y = 0.0f;
	diff_BR_y = 0.0f;

	g_avmBBConfig.diff_part1[0] = 0.0f;
	g_avmBBConfig.diff_part1[1] = 0.0f;
	g_avmBBConfig.diff_part1[2] = 0.0f;
	g_avmBBConfig.diff_part1[3] = 0.0f;
	g_avmBBConfig.diff_part2[0] = 0.0f;
	g_avmBBConfig.diff_part2[1] = 0.0f;
	g_avmBBConfig.diff_part2[2] = 0.0f;
	g_avmBBConfig.diff_part2[3] = 0.0f;

	g_avmBBConfig.diff[0] = 0.0f;
	g_avmBBConfig.diff[1] = 0.0f;
	g_avmBBConfig.diff[2] = 0.0f;
	g_avmBBConfig.diff[3] = 0.0f;
	g_avmBBConfig.avg[0] = 0.0f;
	g_avmBBConfig.avg[1] = 0.0f;
	g_avmBBConfig.avg[2] = 0.0f;
	g_avmBBConfig.avg[3] = 0.0f;

	g_avmBBConfig.homo[0] = 0.0f;
	g_avmBBConfig.homo[1] = 0.0f;
	g_avmBBConfig.homo[2] = 0.0f;
	g_avmBBConfig.homo[3] = 0.0f;
	g_avmBBConfig.avg_part1[0] = 0.0f;
	g_avmBBConfig.avg_part1[1] = 0.0f;
	g_avmBBConfig.avg_part1[2] = 0.0f;
	g_avmBBConfig.avg_part1[3] = 0.0f;
	g_avmBBConfig.avg_part2[0] = 0.0f;
	g_avmBBConfig.avg_part2[1] = 0.0f;
	g_avmBBConfig.avg_part2[2] = 0.0f;
	g_avmBBConfig.avg_part2[3] = 0.0f;
	g_avmBBConfig.gammaL[0] = 0.0f;
	g_avmBBConfig.gammaL[1] = 0.0f;
	g_avmBBConfig.gammaL[2] = 0.0f;
	g_avmBBConfig.gammaL[3] = 0.0f;
	g_avmBBConfig.gammaR[0] = 0.0f;
	g_avmBBConfig.gammaR[1] = 0.0f;
	g_avmBBConfig.gammaR[2] = 0.0f;
	g_avmBBConfig.gammaR[3] = 0.0f;
}
void main_mem_malloc(void)
{
	y_list = (int *)malloc(sizeof(int) * 256);
	g_u_list = (int *)malloc(sizeof(int) * 256);
	g_v_list = (int *)malloc(sizeof(int) * 256);
	r_v_list = (int *)malloc(sizeof(int) * 256);
	b_u_list = (int *)malloc(sizeof(int) * 256);
	BY_yuvlist(y_list, g_u_list, g_v_list, r_v_list, b_u_list);
}
void caculate_color_offset(int isCameraFailure)
{

	if (isCameraFailure)
	{
		AVM_BB_RESET();
	}

	float FM = (float)rgb2yuvv(SUM_FM) / 255.0;
	float BM=  (float)rgb2yuvv(SUM_BM)/ 255.0;
	float RM = (float)rgb2yuvv(SUM_RM)/ 255.0;
	float LM = (float)rgb2yuvv(SUM_LM)/ 255.0;
	autosys_current_info* cur_info = &autosys.current_info;
	if(cur_info->BB_VERSION == 0)
	{
		avg_L_y  = (float)(rgb2yuvv(SUM_LF) + rgb2yuvv(SUM_LB)) / 2.0 / 255.0;
		avg_R_y  = (float)(rgb2yuvv(SUM_RF) + rgb2yuvv(SUM_RB)) / 2.0 / 255.0;

		avg_FL_y = (float)(rgb2yuvv(SUM_FL) + rgb2yuvv(SUM_LF)) / 2.0 / 255.0;
		avg_FR_y = (float)(rgb2yuvv(SUM_FR) + rgb2yuvv(SUM_RF)) / 2.0 / 255.0;

		avg_BL_y = (float)(rgb2yuvv(SUM_BL) + rgb2yuvv(SUM_LB)) / 2.0 / 255.0;
		avg_BR_y = (float)(rgb2yuvv(SUM_BR) + rgb2yuvv(SUM_RB)) / 2.0 / 255.0;




		float avg_total_y = (avg_L_y + avg_R_y + avg_FL_y + avg_FR_y + avg_BL_y + avg_BR_y + RM + BM + FM + LM) /10.0f;

		m_avm_BB_GAMMA_FLAG = 1;

		if(avg_total_y >= PARAM_AVM_BB_GAMMA_THRESHOLD) m_avm_BB_GAMMA_FLAG = 1;
		else m_avm_BB_GAMMA_FLAG = 0;


		VEC3_MINUS(SUM_Diff_LF_Minus_FL, SUM_LF, SUM_FL);
		VEC3_MINUS(SUM_Diff_RF_Minus_FR, SUM_RF, SUM_FR);
		VEC3_MINUS(SUM_Diff_LB_Minus_BL, SUM_LB, SUM_BL);
		VEC3_MINUS(SUM_Diff_RB_Minus_BR, SUM_RB, SUM_BR);

		VEC3_PLUS(RESULT_L,   SUM_Diff_LF_Minus_FL, SUM_Diff_LB_Minus_BL);
		VEC3_PLUS(RESULT_R,   SUM_Diff_RF_Minus_FR, SUM_Diff_RB_Minus_BR);
		VEC3_MINUS(RESULT_FL, SUM_Diff_LF_Minus_FL, SUM_Diff_LB_Minus_BL);
		VEC3_MINUS(RESULT_FR, SUM_Diff_RF_Minus_FR, SUM_Diff_RB_Minus_BR);

		VEC3_DIVI(RESULT_L, 2);
		VEC3_DIVI(RESULT_R, 2);
		VEC3_DIVI(RESULT_FR, 2);
		VEC3_DIVI(RESULT_FL, 2);

		m_HOMO_Lside_flag = 1;
	    m_HOMO_Rside_flag = 1;

	#if 1
	float birgY_L_DIFF = rgb2yuvv(RESULT_L);
    float birgY_R_DIFF = rgb2yuvv(RESULT_R);
    float brigY_L_TOP  = rgb2yuvv(SUM_LF) + birgY_L_DIFF;
    float brigY_L_DOWN = rgb2yuvv(SUM_LB) + birgY_L_DIFF;
    float brigY_L_MID  = rgb2yuvv(SUM_LM) + birgY_L_DIFF;
    float brigY_R_TOP  = rgb2yuvv(SUM_RF) + birgY_R_DIFF;
    float brigY_R_DOWN = rgb2yuvv(SUM_RB) + birgY_R_DIFF;
    float brigY_R_MID  = rgb2yuvv(SUM_RM) + birgY_R_DIFF;

    float brigY_R_TOP_RAW  = rgb2yuvv(SUM_RF);
    float brigY_R_DOWN_RAW = rgb2yuvv(SUM_RB);
    float brigY_L_TOP_RAW  = rgb2yuvv(SUM_LF);
    float brigY_L_DOWN_RAW = rgb2yuvv(SUM_LB);
    float brigY_FR_MID     = rgb2yuvv(SUM_FR);
    float brigY_FL_MID     = rgb2yuvv(SUM_FL);
    float brigY_BR_MID     = rgb2yuvv(SUM_BR);
    float brigY_BL_MID     = rgb2yuvv(SUM_BL);

	/* GAMMA*/
    float diff_tip_left_top   = ( brigY_FL_MID - brigY_L_TOP_RAW)  / 255.0f;
    float diff_tip_left_down  = ( brigY_BL_MID - brigY_L_DOWN_RAW) / 255.0f;

    float diff_tip_right_top  = ( brigY_FR_MID - brigY_R_TOP_RAW) / 255.0f;
    float diff_tip_right_down = ( brigY_BR_MID - brigY_R_DOWN_RAW) / 255.0f;

	m_tip_gamma_top[0] =(fabs(diff_tip_left_down)>0.2f)  ? 0.0f : -diff_tip_left_down;
	m_tip_gamma_down[0]=(fabs(diff_tip_left_top)>0.2f)   ? 0.0f : -diff_tip_left_top;
	m_tip_gamma_top[1] =(fabs(diff_tip_right_top)>0.2f)  ? 0.0f : -diff_tip_right_top;
	m_tip_gamma_down[1]=(fabs(diff_tip_right_down)>0.2f) ? 0.0f : -diff_tip_right_down;
	/* X- RATIO */
	float diff_homo_left_top = 0.0f;
	float diff_homo_right_top = 0.0f;

	if (fabs(brigY_L_MID) < EPSILON) // 替換原本的區間檢查，更安全
	{
		m_tip_homo_top[0] = 0.0f;
		diff_homo_left_top = 0.0f;
	}
	else
	{
		diff_homo_left_top = ((brigY_L_TOP + brigY_L_DOWN) / 2.0f - brigY_L_MID) / brigY_L_MID;
		m_tip_homo_top[0] = (diff_homo_left_top < -0.5f) ? -0.5f : diff_homo_left_top;
	}

	if (fabs(brigY_R_MID) < EPSILON)
	{
		m_tip_homo_top[1] = 0.0f;
		diff_homo_right_top = 0.0f;
	}
	else
	{
		diff_homo_right_top = ((brigY_R_TOP + brigY_R_DOWN) / 2.0f - brigY_R_MID) / brigY_R_MID;
		m_tip_homo_top[1] = (diff_homo_right_top < -0.5f) ? -0.5f : diff_homo_right_top;
	}


    if (birgY_L_DIFF < 0.0f || diff_homo_left_top > 0.0f)//(brigY_L_TOP+ brigY_L_DOWN)/2.0f > brigY_L_MID)
    {
		m_tip_homo_top[0] = 0.0f;
        m_HOMO_Lside_flag = 0;
    }
    else
    {
        if ((brigY_L_TOP > brigY_L_MID * 1.5f) || (brigY_L_TOP < brigY_L_MID * 0.5f))
        {
           m_tip_homo_top[0] = 0.0f;
        }
        if ((brigY_L_DOWN > brigY_L_MID * 1.5f) || (brigY_L_DOWN < brigY_L_MID * 0.5f))
        {
          m_tip_homo_top[0] = 0.0f;
        }
		if((brigY_L_TOP_RAW / brigY_FL_MID) > 1.5f)
        {
            m_tip_homo_top[0] = 0.0f;
        }
        if ((brigY_L_DOWN_RAW / brigY_BL_MID) > 1.5f)
        {
           m_tip_homo_top[0] = 0.0f;
        }
    }

    if (birgY_R_DIFF < 0.0f || diff_homo_right_top > 0.0f ) //(brigY_R_TOP + brigY_R_DOWN) / 2.0f > brigY_R_MID)
    {
        m_tip_homo_top[1]  = 0.0f;
    }
    else
    {
        if ( (brigY_R_TOP > brigY_R_MID*1.5f) || (brigY_R_TOP < brigY_R_MID * 0.5f))
        {
            m_tip_homo_top[1]  = 0.0f;
        }
        if ((brigY_R_DOWN > brigY_R_MID * 1.5f) || (brigY_R_DOWN < brigY_R_MID * 0.5f))
        {
            m_tip_homo_top[1]  = 0.0f;
        }

		if ((brigY_R_TOP_RAW / brigY_FR_MID) > 1.5f)
        {
            m_tip_homo_top[1]  = 0.0f;
        }
        if ((brigY_R_DOWN_RAW / brigY_BR_MID) > 1.5f)
        {
            m_tip_homo_top[1]  = 0.0f;
        }
    }
	// if(dark_count++ % 120 == 0)
	// {
	// 	printf("TESTpara [%f], [%f], [%f]\n", brigY_L_TOP, brigY_L_DOWN,brigY_L_MID);
	// }
#endif
	}
	else
	{
		float sum_fl  = colorData.block_y_value.SUM_FL;
		float sum_fr  = colorData.block_y_value.SUM_FR;
		float sum_bl  = colorData.block_y_value.SUM_BL;
		float sum_br  = colorData.block_y_value.SUM_BR;
		float sum_lf  = colorData.block_y_value.SUM_LF;
		float sum_lb  = colorData.block_y_value.SUM_LB;
		float sum_rf  = colorData.block_y_value.SUM_RF;
		float sum_rb  = colorData.block_y_value.SUM_RB;
		//avg_L_y  = (float)(rgb2yuvv(SUM_LF) + rgb2yuvv(SUM_LB)) / 2.0 / 255.0;
		avg_FL_y = (float)(sum_fl + sum_lf) / 2.0 / 255.0;
		avg_FR_y = (float)(sum_fr + sum_rf) / 2.0 / 255.0;
		avg_BL_y = (float)(sum_bl + sum_lb) / 2.0 / 255.0;
		avg_BR_y = (float)(sum_br + sum_rb) / 2.0 / 255.0;

		avg_LF_y = avg_FL_y;
		avg_LB_y = avg_FR_y;
		avg_RF_y = avg_BL_y;
		avg_RB_y = avg_BR_y;

		float diffOver_FL = (float)(sum_lf - sum_fl) / 255.0;
		float diffOver_FR = (float)(sum_rf - sum_fr) / 255.0;
		float diffOver_BL = (float)(sum_lb - sum_bl) / 255.0;
		float diffOver_BR = (float)(sum_rb - sum_br) / 255.0;
		float diffOver_LF = (float)(sum_fl - sum_lf) / 255.0;
		float diffOver_LB = (float)(sum_bl - sum_lb) / 255.0;
		float diffOver_RF = (float)(sum_fr - sum_rf) / 255.0;
		float diffOver_RB = (float)(sum_br - sum_rb) / 255.0;
		//  計算差距-左右
		float diffRF = diffOver_RF / 2.0f;
		float diffRB = diffOver_RB / 2.0f;
		float diffLF = diffOver_LF / 2.0f;
		float diffLB = diffOver_LB / 2.0f;
		// 計算差距-前後
		float diffFL = diffOver_FL / 2.0f;
		float diffFR = diffOver_FR / 2.0f;
		float diffBR = diffOver_BR / 2.0f;
		float diffBL = diffOver_BL / 2.0f;

		colorData.diff_value.diffFL = diffFL;
		colorData.diff_value.diffFR = diffFR;
		colorData.diff_value.diffBL = diffBL;
		colorData.diff_value.diffBR = diffBR;
		colorData.diff_value.diffLF = diffLF;
		colorData.diff_value.diffLB = diffLB;
		colorData.diff_value.diffRF = diffRF;
		colorData.diff_value.diffRB = diffRB;

        colorObj *pd = &colorData;
        vignetteConfig *pv = &vignetteData;
		pd->gamma_value.Y_gamma_FL = calculate_vignette_ratio(
            colorData.vignette.Y_vignette_Cam_1[0], sum_fl, pv[0].nor_value, colorData.gamma_value.Y_gamma_FL);
		pd->gamma_value.Y_gamma_FR = calculate_vignette_ratio(pd->vignette.Y_vignette_Cam_2[0], sum_fr, pv[0].nor_value,pd->gamma_value.Y_gamma_FR);
		pd->gamma_value.Y_gamma_BL = calculate_vignette_ratio(pd->vignette.Y_vignette_Cam_1[1], sum_bl, pv[1].nor_value,pd->gamma_value.Y_gamma_BL);
		pd->gamma_value.Y_gamma_BR = calculate_vignette_ratio(pd->vignette.Y_vignette_Cam_2[1], sum_br, pv[1].nor_value,pd->gamma_value.Y_gamma_BR);

		pd->gamma_value.Y_gamma_LL = calculate_vignette_ratio(pd->vignette.Y_vignette_Cam_1[2], sum_lf, pv[2].nor_value,pd->gamma_value.Y_gamma_LL);
		pd->gamma_value.Y_gamma_LR = calculate_vignette_ratio(pd->vignette.Y_vignette_Cam_2[2], sum_lb, pv[2].nor_value,pd->gamma_value.Y_gamma_LR);
		pd->gamma_value.Y_gamma_RL = calculate_vignette_ratio(pd->vignette.Y_vignette_Cam_1[3], sum_rf, pv[3].nor_value,pd->gamma_value.Y_gamma_RL);
		pd->gamma_value.Y_gamma_RR = calculate_vignette_ratio(pd->vignette.Y_vignette_Cam_2[3], sum_rb, pv[3].nor_value,pd->gamma_value.Y_gamma_RR);

        g_avmBBConfig.gamma_angle[0] =    vignetteData[0].angle;
        g_avmBBConfig.gamma_angle[1] =    vignetteData[1].angle;
        g_avmBBConfig.gamma_angle[2] =    vignetteData[2].angle;
        g_avmBBConfig.gamma_angle[3] =    vignetteData[3].angle;
	}

	SAFE_VAL(m_tip_homo_top[0], 0.0f);
	SAFE_VAL(m_tip_homo_top[1], 0.0f);
}


void Read_file_yuv_preloading()
{
    #ifdef READ_FILE
    static int m_write_count = 0;
    FILE *	fpFront;
    FILE *	fpBack;
    FILE *	fpLeft;
    FILE *	fpRight;
    
    switch(m_write_count)
    {
        case 0:
        fpFront  = fopen("./EVSTable/input_img/front.yuv","rb");
        fread(m_cameraFileFront, 1, imageWidth * imageHeight * 2, fpFront);
        fclose(fpFront);
        break;
        case 1:
        fpBack   = fopen("./EVSTable/input_img/back.yuv", "rb");
        fread(m_cameraFileBack, 1, imageWidth * imageHeight * 2, fpBack);
        fclose(fpBack);
        break;
        case 2:
        fpLeft   = fopen("./EVSTable/input_img/left.yuv" , "rb");
        fread(m_cameraFileLeft, 1, imageWidth * imageHeight * 2, fpLeft);
        fclose(fpLeft);
        break;
        case 3:
        fpRight  = fopen("./EVSTable/input_img/right.yuv", "rb");
        fread(m_cameraFileRight, 1, imageWidth * imageHeight * 2, fpRight);
        fclose(fpRight);
        break;
        default:
        break;
    }
    m_write_count++;
    #endif
}
void calculate_color(void)
{
	int j, i;
	volatile int isCameraFailure = 0;

    Read_file_yuv_preloading();
    #ifdef READ_FILE
        unsigned char *cambuf[] = {m_cameraFileFront, m_cameraFileBack, m_cameraFileLeft, m_cameraFileRight};
    #else
        unsigned char *cambuf[] = {m_cameraFileFrontBuf, m_cameraFileBackBuf, m_cameraFileLeftBuf, m_cameraFileRightBuf};
    #endif
	//if (color_enable == 1)
	{
		int part;
		for (part = 0; part < 12; part++)
		{
			unsigned char *avmcamera;
			int side = 0;
			int view = 0;
			int *p_sumY;
			int *colorblk, *rtotal, *gtotal, *btotal;
			switch (part)
			{
			case 0: // 前左
				avmcamera = cambuf[0];
				colorblk  = m_colorblk0;
				rtotal = &SUM_FL.X;
				gtotal = &SUM_FL.Y;
				btotal = &SUM_FL.Z;
				p_sumY = &colorData.block_y_value.SUM_FL;
				side = 0;
				view = 0;
				break;
			case 1: // 前右
				avmcamera = cambuf[0];
				colorblk = m_colorblk1;
				rtotal = &SUM_FR.X;
				gtotal = &SUM_FR.Y;
				btotal = &SUM_FR.Z;
				p_sumY = &colorData.block_y_value.SUM_FR;
				side = 1;
				view = 0;
				break;
			case 2: // 後左
				avmcamera = cambuf[1];
				colorblk = m_colorblk2;
				rtotal = &SUM_BL.X;
				gtotal = &SUM_BL.Y;
				btotal = &SUM_BL.Z;
				p_sumY = &colorData.block_y_value.SUM_BL;
				side = 0;
				view = 1;
				break;
			case 3: // 後右
				avmcamera = cambuf[1];
				colorblk = m_colorblk3;
				rtotal = &SUM_BR.X;
				gtotal = &SUM_BR.Y;
				btotal = &SUM_BR.Z;
				p_sumY = &colorData.block_y_value.SUM_BR;
				side = 1;
				view = 1;
				break;
			case 4: // 左前
				avmcamera = cambuf[2];
				colorblk = m_colorblk4;
				rtotal = &SUM_LF.X;
				gtotal = &SUM_LF.Y;
				btotal = &SUM_LF.Z;
				p_sumY = &colorData.block_y_value.SUM_LF;
				side = 0;
				view = 2;
				break;
			case 5: // 左後
				avmcamera = cambuf[2];
				colorblk = m_colorblk5;
				rtotal = &SUM_LB.X;
				gtotal = &SUM_LB.Y;
				btotal = &SUM_LB.Z;
				p_sumY = &colorData.block_y_value.SUM_LB;
				side = 1;
				view = 2;
				break;
			case 6: // 右前
				avmcamera = cambuf[3];
				colorblk = m_colorblk6;
				rtotal = &SUM_RF.X;
				gtotal = &SUM_RF.Y;
				btotal = &SUM_RF.Z;
				p_sumY = &colorData.block_y_value.SUM_RF;
				side = 0;
				view = 3;
				break;
			case 7: // 右後
				avmcamera = cambuf[3];
				colorblk = m_colorblk7;
				rtotal = &SUM_RB.X;
				gtotal = &SUM_RB.Y;
				btotal = &SUM_RB.Z;
				p_sumY = &colorData.block_y_value.SUM_RB;
				side = 1;
				view = 3;
				break;
			case 8: // 前中
				avmcamera = cambuf[0];
				colorblk = m_colorblk_mid0;
				rtotal = &SUM_FM.X;
				gtotal = &SUM_FM.Y;
				btotal = &SUM_FM.Z;
				p_sumY = &colorData.block_y_value.SUM_FM;
				side = 2;
				view = 0;

				break;
			case 9: // 前中
				avmcamera = cambuf[1];
				colorblk = m_colorblk_mid1;
				rtotal = &SUM_BM.X;
				gtotal = &SUM_BM.Y;
				btotal = &SUM_BM.Z;
				p_sumY = &colorData.block_y_value.SUM_BM;
				side = 2;
				view = 1;
				break;
			case 10: // 前中
				avmcamera = cambuf[2];
				colorblk = m_colorblk_mid2;
				rtotal = &SUM_LM.X;
				gtotal = &SUM_LM.Y;
				btotal = &SUM_LM.Z;
				p_sumY = &colorData.block_y_value.SUM_LM;
				side = 2;
				view = 2;
				break;
			case 11: // 前中
				avmcamera = cambuf[3];
				colorblk = m_colorblk_mid3;
				rtotal = &SUM_RM.X;
				gtotal = &SUM_RM.Y;
				btotal = &SUM_RM.Z;
				p_sumY = &colorData.block_y_value.SUM_RM;
				side = 2;
				view = 3;
				break;

			default:
				break;
			}

			int ptr;
			int sumY = 0, sumU = 0, sumV = 0;
			int total_effect_count = 0;
            int max_colorblk_y = 0.0f;
            int min_colorblk_y = 300.0f;
            int avg_colorblk_y = 0.0f;

			int avg_warning_count = 0;
			

			unsigned char yuv_list_patern[3000];
			int sum_yuv_list = 0;
			int mean = 0;
			for(ptr = 0; ptr < m_total_ov_area; ptr++)
			{
				unsigned char tmpy, tmpu, tmpv, tmpr, tmpg, tmpb;
				int i, j, ptry, ptru, ptrv;

				i = colorblk[2 * ptr];
				j = colorblk[2 * ptr + 1];
				ptry = 2 * (i + j * imageWidth);
				tmpy = avmcamera[ptry];
				if(ptr < 3000)
				{
					yuv_list_patern[ptr] = tmpy;
					sum_yuv_list += (int)tmpy;
				}
			}

			mean = sum_yuv_list / m_total_ov_area;
			double sumOfSquares = 0.0;
    		for(int i = 0; i < m_total_ov_area; i++) 
			{
        		double diff = (double)((int)yuv_list_patern[i] - mean);
        		sumOfSquares += diff * diff;
    		}
    		double variance = sumOfSquares / (double)m_total_ov_area;
    		double stdDev = sqrt(variance);
			
			//unsigned char upper_limit = (unsigned char )mean + (unsigned char )(2.0 * stdDev);
			unsigned char upper_limit = ((double)mean + (2.0 * stdDev) < 255.0)?(unsigned char )mean + (unsigned char )(2.0 * stdDev):255;
			unsigned char lower_limit = ((double)mean > (2.0 * stdDev))?(unsigned char )mean - (unsigned char )(2.0 * stdDev):0;

			// if(g_frame % 10 == 0)
			// {
			// 	AVM_LOGI("part = %d, upper_limit = %d, lower_limit = %d\n", part, upper_limit, lower_limit);
			// }

			for (ptr = 0; ptr < m_total_ov_area; ptr++)
			{
				unsigned char tmpy, tmpu, tmpv, tmpr, tmpg, tmpb;
				int i, j, ptry, ptru, ptrv;

				i = colorblk[2 * ptr];
				j = colorblk[2 * ptr + 1];

#if (FORMAT == YUYV)
				// yuyv
				ptry = 2 * (i + j * imageWidth);
				ptru = 2 * ((i / 2 * 2) + j * imageWidth) + 1;
				ptrv = ptru + 2;
#elif (FORMAT == UYVY)
				// uyvy
				ptry = 2 * (i + j * imageWidth) + 1;
				ptru = 2 * ((i / 2 * 2) + j * imageWidth);
				ptrv = ptru + 2;
#endif

				tmpy = avmcamera[ptry];
				tmpu = avmcamera[ptru];
				tmpv = avmcamera[ptrv];

#if PARAM_AVM_BB_LIMIT_FILTER_FUNCTION
				if (tmpy > upper_limit || tmpy < lower_limit) continue;
				//if (tmpy < 15) continue;
				if (tmpy > 200) avg_warning_count++;
#endif

				sumY += tmpy;
				sumU += tmpu;
				sumV += tmpv;

				avg_colorblk_y += (int)tmpy;
                if (max_colorblk_y < (int)tmpy)
                {
                    max_colorblk_y = (int)tmpy;
                }
                if(min_colorblk_y > (int)tmpy)
                {
                    min_colorblk_y = (int)tmpy;
                }



				total_effect_count++;

				YUV2RGBnew(tmpy, tmpu, tmpv, &tmpr, &tmpg, &tmpb, y_list, r_v_list, g_u_list, g_v_list, b_u_list);
				*rtotal += tmpr;
				*gtotal += tmpg;
				*btotal += tmpb;
			}

			Calculate_Vignette_Value(avmcamera, view , side);

			if(total_effect_count > 0)
			{
				sumY /= total_effect_count;
				sumU /= total_effect_count;
				sumV /= total_effect_count;

			}


			int count_filter = 0;

			if(total_effect_count > 20)
			{
				*rtotal /= (total_effect_count);
				*gtotal /= (total_effect_count);
				*btotal /= (total_effect_count);
				*p_sumY = sumY;
			}
			else
			{
				unsigned char tmpr, tmpg, tmpb;
				YUV2RGBnew((unsigned char)sumY, (unsigned char)sumU, (unsigned char)sumV,  &tmpr, &tmpg, &tmpb, y_list, r_v_list, g_u_list, g_v_list, b_u_list);
				*rtotal = tmpr;
				*gtotal = tmpg;
				*btotal = tmpb;
				*p_sumY = sumY;
			}


			// printf("\033[31m part = %d\t sumY = %d \033[0m\n", part, sumY);
			// printf("\033[31m part = %d\t sumU = %d \033[0m\n", part, sumU);
			// printf("\033[31m part = %d\t sumV = %d \033[0m\n", part, sumV);

			if (abs(sumY - 28) <= 2 &&
				abs(sumU - 212) <= 2 &&
				abs(sumV - 120) <= 2)
			{

				isCameraFailure = 1;
			}
		}
	}

	// printf("isCameraFailure = %d\n",isCameraFailure);
	caculate_color_offset(isCameraFailure);
	PID_BB_FLOW();
}
#pragma endregion