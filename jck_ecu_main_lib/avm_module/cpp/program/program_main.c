/**
 *******************************************************************************
 * @file : program_main.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211116 0.1.0 Linda.
 * @date 20211222 0.1.1 Linda.
 *******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../../system.h"
#include "user_control/ui.h"
#include "avm/GLHeader.h"

#include "program/program_main.h"
#include "avm/fp_source.h"
#include "avm/maths.h"
#include "avm/bb_func.h"
#include <math.h>
#include "gl/glShaderUtils.h"
#include FRAGSHADER
#include VERTSHADER

#include "../camera.h"
// #define STB_IMAGE_IMPLEMENTATION
#include "avm/stb_image.h"
#include "../../autosys/autosys.h"
#include "avm/avm_support_funcs.h"
#include <EGL/egl.h>
#include <GLES2/gl2ext.h>
// //struct timeval start1, end1;

static unsigned char *cameraFileADASIMS; // IMS AND ADAS

static GLuint programHandleMain[2];
static GLuint programHandleCarShadow; // test_shadow

static GLuint VAO[VAO_NUM];
static GLuint VAOView2d[VAO_NUM];
static GLuint VAOViewCorrect2d[20];
static GLuint VAOViewUnique2d[VAO_NUM];
static GLuint VAOViewAdas[VAO_NUM];
static GLuint VAOCarShadow[5]; // test_shadow
static GLuint VAOCameraWarning[VAO_NUM];


static GLuint VBO_3davm[4][3]; // 0:front 1:back 2:left 3:right		0:vertex 1:texture 2:alpha

static GLuint VBO_view2d[2][2];
static GLuint VBO_alley2d[1][2];
static GLuint VBO_back2d[1][2];

static GLuint VBO_correct[20][2];

static GLuint VBO_tire[2][2];

static GLuint VBO_bsd[2][2];

static GLuint VBO_side[2][2];
static GLuint VBO_stopngo[1][2];
static GLuint VBOCameraWarning[4][2];

GLuint VBOCarShadow[5][3]; // test_shadow

static GLint glAttr3DVertexFB = 0;
static GLint glAttr3DTextureFB = 0;
static GLint glAttralphaFB = 0;

static GLint glAttr3DVertexLR = 0;
static GLint glAttr3DTextureLR = 0;
static GLint glAttralphaLR = 0;

static GLint glAttrShadowVertex = 0;
static GLint glAttrShadowTexture = 0;
static GLint glAttrShadowalpha = 0;

static unsigned char *cameraFileFront;
static unsigned char *cameraFileBack;
static unsigned char *cameraFileLeft;
static unsigned char *cameraFileRight;
static unsigned char *cameraFileLeftBSD;
static unsigned char *cameraFileRightBSD;

static unsigned char *cameraFileFrontBuf;
static unsigned char *cameraFileBackBuf;
static unsigned char *cameraFileLeftBuf;
static unsigned char *cameraFileRightBuf;
static unsigned char *cameraFileLeftBSDBuf;
static unsigned char *cameraFileRightBSDBuf;

static unsigned int cameraFBufPA;
static unsigned int cameraBBufPA;
static unsigned int cameraLBufPA;
static unsigned int cameraRBufPA;

static GLint glUni_SamplerYFront;
static GLint glUni_SamplerYBack;
static GLint glUni_SamplerYLeft;
static GLint glUni_SamplerYRight;
static GLint glUni_SamplerView;

static float *vertices_3D_model_info;
static int F_index_num;
static int avm2d_vert_num;

static float *vertices_3d_f;
static float *vertices_3d_b;
static float *vertices_3d_l;
static float *vertices_3d_r;

static int index_num_f;
static int index_num_b;
static int index_num_l;
static int index_num_r;
static int delay_loading = 0;

static float *texture_3d_f;
static float *texture_3d_b;
static float *texture_3d_l;
static float *texture_3d_r;

static int *colorblk0;
static int *colorblk1;
static int *colorblk2;
static int *colorblk3;
static int *colorblk4;
static int *colorblk5;
static int *colorblk6;
static int *colorblk7;
static int *colorblk_mid0;
static int *colorblk_mid1;
static int *colorblk_mid2;
static int *colorblk_mid3;

static float *texture_alpha_f;
static float *texture_alpha_b;

static float *Front_texture;
static float *Back_texture;
static float *Left_texture;
static float *Right_texture;
static float *texture_alpha;

static int total_ov_area;
static int *colorblkArray;

gl_coord_t view2d[4][6];
gl_coord_3d_t avm3D[4];
gl_coord_3d_t carShadow; // test_shadow
static float *vertices_carBottom;

static GLuint VAO_Cylindrical2d[VAO_NUM];
static GLuint VBO_Cylindrical2d[6][2];
gl_coord_t Cylindrical2d[6];
static unsigned char *cameraFileAll;

float chassisColor[3] = {0.3, 0.3, 0.3};

static int y_avg;

static int aftercolor[36];



static GLuint g_texYId[10];
static GLuint g_tex_alpha[1];
static GLuint g_tex_color[1];
static GLuint g_tex_color2[1];
static GLuint g_camera_warning[4];
static GLuint g_shadowImgId[2];



static GLint glUni_Alphamap;
static GLint glUni_colormap;
static GLint glUni_colormap2;
static GLint glUni_SamplerY;

static unsigned char *CarShadowBuf;
static GLuint g_CarShadowtexId[2];

static GLuint g_UI_enable_texId[10];
static GLuint g_UI_disable_texId[10];

static GLint glUni_matrixLocation;
static GLint glUni_shadow_matrixLocation;
static GLint glCarShadowWindow;
static GLint CarShadowBlurFlag;

unsigned char *AVM_BOWL_alpha_data1 ;
unsigned char *AVM_BOWL_alpha_data2 ;
unsigned char *AVM_BOWL_alpha_data3 ; //color_l_r alphamap
int AVM_BOWL_alpha_data1_w1,AVM_BOWL_alpha_data1_h1,AVM_BOWL_alpha_data1_n1;
int AVM_BOWL_alpha_data1_w2,AVM_BOWL_alpha_data1_h2,AVM_BOWL_alpha_data1_n2;
int AVM_BOWL_alpha_data1_w3,AVM_BOWL_alpha_data1_h3,AVM_BOWL_alpha_data1_n3;
/**
 * @brief :  Adjustment avm brightness
 * @author : Boyuchen
 * @date : 2024/03/26
 */
int overlay_warning_history[12][3]= {0};
/*Sceneview flag*/
int flag_sceneview_updata_count[4] = {-1,-1,-1,-1};

// PFNGLTEXDIRECTVIVMAPPROC glTexDirectVIVMap = NULL;
// PFNGLTEXDIRECTINVALIDATEVIVPROC glTexDirectInvalidateVIV = NULL;

void CalcFisheyeMatrices(int cam, float* rotateReslut, cropped_view* cropped, 
float* transferM,float* scaleM, float* transM, int *switchPos);
void AVM_BB_RESET();
//======================================================================================================================================================================================================
//======================================================================================================================================================================================================

void alloc_cam_buf(void)
{	
	cameraFileFrontBuf = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileBackBuf = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileLeftBuf = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileRightBuf = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileLeftBSDBuf = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileRightBSDBuf = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));

	trans_cam_buffer_to_bb(cameraFileFrontBuf,cameraFileBackBuf,cameraFileLeftBuf,cameraFileRightBuf);
}

void alloc_cam_one_buffer()
{
	fp_read_image_avm("EVSTable/avm/alpha.png",     &AVM_BOWL_alpha_data1, &AVM_BOWL_alpha_data1_w1, &AVM_BOWL_alpha_data1_h1, &AVM_BOWL_alpha_data1_n1, 3,0);
	fp_read_image_avm("EVSTable/avm/colorOut1.png", &AVM_BOWL_alpha_data2, &AVM_BOWL_alpha_data1_w2, &AVM_BOWL_alpha_data1_h2, &AVM_BOWL_alpha_data1_n2, 3,0);
	fp_read_image_avm("EVSTable/avm/colorOut2.png", &AVM_BOWL_alpha_data3, &AVM_BOWL_alpha_data1_w3, &AVM_BOWL_alpha_data1_h3, &AVM_BOWL_alpha_data1_n3, 3,0);
}

void init_main_program(void)
{
	AVM_LOGI("-----------------------------\n");
	#if READ_OFFLINE_SHADER
	LoadBinary2Program(&programHandleMain[0],"shader_program.bin_format_36805_4");
	LoadBinary2Program(&programHandleMain[1],"shader_program.bin_format_36805_5");
	LoadBinary2Program(&programHandleCarShadow,"shader_program.bin_format_36805_6");
	if(programHandleMain[1] == GL_NONE || programHandleMain[0] == GL_NONE || programHandleCarShadow == GL_NONE)
	{
		AVM_LOGI(LOG_RED("[ERROR]")"GL Loading Main offline file error!!!\n");
		AVM_LOGI("[WARNING] Using online program!!!\n");
		programHandleMain[0]   = create_program(vertexShader_main_fb, fragmentShader_main_fb,4); // 3D AVM
		programHandleMain[1]   = create_program(vertexShader_main_lr, fragmentShader_main_lr,5); // 3D AVM
		programHandleCarShadow = create_program(vertexShader_car_shadow, fragmentShader_car_shadow,6); // 3D AVM
	}

	#else
		programHandleMain[0] = create_program(vertexShader_main_fb, fragmentShader_main_fb,4); // 3D AVM
		programHandleMain[1] = create_program(vertexShader_main_lr, fragmentShader_main_lr,5); // 3D AVM
		programHandleCarShadow = create_program(vertexShader_car_shadow, fragmentShader_car_shadow,6); // 3D AVM
		write_offline_shader_program(programHandleMain[0], 4);
		write_offline_shader_program(programHandleMain[1], 5);
		write_offline_shader_program(programHandleCarShadow, 6);
	#endif

	AVM_LOGI("AVM program_Main[0]: %d \n", programHandleMain[0]);
	AVM_LOGI("AVM program_Main[1]: %d \n", programHandleMain[1]);
	AVM_LOGI("AVM program_CarShadow[0]: %d \n", programHandleCarShadow);
	AVM_LOGI("-----------------------------\n");
}

void init_static_variable(void)
{
	flag_sceneview_updata_count[0] = -1;
	flag_sceneview_updata_count[1] = -1;
	flag_sceneview_updata_count[2] = -1;
	flag_sceneview_updata_count[3] = -1;
}


void get_cam_va_from_8368_p(const unsigned char *cameraBufF, const unsigned char *cameraBufB, const unsigned char *cameraBufL, const unsigned char *cameraBufR)
{
	#ifndef READ_FILE
	cameraFileFrontBuf = cameraBufF;
	cameraFileBackBuf = cameraBufB;
	cameraFileLeftBuf = cameraBufL;
	cameraFileRightBuf = cameraBufR;
	trans_cam_buffer_to_bb(cameraFileFrontBuf,cameraFileBackBuf,cameraFileLeftBuf,cameraFileRightBuf);
	#else
	// cameraFileFrontBuf = cameraFileFront;
	// cameraFileBackBuf  = cameraFileBack;
	// cameraFileLeftBuf = cameraFileLeft;
	// cameraFileRightBuf = cameraFileRight;
	#endif
}

void get_cam_pa_from_8368_p(unsigned int cameraPAF, unsigned int cameraPAB, unsigned int cameraPAL, unsigned int cameraPAR)
{
	cameraFBufPA = cameraPAF;
	cameraBBufPA = cameraPAB;
	cameraLBufPA = cameraPAL;
	cameraRBufPA = cameraPAR;
}

void pass_campa(unsigned int cameraPAF, unsigned int cameraPAB, unsigned int cameraPAL, unsigned int cameraPAR)
{
	get_cam_pa_from_8368_p(cameraPAF, cameraPAB, cameraPAL, cameraPAR);
}

void read_camera_file(void)
{
	FILE *fpFront;
	FILE *fpBack;
	FILE *fpLeft;
	FILE *fpRight;
	FILE *fpLeftBSD;
	FILE *fpRightBSD;

	// fpFront = fp_source_rb("/media/flash/userdata/EVSTable/input_img/front.yuv");
	// fpBack = fp_source_rb("/media/flash/userdata/EVSTable/input_img/back.yuv");
	// fpLeft = fp_source_rb("/media/flash/userdata/EVSTable/input_img/left.yuv");
	// fpRight = fp_source_rb("/media/flash/userdata/EVSTable/input_img/right.yuv");

	alloc_cam_buf();

	#if 1

	glGenTextures(1, &g_texYId[0]);
	glGenTextures(1, &g_texYId[1]);
	glGenTextures(1, &g_texYId[2]);
	glGenTextures(1, &g_texYId[3]);

    int w1,h1,n;
    stbi_set_flip_vertically_on_load(0);
    unsigned char *data1 = stbi_load("./EVSTable/input_img/front.png", &w1, &h1, &n, 0);
    unsigned char *data2 = stbi_load("./EVSTable/input_img/back.png", &w1, &h1, &n, 0);
    unsigned char *data3 = stbi_load("./EVSTable/input_img/left.png", &w1, &h1, &n, 0);
    unsigned char *data4 = stbi_load("./EVSTable/input_img/right.png", &w1, &h1, &n, 0);

	cameraFileFront = (unsigned char *)calloc(w1 * h1 * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileBack  = (unsigned char *)calloc(w1 * h1 * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileLeft  = (unsigned char *)calloc(w1 * h1 * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileRight = (unsigned char *)calloc(w1 * h1 * 2 * sizeof(unsigned char), sizeof(unsigned char));
	trans_ReadFile_buffer_to_bb(cameraFileFront,cameraFileBack,cameraFileLeft,cameraFileRight);
AVM_LOGI("INIT Start\n");
	glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(data1);

	glActiveTexture(GL_TEXTURE0 + BACK_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(data2);

	glActiveTexture(GL_TEXTURE0 + LEFT_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data3);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(data3);

	glActiveTexture(GL_TEXTURE0 + RIGHT_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(data4);

	AVM_LOGI("INIT OKOKOKOKOKOKOKO\n");
#endif


	// FILE * fp_camera_front;
	// 	FILE * fp_camera_back;
	// 	FILE * fp_camera_left;
	// 	FILE * fp_camera_right;
	// 	fp_camera_front = fopen("camera_front.yuv", "w");
	// 	fp_camera_back = fopen("camera_back.yuv", "w");
	// 	fp_camera_left = fopen("camera_left.yuv", "w");
	// 	fp_camera_right = fopen("camera_right.yuv", "w");
	// 	fwrite(cameraFileFrontBuf,1,1280*720*2,fp_camera_front);
	// 	fwrite(cameraFileBackBuf,1,1280*720*2,fp_camera_back);
	// 	fwrite(cameraFileLeftBuf,1,1280*720*2,fp_camera_left);
	// 	fwrite(cameraFileRightBuf,1,1280*720*2,fp_camera_right);
	// 	fclose(fp_camera_front);
	// 	fclose(fp_camera_back);
	// 	fclose(fp_camera_left);
	// 	fclose(fp_camera_right);

	// cameraFileLeftBSD = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	// cameraFileRightBSD = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));

	// fread(cameraFileLeftBSD, 1, imageWidth * imageHeight * 2, fpLeftBSD);
	// fread(cameraFileRightBSD, 1, imageWidth * imageHeight * 2, fpRightBSD);

	// memcpy(cameraFileLeftBSDBuf, cameraFileLeftBSD, imageWidth * imageHeight * 2);
	// memcpy(cameraFileRightBSDBuf, cameraFileRightBSD, imageWidth * imageHeight * 2);

	// fclose(fpLeftBSD);
	// fclose(fpRightBSD);

#if (PLAFORM != IMAX8)
	//		free(cameraFileFront);
	//		free(cameraFileBack);
	//		free(cameraFileLeft);
	//		free(cameraFileRight);
	//		free(cameraFileLeftBSD);
	//		free(cameraFileRightBSD);
#endif

	// fpFront  = fopen("./EVSTable/input_img/front.yuv","rb");
	// fpBack   = fopen("./EVSTable/input_img/back.yuv", "rb");
	// fpLeft   = fopen("./EVSTable/input_img/left.yuv" , "rb");
	// fpRight  = fopen("./EVSTable/input_img/right.yuv", "rb");


	// cameraFileFront = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	// cameraFileBack  = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	// cameraFileLeft  = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	// cameraFileRight = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));

	// fread(cameraFileFront, 1, imageWidth * imageHeight * 2, fpFront);
	// fread(cameraFileBack, 1, imageWidth * imageHeight * 2, fpBack);
	// fread(cameraFileLeft, 1, imageWidth * imageHeight * 2, fpLeft);
	// fread(cameraFileRight, 1, imageWidth * imageHeight * 2, fpRight);

	// memcpy(cameraFileFrontBuf, cameraFileFront, imageWidth * imageHeight * 2);
	// memcpy(cameraFileBackBuf, cameraFileBack, imageWidth * imageHeight * 2);
	// memcpy(cameraFileLeftBuf, cameraFileLeft, imageWidth * imageHeight * 2);
	// memcpy(cameraFileRightBuf, cameraFileRight, imageWidth * imageHeight * 2);

	// fclose(fpFront);
	// fclose(fpBack);
	// fclose(fpLeft);
	// fclose(fpRight);

	AVM_LOGI("read_camera_file ok \n");
}

void read_camera_file_one(void)
{
	FILE *fpAll;

	fpAll = fp_source("./input_img/video0.yuv", "rb");

	cameraFileAll = (unsigned char *)calloc(imageWidth * imageHeight * 2 * 4 * sizeof(unsigned char), sizeof(unsigned char));

	cameraFileFront = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileBack = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileLeft = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileRight = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileLeftBSD = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));
	cameraFileRightBSD = (unsigned char *)calloc(imageWidth * imageHeight * 2 * sizeof(unsigned char), sizeof(unsigned char));

	alloc_cam_buf();

	fread(cameraFileAll, 1, imageWidth * imageHeight * 2 * 4, fpAll);

	memcpy(cameraFileFront, cameraFileAll, imageWidth * imageHeight * 2);
	memcpy(cameraFileBack, cameraFileAll + imageWidth * imageHeight * 2, imageWidth * imageHeight * 2);
	memcpy(cameraFileLeft, cameraFileAll + imageWidth * imageHeight * 2 * 2, imageWidth * imageHeight * 2);
	memcpy(cameraFileRight, cameraFileAll + imageWidth * imageHeight * 2 * 3, imageWidth * imageHeight * 2);

	memcpy(cameraFileFrontBuf, cameraFileFront, imageWidth * imageHeight * 2);
	memcpy(cameraFileBackBuf, cameraFileBack, imageWidth * imageHeight * 2);
	memcpy(cameraFileLeftBuf, cameraFileLeft, imageWidth * imageHeight * 2);
	memcpy(cameraFileRightBuf, cameraFileRight, imageWidth * imageHeight * 2);

	fclose(fpAll);

#if (PLAFORM != IMAX8)
	//		free(cameraFileFront);
	//		free(cameraFileBack);
	//		free(cameraFileLeft);
	//		free(cameraFileRight);
#endif

	AVM_LOGI("read_camera_file ok \n");
}

int get_fisheye_raw_textrueID(int cameraNumber)
{
	//glBindTexture(GL_TEXTURE_2D, g_texYId[cameraNumber]);
	return g_texYId[cameraNumber];
}
void read_vert_text_bin(const char *tarDir, gl_coord_t *coord)
{
	FILE *fpBin = fp_source_app(tarDir, "r");
	if(fpBin == NULL)
	{
		coord->num = 0;
		// AVM_LOGI(LOG_RED("[ERROR]")" read_vert_text_bin file error!!!\n");
		// AVM_LOGI("[WARNING] Using Default value!!!\n");
		// SET_STATUS_CALIBRATION_IVI(sceneView, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
		return;
	}

	int num = 0;
	fp_read_data_safty(&num, 1, 1 * sizeof(int), fpBin);
	coord->num = num;
	AVM_LOGI("index_num = %d\n", coord->num);

	coord->vertice = (float *)calloc(num * 3 * sizeof(float), sizeof(float));
	coord->texture = (float *)calloc(num * 2 * sizeof(float), sizeof(float));

	int index = (num - 1) * 3;
	fp_read_data_safty(coord->vertice, 1, num * 3 * sizeof(float), fpBin);
	AVM_LOGI("vertices[%d] = %f\t", index, (coord->vertice)[index]);
	AVM_LOGI("vertices[%d] = %f\t", index + 1, (coord->vertice)[index + 1]);
	AVM_LOGI("vertices[%d] = %f\n", index + 2, (coord->vertice)[index + 2]);

	index = (num - 1) * 2;
	fp_read_data_safty(coord->texture, 1, num * 2 * sizeof(float), fpBin);
	AVM_LOGI("texture[%d] = %f\t", index, (coord->texture)[index]);
	AVM_LOGI("texture[%d] = %f\n", index + 1, (coord->texture)[index + 1]);

	fp_close_data_safty(fpBin);
}

void read_scene_vert_text_bin_wCRC(const char *tarDir, gl_coord_t *coord)
{
	FILE *fpBin = fp_source_avm(tarDir, "r");
	if(fpBin == NULL)
	{
		coord->num = 0;
		// AVM_LOGI(LOG_RED("[ERROR]")" read_vert_text_bin file error!!!\n");
		// AVM_LOGI("[WARNING] Using Default value!!!\n");
		// SET_STATUS_CALIBRATION_IVI(sceneView, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
		return;
	}

	int num = 0;
	int empty = 0;
	#ifdef __CRC_SYSTEM_WRITE__
		fp_read_data_safty(&empty, 1, 1 * sizeof(int), fpBin);
	#endif
	fp_read_data_safty(&num, 1, 1 * sizeof(int), fpBin);
	coord->num = num;
	AVM_LOGI("index_num = %d\n", coord->num);

	coord->vertice = (float *)calloc(num * 3 * sizeof(float), sizeof(float));
	coord->texture = (float *)calloc(num * 2 * sizeof(float), sizeof(float));

	int index = (num - 1) * 3;
	fp_read_data_safty(coord->vertice, 1, num * 3 * sizeof(float), fpBin);
	AVM_LOGI("vertices[%d] = %f\t", index, (coord->vertice)[index]);
	AVM_LOGI("vertices[%d] = %f\t", index + 1, (coord->vertice)[index + 1]);
	AVM_LOGI("vertices[%d] = %f\n", index + 2, (coord->vertice)[index + 2]);

	index = (num - 1) * 2;
	fp_read_data_safty(coord->texture, 1, num * 2 * sizeof(float), fpBin);
	AVM_LOGI("texture[%d] = %f\t", index, (coord->texture)[index]);
	AVM_LOGI("texture[%d] = %f\n", index + 1, (coord->texture)[index + 1]);

	fp_close_data_safty(fpBin);
}

void read_vert_cylindrical_text_bin(const char *tarDir, gl_coord_t *coord)
{
	FILE *fpBin = fp_source_app(tarDir, "r");
	if(fpBin == NULL)
	{
		coord->num = 0;
		AVM_LOGI(LOG_RED("[ERROR]")" read_vert_text_bin file error!!!\n");
		AVM_LOGI("[WARNING] Using Default value!!!\n");
		SET_STATUS_CALIBRATION_IVI(sceneView, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
		return;
	}

	int num = 0;
	fp_read_data_safty(&num, 1, 1 * sizeof(int), fpBin);
	num *= 3;
	coord->num = num;
	AVM_LOGI("index_num = %d\n", coord->num);

	coord->vertice = (float *)calloc(num * 3 * sizeof(float), sizeof(float));
	coord->texture = (float *)calloc(num * 2 * sizeof(float), sizeof(float));

	int index = (num - 1) * 3;
	fp_read_data_safty(coord->vertice, 1, num * 3 * sizeof(float), fpBin);
	AVM_LOGI("vertices[%d] = %f\t", index, (coord->vertice)[index]);
	AVM_LOGI("vertices[%d] = %f\t", index + 1, (coord->vertice)[index + 1]);
	AVM_LOGI("vertices[%d] = %f\n", index + 2, (coord->vertice)[index + 2]);

	index = (num - 1) * 2;
	fp_read_data_safty(coord->texture, 1, num * 2 * sizeof(float), fpBin);
	AVM_LOGI("texture[%d] = %f\t", index, (coord->texture)[index]);
	AVM_LOGI("texture[%d] = %f\n", index + 1, (coord->texture)[index + 1]);

	fp_close_data_safty(fpBin);
}

void read_3D_bin(const char *tarDir, gl_coord_3d_t *coord)
{
	FILE *fpBin = fp_source_avm(tarDir, "r");
	if(fpBin == NULL)
	{
		SET_STATUS_CALIBRATION_IVI(avm_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
		AVM_LOGI(LOG_RED("[ERROR]")" read_3D_bin file error!!!\n");
		coord->num = 0;
		coord->num_2d = 0;
		return;
	}
	int empty = 0;
	#ifdef __CRC_SYSTEM_WRITE__
		fp_read_data_safty(&empty, 1, 1 * sizeof(int), fpBin);
	#endif

	int num = 0, num_2d = 0;
	fp_read_data_safty(&num, 1, 1 * sizeof(int), fpBin);
	fp_read_data_safty(&num_2d, 1, 1 * sizeof(int), fpBin);
	coord->num = num;
	coord->num_2d = num_2d;
	AVM_LOGI("index_num = %d\n", coord->num);
	AVM_LOGI("index_num2D = %d\n", coord->num_2d);

	coord->vertice = (float *)calloc(num * 3 * sizeof(float), sizeof(float));
	coord->texture = (float *)calloc(num * 2 * sizeof(float), sizeof(float));
	coord->alpha = (float *)calloc(num * 2 * sizeof(float), sizeof(float));

	int index = (num - 1) * 3;
	fp_read_data_safty(coord->vertice, sizeof(float), num * 3, fpBin);
	AVM_LOGI("vertices[%d] = %f\t", index, (coord->vertice)[index]);
	AVM_LOGI("vertices[%d] = %f\t", index + 1, (coord->vertice)[index + 1]);
	AVM_LOGI("vertices[%d] = %f\n", index + 2, (coord->vertice)[index + 2]);

	index = (num - 1) * 2;
	fp_read_data_safty(coord->texture, sizeof(float), num * 2, fpBin);
	AVM_LOGI("texture[%d] = %f\t", index, (coord->texture)[index]);
	AVM_LOGI("texture[%d] = %f\n", index + 1, (coord->texture)[index + 1]);

	fp_read_data_safty(coord->alpha, sizeof(float), num * 2, fpBin);
	AVM_LOGI("alpha[%d] = %f\t", index, (coord->alpha)[index]);
	AVM_LOGI("alpha[%d] = %f\n", index + 1, (coord->alpha)[index + 1]);

	fp_close_data_safty(fpBin);
}

void read_opengl_car_seethrough_frame()
{
	char tarDir[1024];
	sprintf(tarDir, "%sEVSTable/car_model/seethroughframe/seethroughFrame.png", EVSTable_path[0]);
	int w1,h1,n;
	glGenTextures(1, &g_shadowImgId[0]);

	// while(g_shadowImgId[0] == 0)
	// {
	// 	int maxTextureUnits;
	// 	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
	// 	AVM_LOGI("Max texture units: %d\n", maxTextureUnits);
	// 	glGenTextures(1, &g_shadowImgId[0]);
	// }

    stbi_set_flip_vertically_on_load(1);
    unsigned char *data1 = stbi_load(tarDir, &w1, &h1, &n, 4);
	if(data1 != NULL)
	{
		autosys.system_info.seethrough_frame_read_flag = 1;
		glBindTexture(GL_TEXTURE_2D, g_shadowImgId[0]);
    	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w1, h1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    	stbi_image_free(data1);
		glBindTexture(GL_TEXTURE_2D, 0);
		AVM_LOGI("\033[35m CarSeethrough frame png Successfully\033[0m\n");
	}
	else
	{
		AVM_LOGE("Carseethrough frame non used\n");
		AVM_LOGI("\033[31m CarSeethrough frame png not find\033[0m\n");
	}


}
// test_shadow
void read_car_shadow(const char *tarDir, gl_coord_3d_t *coord)
{
	FILE *fpBin = fp_source_app(tarDir, "r");
	if (fpBin == NULL)
	{
		AVM_LOGI("[WARNING] Failed to open shadow file.\n");
		return;
	}
	char data[150] = {0};
	int _vNum = 0;

	if (fscanf(fpBin, "rgb %f %f %f\n", &chassisColor[0], &chassisColor[1], &chassisColor[2]) == 3)
	{
		chassisColor[0] /= 255.0f;
		chassisColor[1] /= 255.0f;
		chassisColor[2] /= 255.0f;
		autosys.shadow.color.X = chassisColor[0];
		autosys.shadow.color.Y = chassisColor[1];
		autosys.shadow.color.Z = chassisColor[2];
		AVM_LOGI("CarShadow RGB: %f %f %f\n", chassisColor[0], chassisColor[1], chassisColor[2]);
	}
	AVM_LOGI("CarShadow RGB: %f %f %f\n", chassisColor[0], chassisColor[1], chassisColor[2]);

	// CarShadowBlurFlag

	if (fscanf(fpBin, "blur %d\n", &CarShadowBlurFlag) == 1)
	{
		// printf("CarShadow RGB: %f %f %f\n", chassisColor[0], chassisColor[1], chassisColor[2]);
	}
	else
	{
		autosys.avm_page.page->set.shadow.flag_img_mode = 1;
		CarShadowBlurFlag = 1;
	}

	while (fgets(data, 150, fpBin) != NULL)
	{
		if (data[0] == 'a')
		{
			_vNum++;
		}
	}
	AVM_LOGI("_vNum: %d\n", _vNum);

	coord->vertice = (float *)calloc(_vNum * 3 * sizeof(float), sizeof(float));
	coord->texture = (float *)calloc(_vNum * 2 * sizeof(float), sizeof(float));
	coord->alpha = (float *)calloc(_vNum * sizeof(float), sizeof(float));
	coord->num = _vNum;

	fseek(fpBin, 0, SEEK_SET);
	_vNum = 0;

	int _v_Num = 0;
	int _vt_Num = 0;
	int _a_Num = 0;

	while (fgets(data, 150, fpBin) != NULL)
	{
		if (strncmp(data, "v ", 2) == 0)
		{
			sscanf(data, "v %f %f %f\n", &coord->vertice[3 * _v_Num], &coord->vertice[3 * _v_Num + 1], &coord->vertice[3 * _v_Num + 2]);
			coord->vertice[3 * _v_Num + 1] = 0.0f;
			_v_Num++;
		}
		else if (strncmp(data, "a ", 2) == 0)
		{
			sscanf(data, "a %f\n", &coord->alpha[_a_Num]);
			_a_Num++;
		}
		else if (strncmp(data, "vt ", 3) == 0)
		{
			sscanf(data, "vt %f %f\n", &coord->texture[2 * _vt_Num], &coord->texture[2 * _vt_Num + 1]);
			_vt_Num++;
		}
	}

	int index = (_a_Num - 1) * 3;
	AVM_LOGI("CarShadow obj :\n");
	AVM_LOGI("vertices[%d] = %f\t", index, (coord->vertice)[index]);
	AVM_LOGI("vertices[%d] = %f\t", index + 1, (coord->vertice)[index + 1]);
	AVM_LOGI("vertices[%d] = %f\n", index + 2, (coord->vertice)[index + 2]);

	index = (_a_Num - 1) * 2;
	fp_read_data_safty(coord->texture, sizeof(float), _vNum * 2, fpBin);
	AVM_LOGI("texture[%d] = %f\t", index, (coord->texture)[index]);
	AVM_LOGI("texture[%d] = %f\n", index + 1, (coord->texture)[index + 1]);
	index = (_a_Num - 1) * 1;
	fp_read_data_safty(coord->alpha, sizeof(float), _vNum * 1, fpBin);
	AVM_LOGI("alpha[%d] = %f\t", index, (coord->alpha)[index]);
	AVM_LOGI("alpha[%d] = %f\n", index + 1, (coord->alpha)[index + 1]);

	fp_close_data_safty(fpBin);
}

void read_file_vec_2d(int determinInitialCount)
{

	char read_file_vec_2d_path[1024];
	char *cam_name[] = {"f", "b", "l", "r"};
	const int cam_count = sizeof(cam_name) / sizeof(cam_name[0]);  
	AVM_LOGI("READ SCENE VIEW DATA\n");
	for(int style = 0; style < PARAM_PGL_MAX_STYLE_NUM; style++)
	{
		for(int ix = 0; ix < cam_count; ix++)
		{
			sprintf(read_file_vec_2d_path, "EVSTable/sceneView/obj_%s_%d.bin", cam_name[ix], style);
			read_vert_text_bin(read_file_vec_2d_path, &view2d[ix][style]);
		}
	}


	// read_vert_text_bin("EVSTable/sceneView/obj_l.bin", &view2d[2]);
	// AVM_LOGI("FISH CAMERA Right\n");
	// read_vert_text_bin("EVSTable/sceneView/obj_r.bin", &view2d[3]);

	AVM_LOGI("READ CYLINDRICAL \n");
	read_vert_cylindrical_text_bin("EVSTable/cylindrical/front.obj", &Cylindrical2d[0]);
	read_vert_cylindrical_text_bin("EVSTable/cylindrical/back.obj",  &Cylindrical2d[1]);
	read_vert_cylindrical_text_bin("EVSTable/cylindrical/left.obj",  &Cylindrical2d[2]);
	read_vert_cylindrical_text_bin("EVSTable/cylindrical/right.obj", &Cylindrical2d[3]);
	// FILE *fp = fopen("/media/flash/userdata/EVSTable/sceneView/obj_l.bin", "r");
	// if (fp)
	// {
	// 	fclose(fp);
	// 	AVM_LOGI("\nFISH CAMERA Left\n");
	// 	read_vert_text_bin("/media/flash/userdata/EVSTable/sceneView/obj_l.bin", &view2d[2]);

	// 	AVM_LOGI("\nFISH CAMERA Right\n");
	// 	read_vert_text_bin("/media/flash/userdata/EVSTable/sceneView/obj_r.bin", &view2d[3]);
	// }
	// else
	// {
	// 	// AVM_LOGI("\nFISH CAMERA Front\n");
	// 	// read_vert_text_bin("EVSTable/sceneView/obj_f.bin", &view2d[0]);

	// 	// AVM_LOGI("\nFISH CAMERA Back\n");
	// 	// read_vert_text_bin("EVSTable/sceneView/obj_b.bin", &view2d[1]);

	// 	AVM_LOGI("\nFISH CAMERA Left\n");
	// 	read_vert_text_bin("EVSTable/sceneView/obj_l.bin", &view2d[2]);

	// 	AVM_LOGI("\nFISH CAMERA Right\n");
	// 	read_vert_text_bin("EVSTable/sceneView/obj_r.bin", &view2d[3]);

	// 	// AVM_LOGI("\nBSD Left\n");
	// 	// read_vert_text_bin("view2d/obj_bsd_L.bin", &view2d[4]);

	// 	// AVM_LOGI("\nBSD Right\n");
	// 	// read_vert_text_bin("view2d/obj_bsd_R.bin", &view2d[5]);
	// }
}

uint16_t crc16(const uint8_t *data, uint32_t len, uint16_t crc) {
    uint8_t i;

    while (len--) {
        crc ^= *data++ << 8;
        for (i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x8005;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

void read_file_vec(int determinInitialCount)
{
	if (determinInitialCount != 0)
	{
		free(colorblk0);
		free(colorblk1);
		free(colorblk2);
		free(colorblk3);
		free(colorblk4);
		free(colorblk5);
		free(colorblk6);
		free(colorblk7);
		free(colorblk_mid0);
		free(colorblk_mid1);
		free(colorblk_mid2);
		free(colorblk_mid3);
		free(vertices_3D_model_info);
		free(Front_texture);
		free(Back_texture);
		free(Left_texture);
		free(Right_texture);
		free(texture_alpha);
	}

	int ix, iy;
	int empty;
	AVM_LOGI("Load avm3D vert in bin file.\n");
	char read_file_vec_path[1024];
	char read_file_vec_path_color[1024];
	FILE *fp ;
for(ix = 0; ix < 4 ; ix++)
	{
		sprintf(read_file_vec_path, "EVSTable/avm/obj_%d.bin", ix);

		AVM_LOGI("**************************************************************\n");
		read_3D_bin(read_file_vec_path, &avm3D[ix]);
		AVM_LOGI("**************************************************************\n");
	}

	read_car_shadow("EVSTable/car_model/shadow/Car_shadow_area.obj", &carShadow); // test_shadow
	AVM_LOGI("**************************************************************\n");
	FILE *fpavm3Dcolor = fp_source_avm("EVSTable/avm/avm3Dcolor.bin", "r");
	if(fpavm3Dcolor == NULL)
	{
		AVM_LOGI(LOG_RED("[ERROR]")" fpavm3Dcolor file error!!!\n");
		AVM_LOGI("[WARNING] Using Default value!!!\n");
		total_ov_area = 2500;
		AVM_LOGI("total_ov_area = %d\n", total_ov_area);
		colorblk0 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk1 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk2 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk3 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk4 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk5 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk6 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk7 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk_mid0 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk_mid1 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk_mid2 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		colorblk_mid3 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
		SET_STATUS_CALIBRATION_IVI(avm_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
	}
	else
	{
#ifdef __CRC_SYSTEM_WRITE__
	fp_read_data_safty(&empty, 1, 1 * sizeof(int), fpavm3Dcolor);
#endif
	fp_read_data_safty(&total_ov_area, 1, 1 * sizeof(int), fpavm3Dcolor);
	AVM_LOGI("total_ov_area = %d\n", total_ov_area);
	colorblk0 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk1 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk2 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk3 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk4 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk5 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk6 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk7 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk_mid0 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk_mid1 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk_mid2 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));
	colorblk_mid3 = (int *)calloc(total_ov_area * 2 * sizeof(int), sizeof(int));

	fp_read_data_safty(colorblk0, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk1, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk2, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk3, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk4, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk5, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk6, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk7, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk_mid0, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk_mid1, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk_mid2, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);
	fp_read_data_safty(colorblk_mid3, 1, total_ov_area * 2 * sizeof(int), fpavm3Dcolor);

	fp_close_data_safty(fpavm3Dcolor);
	}

	int *colorblk[12] = 
	{
		colorblk0,
		colorblk1,
		colorblk2,
		colorblk3,
		colorblk4,
		colorblk5,
		colorblk6,
		colorblk7,
		colorblk_mid0,
		colorblk_mid1,
		colorblk_mid2,
		colorblk_mid3
	};
	trans_color_block_to_bb(total_ov_area, colorblk);
	AVM_LOGI("**************************************************************\n");
}

void routine_camera(trans_camera_t transCamera, can_bus_info_t canbus)
{
#ifndef READ_FILE
#if (PLAFORM == IMAX8)
	get_camera_buf(&cameraFileFrontBuf, &cameraFileLeftBuf, &cameraFileRightBuf, &cameraFileBackBuf);
#elif (PLAFORM == MTK2712)
	get_camera_buf(&cameraFileFrontBuf, &cameraFileLeftBuf, &cameraFileRightBuf, &cameraFileBackBuf, &cameraFileLeftBSDBuf, &cameraFileRightBSDBuf, transCamera);
#elif (PLAFORM == SPHE8368_P)
	//			get_cam_va_from_8368_p(cameraBufF, cameraBufB, cameraBufL, cameraBufR);
#endif
#endif
}

static void open_gl_init_texture(void)
{
	#ifndef READ_FILE
	glGenTextures(1, &g_texYId[0]);
	glGenTextures(1, &g_texYId[1]);
	glGenTextures(1, &g_texYId[2]);
	glGenTextures(1, &g_texYId[3]);
	#endif
	glGenTextures(1, &g_texYId[4]);
	glGenTextures(1, &g_texYId[5]);
	glGenTextures(1, &g_texYId[6]); // for carshoadow;

#if (PLAFORM == MTK2712 || PLAFORM == NT98690)
	glActiveTexture(GL_TEXTURE0 + LEFT_BSD_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[4]);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBSDBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBSDBuf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE0 + RIGHT_BSD_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[5]);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileRightBSDBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBSDBuf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[0]);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileFrontBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBSDBuf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE0 + BACK_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[1]);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileBackBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBSDBuf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE0 + LEFT_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[2]);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBSDBuf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE0 + RIGHT_CAMERA);
	glBindTexture(GL_TEXTURE_2D, g_texYId[3]);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileRightBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, imageWidth, imageHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBSDBuf);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
	glGenTextures(1, &g_tex_alpha[0]);
	glGenTextures(1, &g_tex_color[0]);
	glGenTextures(1, &g_tex_color2[0]); // for carshoadow;

	// int w1,h1,n;

	// stbi_set_flip_vertically_on_load(0);

	// char imgpath[1024];
	// sprintf(imgpath, "%sEVSTable/avm/alpha.png", EVSTable_path[0]);
    // unsigned char *data1 = stbi_load(imgpath, &w1, &h1, &n, 3);
	// sprintf(imgpath, "%sEVSTable/avm/colorOut1.png", EVSTable_path[0]);
    // unsigned char *data2 = stbi_load(imgpath, &w1, &h1, &n, 3);
	// // if(delay_loading == 100)z
	// // {
	// glActiveTexture(GL_TEXTURE0 + MAIN_ALPHA);
	// glBindTexture(GL_TEXTURE_2D, g_tex_alpha[0]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // stbi_image_free(data1);

	// glActiveTexture(GL_TEXTURE0 + MAIN_COLOR2);
	// glBindTexture(GL_TEXTURE_2D, g_tex_color2[0]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // stbi_image_free(data2);
	// }
	// else
	// {
	// 	delay_loading++;
	// }
}

static void open_gl_init_avm_3d(void)
{
	glGenVertexArrays(VAO_NUM, VAO);

	glUseProgram(programHandleMain[0]);

	glAttr3DVertexFB = glGetAttribLocation(programHandleMain[0], "vPosition");	 // 3D model
	glAttr3DTextureFB = glGetAttribLocation(programHandleMain[0], "a_texCoord"); // Front Camera
	glAttralphaFB = glGetAttribLocation(programHandleMain[0], "a_tex_alpha");	 // alpha

	glAttr3DVertexLR = glGetAttribLocation(programHandleMain[1], "vPosition");	 // Back Camera
	glAttr3DTextureLR = glGetAttribLocation(programHandleMain[1], "a_texCoord"); // Left Camera
	glAttralphaLR = glGetAttribLocation(programHandleMain[1], "a_tex_alpha"); // Left Camera

	glGenBuffers(3, VBO_3davm[0]); // 3D AVM model
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[0][0]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[0].num * 3 * sizeof(float), avm3D[0].vertice, GL_STATIC_DRAW); // VBO bowl Vertex

	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[0][1]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[0].num * 2 * sizeof(float), avm3D[0].texture, GL_STATIC_DRAW); // Front Vertex

	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[0][2]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[0].num * 2 * sizeof(float), avm3D[0].alpha, GL_STATIC_DRAW); // Front Vertex

	free(avm3D[0].vertice);
	free(avm3D[0].texture);
	free(avm3D[0].alpha);

	glGenBuffers(3, VBO_3davm[1]); // 3D AVM model
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[1][0]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[1].num * 3 * sizeof(float), avm3D[1].vertice, GL_STATIC_DRAW); // VBO bowl Vertex

	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[1][1]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[1].num * 2 * sizeof(float), avm3D[1].texture, GL_STATIC_DRAW); // Front Vertex

	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[1][2]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[1].num * 2 * sizeof(float), avm3D[1].alpha, GL_STATIC_DRAW); // Front Vertex

	free(avm3D[1].vertice);
	free(avm3D[1].texture);
	free(avm3D[1].alpha);

	glGenBuffers(3, VBO_3davm[2]); // 3D AVM model
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[2][0]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[2].num * 3 * sizeof(float), avm3D[2].vertice, GL_STATIC_DRAW); // VBO bowl Vertex

	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[2][1]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[2].num * 2 * sizeof(float), avm3D[2].texture, GL_STATIC_DRAW); // Front Vertex

	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[2][2]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[2].num * 2 * sizeof(float), avm3D[2].alpha, GL_STATIC_DRAW); // Front Vertex

	free(avm3D[2].vertice);
	free(avm3D[2].texture);
	free(avm3D[2].alpha);

	glGenBuffers(3, VBO_3davm[3]); // 3D AVM model
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[3][0]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[3].num * 3 * sizeof(float), avm3D[3].vertice, GL_STATIC_DRAW); // VBO bowl Vertex

	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[3][1]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[3].num * 2 * sizeof(float), avm3D[3].texture, GL_STATIC_DRAW); // Front Vertex

	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[3][2]);
	glBufferData(GL_ARRAY_BUFFER, avm3D[3].num * 2 * sizeof(float), avm3D[3].alpha, GL_STATIC_DRAW); // Front Vertex

	free(avm3D[3].vertice);
	free(avm3D[3].texture);
	free(avm3D[3].alpha);

	glBindVertexArray(VAO[0]);

	glEnableVertexAttribArray(glAttr3DVertexFB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[0][0]);
	glVertexAttribPointer(glAttr3DVertexFB, 3, GL_FLOAT, GL_FALSE, 0, (void *)0); // 3D Vertex

	glEnableVertexAttribArray(glAttr3DTextureFB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[0][1]);
	glVertexAttribPointer(glAttr3DTextureFB, 2, GL_FLOAT, GL_FALSE, 0, (void *)0); // Front

	glEnableVertexAttribArray(glAttralphaFB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[0][2]);
	glVertexAttribPointer(glAttralphaFB, 2, GL_FLOAT, GL_FALSE, 0, (void *)0); // alpha

	glBindVertexArray(0);

	glBindVertexArray(VAO[1]);

	glEnableVertexAttribArray(glAttr3DVertexFB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[1][0]);
	glVertexAttribPointer(glAttr3DVertexFB, 3, GL_FLOAT, GL_FALSE, 0, (void *)0); // 3D Vertex

	glEnableVertexAttribArray(glAttr3DTextureFB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[1][1]);
	glVertexAttribPointer(glAttr3DTextureFB, 2, GL_FLOAT, GL_FALSE, 0, (void *)0); // Front

	glEnableVertexAttribArray(glAttralphaFB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[1][2]);
	glVertexAttribPointer(glAttralphaFB, 2, GL_FLOAT, GL_FALSE, 0, (void *)0); // alpha

	glBindVertexArray(VAO[2]);

	glEnableVertexAttribArray(glAttr3DVertexLR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[2][0]);
	glVertexAttribPointer(glAttr3DVertexLR, 3, GL_FLOAT, GL_FALSE, 0, (void *)0); // 3D Vertex

	glEnableVertexAttribArray(glAttr3DTextureLR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[2][1]);
	glVertexAttribPointer(glAttr3DTextureLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0); // Front

	glEnableVertexAttribArray(glAttralphaLR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[2][2]);
	glVertexAttribPointer(glAttralphaLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0); // alpha

	glBindVertexArray(VAO[3]);

	glEnableVertexAttribArray(glAttr3DVertexLR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[3][0]);
	glVertexAttribPointer(glAttr3DVertexLR, 3, GL_FLOAT, GL_FALSE, 0, (void *)0); // 3D Vertex

	glEnableVertexAttribArray(glAttr3DTextureLR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[3][1]);
	glVertexAttribPointer(glAttr3DTextureLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0); // Front

	glEnableVertexAttribArray(glAttralphaLR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3davm[3][2]);
	glVertexAttribPointer(glAttralphaLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0); // alpha
	glBindVertexArray(0);

	// test_shadow
	// static float vertexCoordView2dADASIMS[3*6] = {
	// 	-0.500000,-0.500000,0.100000,
	// 	0.500000,-0.500000,0.100000,
	// 	0.500000, 0.500000,0.100000,
	// 	0.500000, 0.500000,0.100000,
	// 	-0.500000, 0.500000,0.100000,
	// 	-0.500000,-0.500000,0.100000
	// };
	static float vertexCoordView2dADASIMS[3 * 6] = {
		-1.000000, -1.000000, 0.100000,
		1.000000, -1.000000, 0.100000,
		1.000000, 1.000000, 0.100000,
		1.000000, 1.000000, 0.100000,
		-1.000000, 1.000000, 0.100000,
		-1.000000, -1.000000, 0.100000};
	static float fragmentCoordView2dADASIMS[2 * 6] = {
		0.00000,
		0.00000,
		0.00000,
		1.00000,
		1.00000,
		0.00000,
		0.00000,
		1.00000,
		1.00000,
		0.00000,
		1.00000,
		1.00000,
	};

	glUseProgram(programHandleCarShadow);

	glAttrShadowVertex = glGetAttribLocation(programHandleCarShadow, "vPosition");
	glAttrShadowTexture = glGetAttribLocation(programHandleCarShadow, "InTexCoord");
	glAttrShadowalpha = glGetAttribLocation(programHandleCarShadow, "alphaValue");

	glGenVertexArrays(5, &VAOCarShadow);
	glGenBuffers(3, VBOCarShadow[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[0][0]);
	glBufferData(GL_ARRAY_BUFFER, carShadow.num * 3 * sizeof(float), carShadow.vertice, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[0][1]);
	glBufferData(GL_ARRAY_BUFFER, carShadow.num * 2 * sizeof(float), carShadow.texture, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[0][2]);
	glBufferData(GL_ARRAY_BUFFER, carShadow.num * 1 * sizeof(float), carShadow.alpha, GL_STATIC_DRAW);

	// int v = 0;
	// AVM_LOGI("carshadow vertice %f, %f, %f, \n", carShadow.vertice[v++], carShadow.vertice[v++], carShadow.vertice[v++]);
	// AVM_LOGI("carshadow vertice %f, %f, %f, \n", carShadow.vertice[v++], carShadow.vertice[v++], carShadow.vertice[v++]);
	// AVM_LOGI("carshadow vertice %f, %f, %f, \n", carShadow.vertice[v++], carShadow.vertice[v++], carShadow.vertice[v++]);
	// AVM_LOGI("carshadow vertice %f, %f, %f, \n", carShadow.vertice[v++], carShadow.vertice[v++], carShadow.vertice[v++]);
	// AVM_LOGI("carshadow vertice %f, %f, %f, \n", carShadow.vertice[v++], carShadow.vertice[v++], carShadow.vertice[v++]);
	// AVM_LOGI("carshadow vertice %f, %f, %f, \n", carShadow.vertice[v++], carShadow.vertice[v++], carShadow.vertice[v++]);
	// AVM_LOGI("carshadow vertice %f, %f, %f, \n", carShadow.vertice[v++], carShadow.vertice[v++], carShadow.vertice[v++]);
	// AVM_LOGI("carshadow vertice %f, %f, %f, \n", carShadow.vertice[v++], carShadow.vertice[v++], carShadow.vertice[v++]);
	// v = 0;
	// AVM_LOGI("carshadow vertice %f, %f \n", carShadow.texture[v++], carShadow.texture[v++]);
	// AVM_LOGI("carshadow vertice %f, %f \n", carShadow.texture[v++], carShadow.texture[v++]);
	// AVM_LOGI("carshadow vertice %f, %f \n", carShadow.texture[v++], carShadow.texture[v++]);
	// AVM_LOGI("carshadow vertice %f, %f \n", carShadow.texture[v++], carShadow.texture[v++]);
	// AVM_LOGI("carshadow vertice %f, %f \n", carShadow.texture[v++], carShadow.texture[v++]);
	// AVM_LOGI("carshadow vertice %f, %f \n", carShadow.texture[v++], carShadow.texture[v++]);
	// AVM_LOGI("carshadow vertice %f, %f \n", carShadow.texture[v++], carShadow.texture[v++]);
	// AVM_LOGI("carshadow vertice %f, %f \n", carShadow.texture[v++], carShadow.texture[v++]);

	glBindVertexArray(VAOCarShadow[0]);

	glEnableVertexAttribArray(glAttrShadowVertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[0][0]);
	glVertexAttribPointer(glAttrShadowVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glEnableVertexAttribArray(glAttrShadowTexture);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[0][1]);
	glVertexAttribPointer(glAttrShadowTexture, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glEnableVertexAttribArray(glAttrShadowalpha);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[0][2]);
	glVertexAttribPointer(glAttrShadowalpha, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);

	#if 1
	/* Seethrough */
	glGenBuffers(3, VBOCarShadow[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[1][0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), autosys.program_context[SEETHROUGH].vertexCoord[0], GL_STATIC_DRAW);
	//AVM_LOGE("LOOKKOKOKOKOK\n");
    //AVM_LOGI("Seethrough vertices (x, y, z):\n");
    // for (int i = 0; i < 18; i += 3)
    // {
    //     AVM_LOGI("Vertex %d: (%f, %f, %f)\n", i / 3 + 1, 
	// 	autosys.program_context[SEETHROUGH].vertexCoord[0][i], 
	// 	autosys.program_context[SEETHROUGH].vertexCoord[0][i + 1], 
	// 	autosys.program_context[SEETHROUGH].vertexCoord[0][i + 2]);
    // }
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[1][1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordView2dADASIMS, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[1][2]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 1 * sizeof(float), autosys.program_context[SEETHROUGH].alphaValue[0], GL_STATIC_DRAW);

	glBindVertexArray(VAOCarShadow[1]);

	glEnableVertexAttribArray(glAttrShadowVertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[1][0]);
	glVertexAttribPointer(glAttrShadowVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glEnableVertexAttribArray(glAttrShadowTexture);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[1][1]);
	glVertexAttribPointer(glAttrShadowTexture, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glEnableVertexAttribArray(glAttrShadowalpha);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[1][2]);
	glVertexAttribPointer(glAttrShadowalpha, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glBindVertexArray(0);

	free(carShadow.vertice);
	free(carShadow.texture);
	free(carShadow.alpha);


	glGenBuffers(2, VBOCarShadow[2]);
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[2][0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordView2dADASIMS, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[2][1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordView2dADASIMS, GL_STATIC_DRAW);
	}

	glBindVertexArray(VAOCarShadow[2]);
	{
		glEnableVertexAttribArray(glAttrShadowVertex);
		glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[2][0]);
		glVertexAttribPointer(glAttrShadowVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(glAttrShadowTexture);
		glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[2][1]);
		glVertexAttribPointer(glAttrShadowTexture, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	}
	glBindVertexArray(0);
#endif
	// float vertexCoord_ui_right_down[3*6] =
	// {
	// 	-0.250000,	-0.250000,	0.100000,
	// 	0.250000,	-0.250000,	0.100000,
	// 	0.250000, 	0.250000,	0.100000,
	// 	0.250000, 	0.250000,	0.100000,
	// 	-0.250000, 	0.250000,	0.100000,
	// 	-0.250000,	-0.250000,	0.100000
	// };
#if 0	
	float vertexCoord_ui_right_down[3 * 6] =
		{
			-1.00000,
			0.64400,
			0.100000,
			-0.80000,
			0.64400,
			0.100000,
			-0.80000,
			1.00000,
			0.100000,
			-0.80000,
			1.00000,
			0.100000,
			-1.00000,
			1.00000,
			0.100000,
			-1.00000,
			0.64400,
			0.100000,
		};
	// float fragmentCoord_ui_2d[2*6] =
	// {
	// 	0.00000, 0.00000,
	// 	0.00000, 1.00000,
	// 	1.00000, 1.00000,
	// 	1.00000, 1.00000,
	// 	0.00000, 0.00000,
	// 	0.00000, 1.00000,
	// };
	float fragmentCoord_ui_2d[2 * 6] =
		{
			0.00000,
			0.00000,
			1.00000,
			0.00000,
			1.00000,
			1.00000,
			1.00000,
			1.00000,
			0.00000,
			1.00000,
			0.00000,
			0.00000,
		};

	glGenBuffers(2, VBOCarShadow[3]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[3][0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoord_ui_right_down, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[3][1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoord_ui_2d, GL_STATIC_DRAW);

	glBindVertexArray(VAOCarShadow[3]);
	glEnableVertexAttribArray(glAttrShadowVertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[3][0]);
	glVertexAttribPointer(glAttrShadowVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glEnableVertexAttribArray(glAttrShadowTexture);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[3][1]);
	glVertexAttribPointer(glAttrShadowTexture, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindVertexArray(0);

	float vertexCoord_ui_left_down[3 * 6] =
		{
			1.00000,
			0.64400,
			0.100000,
			0.80000,
			0.64400,
			0.100000,
			0.80000,
			1.00000,
			0.100000,
			0.80000,
			1.00000,
			0.100000,
			1.00000,
			1.00000,
			0.100000,
			1.00000,
			0.64400,
			0.100000,
		};

	float fragmentCoord_ui_2d_left_down[2 * 6] =
		{
			1.00000,
			0.00000,
			0.00000,
			0.00000,
			0.00000,
			1.00000,
			0.00000,
			1.00000,
			1.00000,
			1.00000,
			1.00000,
			0.00000,
		};

	glGenBuffers(2, VBOCarShadow[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[4][0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoord_ui_left_down, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[4][1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoord_ui_2d_left_down, GL_STATIC_DRAW);

	glBindVertexArray(VAOCarShadow[4]);
	glEnableVertexAttribArray(glAttrShadowVertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[4][0]);
	glVertexAttribPointer(glAttrShadowVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glEnableVertexAttribArray(glAttrShadowTexture);
	glBindBuffer(GL_ARRAY_BUFFER, VBOCarShadow[4][1]);
	glVertexAttribPointer(glAttrShadowTexture, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindVertexArray(0);
#endif	
	 //Init_AVM_Camera_Warining();
}

float* generate_warning_2D_vert_from_user(float top, float left, float w, float h)
{

    float* vertexCoordView2d = (float*)malloc(3 * 6 * sizeof(float));
    if (!vertexCoordView2d) {
        return NULL; 
    }

    vertexCoordView2d[0] = left;
    vertexCoordView2d[1] = top;
    vertexCoordView2d[2] = 0.100000;

    vertexCoordView2d[3] = left + w;
    vertexCoordView2d[4] = top;
    vertexCoordView2d[5] = 0.100000;

    vertexCoordView2d[6] = left + w;
    vertexCoordView2d[7] = top + h;
    vertexCoordView2d[8] = 0.100000;

    vertexCoordView2d[9] = left + w;
    vertexCoordView2d[10] = top + h;
    vertexCoordView2d[11] = 0.100000;

    vertexCoordView2d[12] = left;
    vertexCoordView2d[13] = top + h;
    vertexCoordView2d[14] = 0.100000;

    vertexCoordView2d[15] = left;
    vertexCoordView2d[16] = top;
    vertexCoordView2d[17] = 0.100000;

    return vertexCoordView2d;
}

void Init_AVM_Camera_Warining()
{
	
	glGenVertexArrays(VAO_NUM, VAOCameraWarning);



	// static float vertexCoordView2dADASIMS[3 * 6] = {
	// 	-1.000000, -1.000000, 0.100000,
	// 	1.000000, -1.000000, 0.100000,
	// 	1.000000, 1.000000, 0.100000,
	// 	1.000000, 1.000000, 0.100000,
	// 	-1.000000, 1.000000, 0.100000,
	// 	-1.000000, -1.000000, 0.100000};

	float* vertexCoordView2dADASIMS;
	static float fragmentCoordView2dADASIMS[2 * 6] = {
    	0.00000,
    	1.00000,
    	1.00000,
    	1.00000,
    	1.00000,
    	0.00000,
    	1.00000,
    	0.00000,
    	0.00000,
    	0.00000,
    	0.00000,
    	1.00000,
	};
	glUseProgram(programHandleCarShadow);
	glAttrShadowVertex = glGetAttribLocation(programHandleCarShadow, "vPosition");
	glAttrShadowTexture = glGetAttribLocation(programHandleCarShadow, "InTexCoord");
	glAttrShadowalpha = glGetAttribLocation(programHandleCarShadow, "alphaValue");

	// float User_buffer[5][4] = 
	// {
	// 	{  0.9f,  -0.9f, 0.1f, 0.1f}, //fisheye
	// 	{  0.5f,  0.0f, 0.1f, 0.1f}, //front
	// 	{ -0.5f,  0.0f, 0.1f, 0.1f}, //back
	// 	{  0.0f, -0.5f, 0.1f, 0.1f}, //left
	// 	{  0.0f,  0.5f, 0.1f, 0.1f}, //right
	// };



	for(int ix = 0; ix < 5; ix++)
	{
		float* User_buffer = &ivi_warning.position[ix];
		glGenBuffers(2, VBOCameraWarning[ix]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[ix][0]);
		vertexCoordView2dADASIMS = generate_warning_2D_vert_from_user(User_buffer[0], User_buffer[1], User_buffer[2], User_buffer[3]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordView2dADASIMS, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[ix][1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordView2dADASIMS, GL_STATIC_DRAW);
		free(vertexCoordView2dADASIMS);

		glBindVertexArray(VAOCameraWarning[ix]);
		{
			glEnableVertexAttribArray(glAttrShadowVertex);
			glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[ix][0]);
			glVertexAttribPointer(glAttrShadowVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

			glEnableVertexAttribArray(glAttrShadowTexture);
			glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[ix][1]);
			glVertexAttribPointer(glAttrShadowTexture, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
			glBindVertexArray(0);
		}
	}

	// glGenBuffers(2, VBOCameraWarning[0]);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[0][0]);
	// vertexCoordView2dADASIMS = generate_warning_2D_vert_from_user(-1.0f, -1.0f, 0.1f, 0.1f);
	// glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordView2dADASIMS, GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[0][1]);
	// glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordView2dADASIMS, GL_STATIC_DRAW);
	// free(vertexCoordView2dADASIMS);

	// glGenBuffers(2, VBOCameraWarning[1]);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[1][0]);
	// vertexCoordView2dADASIMS = generate_warning_2D_vert_from_user(-.0f, -1.0f, 0.1f, 0.1f);
	// glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordView2dADASIMS, GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[1][1]);
	// glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordView2dADASIMS, GL_STATIC_DRAW);
	// free(vertexCoordView2dADASIMS);

	// glGenBuffers(2, VBOCameraWarning[2]);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[2][0]);
	// vertexCoordView2dADASIMS = generate_warning_2D_vert_from_user(-1.0f, 1.0f, 0.1f, 0.1f);
	// glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordView2dADASIMS, GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[2][1]);
	// glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordView2dADASIMS, GL_STATIC_DRAW);
	// free(vertexCoordView2dADASIMS);

	// glGenBuffers(2, VBOCameraWarning[3]);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[3][0]);
	// vertexCoordView2dADASIMS = generate_warning_2D_vert_from_user(1.0f, 1.0f, 0.1f, 0.1f);
	// glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordView2dADASIMS, GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[3][1]);
	// glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordView2dADASIMS, GL_STATIC_DRAW);
	// free(vertexCoordView2dADASIMS);

	// glBindVertexArray(VAOCameraWarning[0]);

	// glEnableVertexAttribArray(glAttrShadowVertex);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[0][0]);
	// glVertexAttribPointer(glAttrShadowVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// glEnableVertexAttribArray(glAttrShadowTexture);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOCameraWarning[0][1]);
	// glVertexAttribPointer(glAttrShadowTexture, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	// glBindVertexArray(0);
#if 0
    int w1,h1,n;
	glGenTextures(1, &g_camera_warning[0]);
    stbi_set_flip_vertically_on_load(0);
	unsigned char *data1 = stbi_load("warning.png", &w1, &h1, &n, 0);
	if(data1 != NULL)
	{
		glBindTexture(GL_TEXTURE_2D, g_camera_warning[0]);
    	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    	stbi_image_free(data1);
	}
	#endif
}

static void open_gl_init_cam_2d(void)
{
	glGenVertexArrays(VAO_NUM, VAOView2d);
	glGenVertexArrays(VAO_NUM, VAOViewUnique2d);
	glGenVertexArrays(VAO_NUM, VAOViewAdas);
	glGenVertexArrays(VAO_NUM, VAO_Cylindrical2d);

	glUseProgram(programHandleMain[0]);
	glAttr3DVertexFB = glGetAttribLocation(programHandleMain[0], "vPosition"); // 3D model
	glAttr3DTextureFB = glGetAttribLocation(programHandleMain[0], "a_texCoord"); // Front Camera
	glAttr3DVertexLR = glGetAttribLocation(programHandleMain[1], "vPosition"); // 3D model
	glAttr3DTextureLR = glGetAttribLocation(programHandleMain[1], "a_texCoord"); // Left Camera

	//---------------------------------------------------------------------------------------------fish view
	// VAOView2d[0]
	{
		static float vertexCoordView2d[3 * 6] =
			{
				-1.000000, -1.000000, 0.100000,
				1.000000, -1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				-1.000000, 1.000000, 0.100000,
				-1.000000, -1.000000, 0.100000};

		static float fragmentCoordView2d[2 * 6] =
			{
				0.00000,
				1.00000,
				1.00000,
				1.00000,
				1.00000,
				0.00000,
				1.00000,
				0.00000,
				0.00000,
				0.00000,
				0.00000,
				1.00000,
			};

		static float fragmentCoordMirrorView2d[2 * 6] =
			{
				1.00000,
				1.00000,
				0.00000,
				1.00000,
				0.00000,
				0.00000,
				0.00000,
				0.00000,
				1.00000,
				0.00000,
				1.00000,
				1.00000,
			};

		glGenBuffers(2, VBO_view2d[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[0][0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordView2d, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[0][1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordView2d, GL_STATIC_DRAW);

		glGenBuffers(2, VBO_view2d[1]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[1][0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordView2d, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[1][1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordMirrorView2d, GL_STATIC_DRAW);

		glBindVertexArray(VAOView2d[0]); // Fish eye.
		glEnableVertexAttribArray(glAttr3DVertexFB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[0][0]);
		glVertexAttribPointer(glAttr3DVertexFB, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(glAttr3DTextureFB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[0][1]);
		glVertexAttribPointer(glAttr3DTextureFB, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindVertexArray(0);

		glBindVertexArray(VAOView2d[1]); // Fish eye.
		glEnableVertexAttribArray(glAttr3DVertexLR);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[0][0]);
		glVertexAttribPointer(glAttr3DVertexLR, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(glAttr3DTextureLR);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[0][1]);
		glVertexAttribPointer(glAttr3DTextureLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindVertexArray(0);

		glBindVertexArray(VAOView2d[2]); // Fish eye mirror.
		glEnableVertexAttribArray(glAttr3DVertexFB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[1][0]);
		glVertexAttribPointer(glAttr3DVertexFB, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(glAttr3DTextureFB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_view2d[1][1]);
		glVertexAttribPointer(glAttr3DTextureFB, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindVertexArray(0);
	}
	//---------------------------------------------------------------------------------------------Alley view
	// VAOViewUnique2d[0]
	{
		static float vertexCoordAlley2d[3 * 6] =
			{
				-1.000000, -1.000000, 0.100000,
				1.000000, -1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				-1.000000, 1.000000, 0.100000,
				-1.000000, -1.000000, 0.100000};
		static float fragmentCoordAlley2d[2 * 6] =
			{
				0.03000,
				0.58000,
				0.95000,
				0.58000,
				0.95000,
				0.15000,
				0.95000,
				0.15000,
				0.03000,
				0.15000,
				0.03000,
				0.58000,
			};

		glGenBuffers(2, VBO_alley2d[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_alley2d[0][0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordAlley2d, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_alley2d[0][1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordAlley2d, GL_STATIC_DRAW);

		glBindVertexArray(VAOViewUnique2d[0]);
		glEnableVertexAttribArray(glAttr3DVertexFB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_alley2d[0][0]);
		glVertexAttribPointer(glAttr3DVertexFB, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(glAttr3DTextureFB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_alley2d[0][1]);
		glVertexAttribPointer(glAttr3DTextureFB, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindVertexArray(0);
	}
	//---------------------------------------------------------------------------------------------DR Back view
	// VAOViewUnique2d[1]
	{
		static float vertexCoordback2d[3 * 6] =
			{
				-1.000000, -1.000000, 0.100000,
				1.000000, -1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				-1.000000, 1.000000, 0.100000,
				-1.000000, -1.000000, 0.100000};
		static float fragmentCoordback2d[2 * 6] =
			{
				0.10000,
				0.90000,
				0.90000,
				0.90000,
				0.90000,
				0.10000,
				0.90000,
				0.10000,
				0.10000,
				0.10000,
				0.10000,
				0.90000,
			};

		glGenBuffers(2, VBO_back2d[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_back2d[0][0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordback2d, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_back2d[0][1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordback2d, GL_STATIC_DRAW);

		glBindVertexArray(VAOViewUnique2d[1]);
		glEnableVertexAttribArray(glAttr3DVertexFB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_back2d[0][0]);
		glVertexAttribPointer(glAttr3DVertexFB, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(glAttr3DTextureFB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_back2d[0][1]);
		glVertexAttribPointer(glAttr3DTextureFB, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindVertexArray(0);
	}
	//---------------------------------------------------------------------------------------------Left tire
	// VAOViewUnique2d[2]
	{
		static float vertexCoordTireLeft2d[3 * 6] =
			{
				-1.000000, -1.000000, 0.100000,
				1.000000, -1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				-1.000000, 1.000000, 0.100000,
				-1.000000, -1.000000, 0.100000};
		static float fragmentCoordTireLeft2d[2 * 6] =
			{
				0.48000,
				0.00000,
				0.48000,
				1.00000,
				0.87500,
				1.00000,
				0.87500,
				1.00000,
				0.87500,
				0.00000,
				0.48000,
				0.00000,
			};

		glGenBuffers(2, VBO_tire[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_tire[0][0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordTireLeft2d, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_tire[0][1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordTireLeft2d, GL_STATIC_DRAW);

		glBindVertexArray(VAOViewUnique2d[2]);
		glEnableVertexAttribArray(glAttr3DVertexLR);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_tire[0][0]);
		glVertexAttribPointer(glAttr3DVertexLR, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(glAttr3DTextureLR);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_tire[0][1]);
		glVertexAttribPointer(glAttr3DTextureLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindVertexArray(0);
	}
	//---------------------------------------------------------------------------------------------Right tire
	// VAOViewUnique2d[3]
	{
		static float vertexCoordTireRight2d[3 * 6] =
			{
				-1.000000, -1.000000, 0.100000,
				1.000000, -1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				1.000000, 1.000000, 0.100000,
				-1.000000, 1.000000, 0.100000,
				-1.000000, -1.000000, 0.100000};
		static float fragmentCoordTireRight2d[2 * 6] =
			{
				0.50000,
				1.00000,
				0.50000,
				0.00000,
				0.1000,
				0.00000,
				0.1000,
				0.00000,
				0.1000,
				1.00000,
				0.50000,
				1.00000,
			};

		glGenBuffers(2, VBO_tire[1]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_tire[1][0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordTireRight2d, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_tire[1][1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordTireRight2d, GL_STATIC_DRAW);

		glBindVertexArray(VAOViewUnique2d[3]);
		glEnableVertexAttribArray(glAttr3DVertexLR);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_tire[1][0]);
		glVertexAttribPointer(glAttr3DVertexLR, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(glAttr3DTextureLR);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_tire[1][1]);
		glVertexAttribPointer(glAttr3DTextureLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindVertexArray(0);
	}
	// 	//---------------------------------------------------------------------------------------------BSD Left side view
	#if false
	for(int ix = 0; ix < 4; ix++)
	{
		for(int style = 0; style < PARAM_PGL_MAX_STYLE_NUM; style++)
		{
			if(view2d[ix][style].num <= 0) continue;
			glGenVertexArrays(1, &VAOViewCorrect2d[ix * 5 + style]);
			glGenBuffers(2, VBO_correct[ix * 5 + style]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[ix * 5 + style][0]);
			glBufferData(GL_ARRAY_BUFFER, view2d[ix][style].num * 3 * sizeof(float), view2d[ix][style].vertice, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[ix * 5 + style][1]);
			glBufferData(GL_ARRAY_BUFFER, view2d[ix][style].num * 2 * sizeof(float), view2d[ix][style].texture, GL_STATIC_DRAW);

			glBindVertexArray(VAOViewCorrect2d[ix * 5 + style]);
			glEnableVertexAttribArray(glAttr3DVertexLR);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[ix * 5 + style][0]);
			glVertexAttribPointer(glAttr3DVertexLR, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

			glEnableVertexAttribArray(glAttr3DTextureLR);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[ix * 5 + style][1]);
			glVertexAttribPointer(glAttr3DTextureLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
			glBindVertexArray(0);

			free(view2d[ix][style].vertice);
			free(view2d[ix][style].texture);
		}
	}
	#else
	for(int ix = 0; ix < MAX_CAM_NUM; ix++)
	{
		for(int style = 0; style < PARAM_PGL_MAX_STYLE_NUM; style++)
		{
			if(view2d[ix][style].num <= 0) continue;

			if(VAOViewCorrect2d[ix + 0 * MAX_CAM_NUM] == 0)
			{
				glGenVertexArrays(1, &VAOViewCorrect2d[ix + 0 * MAX_CAM_NUM]);
				glGenBuffers(2, VBO_correct[ix + 0 * MAX_CAM_NUM]);
			}
			glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[ix + 0 * MAX_CAM_NUM][0]);
			glBufferData(GL_ARRAY_BUFFER, view2d[ix][style].num * 3 * sizeof(float), view2d[ix][style].vertice, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[ix + 0 * MAX_CAM_NUM][1]);
			glBufferData(GL_ARRAY_BUFFER, view2d[ix][style].num * 2 * sizeof(float), view2d[ix][style].texture, GL_STATIC_DRAW);

			glBindVertexArray(VAOViewCorrect2d[ix + 0 * MAX_CAM_NUM]);
			glEnableVertexAttribArray(glAttr3DVertexLR);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[ix + 0 * MAX_CAM_NUM][0]);
			glVertexAttribPointer(glAttr3DVertexLR, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

			glEnableVertexAttribArray(glAttr3DTextureLR);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[ix + 0 * MAX_CAM_NUM][1]);
			glVertexAttribPointer(glAttr3DTextureLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
			glBindVertexArray(0);

			free(view2d[ix][style].vertice);
			free(view2d[ix][style].texture);
			break;
		}
	}

#endif
	{
		for (int ix = 0; ix < 4; ix++)
		{
			glGenBuffers(2, VBO_Cylindrical2d[ix]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_Cylindrical2d[ix][0]);
			glBufferData(GL_ARRAY_BUFFER, Cylindrical2d[ix].num * 3 * sizeof(float), Cylindrical2d[ix].vertice, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, VBO_Cylindrical2d[ix][1]);
			glBufferData(GL_ARRAY_BUFFER, Cylindrical2d[ix].num * 2 * sizeof(float), Cylindrical2d[ix].texture, GL_STATIC_DRAW);

			glBindVertexArray(VAO_Cylindrical2d[ix]);
			glEnableVertexAttribArray(glAttr3DVertexLR);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_Cylindrical2d[ix][0]);
			glVertexAttribPointer(glAttr3DVertexLR, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

			glEnableVertexAttribArray(glAttr3DTextureLR);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_Cylindrical2d[ix][1]);
			glVertexAttribPointer(glAttr3DTextureLR, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
			glBindVertexArray(0);

			free(Cylindrical2d[ix].vertice);
			free(Cylindrical2d[ix].texture);
		}
	}
}

static void open_gl_init_car_shadow()
{
	int w, h;
	unsigned char *data;
	glCarShadowWindow = 1;
	FILE *fpBin = fp_source_app("EVSTable/car_model/shadow/shadow.bin", "rb");
	if (fpBin == NULL)
	{
		AVM_LOGI("[WARNING] CarShadow Windows Setting the NULL\n");
		glCarShadowWindow = 0;
		return;
	}

	fp_read_data_safty(&h, 1, 1 * sizeof(int), fpBin);
	fp_read_data_safty(&w, 1, 1 * sizeof(int), fpBin);
	AVM_LOGI("car_shadow img W %d H %d\n", w, h);

	data = (unsigned char *)malloc(w * h * 3 * sizeof(unsigned char));

	fp_read_data_safty(data, 1, w * h * 3, fpBin);
	fp_close_data_safty(fpBin);

	// FILE * resultS;
	// AVM_LOGI("saving windows readpixels\n");
	// char z[50];
	// sprintf(z, "%s/%s", SOURCE_DIR, "result.rgb");
	// resultS = fopen(z, "wb");
	// fwrite(data, 1, w * h * 3, resultS);
	// fclose(resultS);

	glUseProgram(programHandleCarShadow);
	glGenTextures(1, &g_CarShadowtexId[0]);
	glActiveTexture(GL_TEXTURE0 + CAR_SHADOW);
	glBindTexture(GL_TEXTURE_2D, g_CarShadowtexId[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void open_gl_init_3davm_cam2d(void)
{
	open_gl_init_texture();
	open_gl_init_avm_3d();
	open_gl_init_cam_2d();
	open_gl_init_car_shadow();
	read_opengl_car_seethrough_frame();
	init_static_variable();
	// open_gl_init_adas_ims();
}

void trans_cam_cpu_to_gpu(trans_camera_t *transCamera, can_bus_info_t canbus, int cam_ch)
{
	//	int imageWidth = imageWidth;
	//	int imageHeight = imageHeight;

	//printf("IMG W[%d], H[%d]\n", imageWidth, imageHeight);

	GLuint physical = ~0U;
	if (transCamera[0].front)
	{
#if (PLAFORM == SPHE8368_P)
		GLuint physical = cameraFBufPA;
#endif

		glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA);
		glBindTexture(GL_TEXTURE_2D, g_texYId[0]);
#if (GLAPI == VIV)
#ifdef READ_FILE
				//glTexDirectVIV(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileFrontBuf);
		//memcpy(cameraFileFrontBuf, cameraFileFront, imageWidth * imageHeight * 2);
				//glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#else
		//				glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileFrontBuf, &physical);			
		if (Cam_PixelFMT == 0)
		{
			glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_NV12, (GLvoid **)&cameraFileFrontBuf, &physical);
		}
		else
		{
			glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileFrontBuf, &physical);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#endif
#elif (GLAPI == UYVY || GLAPI == YUYV)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileFrontBuf);
#endif
	}
	//===============================================Back
	if (transCamera[0].back)
	{
#if (PLAFORM == SPHE8368_P)
		GLuint physical = cameraBBufPA;
#endif

		glActiveTexture(GL_TEXTURE0 + BACK_CAMERA);
		glBindTexture(GL_TEXTURE_2D, g_texYId[1]);

#if (GLAPI == VIV)
#ifdef READ_FILE
				//glTexDirectVIV(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileBackBuf);
		//memcpy(cameraFileBackBuf, cameraFileBack, imageWidth * imageHeight * 2);
				//glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#else
		//				glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileBackBuf, &physical);
		if (Cam_PixelFMT == 0)
		{
			glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_NV12, (GLvoid **)&cameraFileBackBuf, &physical);
		}
		else
		{
			glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileBackBuf, &physical);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#endif
#elif (GLAPI == UYVY || GLAPI == YUYV)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileBackBuf);
#endif
	}
	//================================================Left
	if (transCamera[0].left)
	{
#if (PLAFORM == SPHE8368_P)
		GLuint physical = cameraLBufPA;
#endif

		glActiveTexture(GL_TEXTURE0 + LEFT_CAMERA);
		glBindTexture(GL_TEXTURE_2D, g_texYId[2]);

#if (GLAPI == VIV)
#ifdef READ_FILE
				//glTexDirectVIV(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileLeftBuf);
		//memcpy(cameraFileLeftBuf, cameraFileLeft, imageWidth * imageHeight * 2);
				//glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#else
		//				glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileLeftBuf, &physical);
		if (Cam_PixelFMT == 0)
		{
			glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_NV12, (GLvoid **)&cameraFileLeftBuf, &physical);
		}
		else
		{
			glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileLeftBuf, &physical);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#endif
#elif (GLAPI == UYVY || GLAPI == YUYV)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBuf);
#endif
	}
	//================================================Right
	if (transCamera[0].right)
	{
#if (PLAFORM == SPHE8368_P)
		GLuint physical = cameraRBufPA;
#endif

		glActiveTexture(GL_TEXTURE0 + RIGHT_CAMERA);
		glBindTexture(GL_TEXTURE_2D, g_texYId[3]);

#if (GLAPI == VIV)
#ifdef READ_FILE
				//glTexDirectVIV(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileRightBuf);
		//memcpy(cameraFileRightBuf, cameraFileRight, imageWidth * imageHeight * 2);
#else
		//				glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileRightBuf, &physical);

		if (Cam_PixelFMT == 0)
		{
			glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_NV12, (GLvoid **)&cameraFileRightBuf, &physical);
		}
		else
		{
			glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileRightBuf, &physical);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#endif
#elif (GLAPI == UYVY || GLAPI == YUYV)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageWidth, imageHeight, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileRightBuf);
#endif
	}
	/*
		//================================================Left
		if(transCamera[0].leftBSD)
		{
			glActiveTexture(GL_TEXTURE0 + LEFT_BSD_CAMERA);
			glBindTexture(GL_TEXTURE_2D, g_texYId[4]);

			#if(GLAPI == VIV)
				#ifdef READ_FILE
					glTexDirectVIV(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileLeftBSDBuf);
					memcpy(cameraFileLeftBSDBuf, cameraFileLeft, 1280 * 960 * 2);
				#else
					glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileLeftBSDBuf, &physical);
				#endif
				glTexDirectInvalidateVIV(GL_TEXTURE_2D);
			#elif(GLAPI == UYVY || GLAPI == YUYV)
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, transCamera[1].leftBSD, imageWidth, transCamera[2].leftBSD-transCamera[1].leftBSD, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileLeftBSDBuf+(imageWidth * transCamera[1].leftBSD * 2));
			#endif
		}

		//================================================Right
		if(transCamera[0].rightBSD)
		{
			glActiveTexture(GL_TEXTURE0 + RIGHT_BSD_CAMERA);
			glBindTexture(GL_TEXTURE_2D, g_texYId[5]);

			#if(GLAPI == VIV)
				#ifdef READ_FILE
					glTexDirectVIV(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileRightBSDBuf);
					memcpy(cameraFileRightBSDBuf, cameraFileRight, 1280 * 960 * 2);
				#else
					glTexDirectVIVMap(GL_TEXTURE_2D, imageWidth, imageHeight, GL_VIV_YUY2, (GLvoid **)&cameraFileRightBSDBuf, &physical);
				#endif
				glTexDirectInvalidateVIV(GL_TEXTURE_2D);
			#elif(GLAPI == UYVY || GLAPI == YUYV)
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, transCamera[1].rightBSD, imageWidth, transCamera[2].rightBSD-transCamera[1].rightBSD, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, cameraFileRightBSDBuf+(imageWidth * transCamera[1].rightBSD * 2));
			#endif
		}
	*/

	// if (cam_ch == 17 ||cam_ch == 18 )
	// {
	// 	// printf("cam_ch = %d\n",cam_ch);
	// 		// IMS AND ADAS
	// 		glActiveTexture(GL_TEXTURE0 + ADAS_IMS);
	// 		glBindTexture(GL_TEXTURE_2D, g_texYId[7]);
	// 		memcpy(adas_ims_image_buf, adas_ims_image_buf, 1280 * 720 * 2);
	// 		glTexDirectInvalidateVIV(GL_TEXTURE_2D);
	// }
	

	// if(delay_loading == 100)
	// {
	// 			int w1,h1,n;

	// stbi_set_flip_vertically_on_load(0);

	// char imgpath[1024];
	// sprintf(imgpath, "%sEVSTable/avm/alpha.png", EVSTable_path[0]);
    // unsigned char *data1 = stbi_load(imgpath, &w1, &h1, &n, 3);
	// sprintf(imgpath, "%sEVSTable/avm/colorOut1.png", EVSTable_path[0]);
    // unsigned char *data2 = stbi_load(imgpath, &w1, &h1, &n, 3);
	// glActiveTexture(GL_TEXTURE0 + MAIN_ALPHA);
	// glBindTexture(GL_TEXTURE_2D, g_tex_alpha[0]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // stbi_image_free(data1);

	// glActiveTexture(GL_TEXTURE0 + MAIN_COLOR2);
	// glBindTexture(GL_TEXTURE_2D, g_tex_color2[0]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // stbi_image_free(data2);
	// }
	// else
	// {
	// 	delay_loading++;
	// }

}

int flag_only_onces = 0;
int flag_contorl_loading_bowl = 0;

void draw_3d_avm_around(float *rotateReslut, int windows, int cameraPosition, int camera)
{
	int *avminfo_door = &autosys.current_info.avm_info.door_status;
	if(flag_only_onces == 0 && flag_contorl_loading_bowl == 0)
	{
		flag_contorl_loading_bowl = 1;

		if(AVM_BOWL_alpha_data1 != NULL && AVM_BOWL_alpha_data1_w1 > 0 && AVM_BOWL_alpha_data1_h1 > 0)
		{
			glActiveTexture(GL_TEXTURE0 + MAIN_ALPHA);
			glBindTexture(GL_TEXTURE_2D, g_tex_alpha[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, AVM_BOWL_alpha_data1_w1, AVM_BOWL_alpha_data1_h1, 0, GL_RGB, GL_UNSIGNED_BYTE, AVM_BOWL_alpha_data1);
#if PARAM_MINIMAP_FUNCTION
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, PARAM_MINIMAP_MAX_LEVEL);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
			stbi_image_free(AVM_BOWL_alpha_data1);
			AVM_LOGI("3D BOWL Alpha reading OK\n");
		}
		else
		{
			SET_STATUS_CALIBRATION_IVI(avm_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
			AVM_LOGI(LOG_RED("[ERROR]") "CAN NOT READ AVM FILE\n");
		}

		if(AVM_BOWL_alpha_data2 != NULL && AVM_BOWL_alpha_data1_w2 > 0 && AVM_BOWL_alpha_data1_h2 > 0 )
		{
			glActiveTexture(GL_TEXTURE0 + MAIN_COLOR2);
			glBindTexture(GL_TEXTURE_2D, g_tex_color2[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, AVM_BOWL_alpha_data1_w2, AVM_BOWL_alpha_data1_h2, 0, GL_RGB, GL_UNSIGNED_BYTE, AVM_BOWL_alpha_data2);
#if PARAM_MINIMAP_FUNCTION
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, PARAM_MINIMAP_MAX_LEVEL);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
			stbi_image_free(AVM_BOWL_alpha_data2);
			AVM_LOGI("3D BOWL Color reading OK\n");
		}
		else
		{
			SET_STATUS_CALIBRATION_IVI(avm_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
			AVM_LOGI(LOG_RED("[ERROR]") "CAN NOT READ AVM FILE\n");
		}

		if(AVM_BOWL_alpha_data3 != NULL && AVM_BOWL_alpha_data1_w3 > 0 && AVM_BOWL_alpha_data1_h3 > 0 )
		{
			glActiveTexture(GL_TEXTURE0 + MAIN_COLOR2);
			glBindTexture(GL_TEXTURE_2D, g_tex_color[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, AVM_BOWL_alpha_data1_w3, AVM_BOWL_alpha_data1_h3, 0, GL_RGB, GL_UNSIGNED_BYTE, AVM_BOWL_alpha_data3);
#if PARAM_MINIMAP_FUNCTION
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, PARAM_MINIMAP_MAX_LEVEL);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
			stbi_image_free(AVM_BOWL_alpha_data3);
			AVM_LOGI("3D BOWL Color reading OK\n");
		}
		else
		{
			SET_STATUS_CALIBRATION_IVI(avm_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
			AVM_LOGI(LOG_RED("[ERROR]") "CAN NOT READ AVM FILE\n");
		}

	}

	int programNum;
	program_context *blur = &autosys.program_context[BLUR];
	autosys_current_info *cur_info = &autosys.current_info;
	AVM_INFO *avm_info = &autosys.current_info.avm_info;
	// rotateReslut[1] *= -1.0f;
	// rotateReslut[5] *= -1.0f;
	// rotateReslut[9] *= -1.0f;
	// rotateReslut[13] *= -1.0f;
	switch (camera)
	{
	case 0:
		glUseProgram(programHandleMain[0]); 

		glUni_matrixLocation = glGetUniformLocation(programHandleMain[0], "transformatiomMatrix");
		glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, rotateReslut); 

		glUni_SamplerYFront = glGetUniformLocation(programHandleMain[0], "Texture");
		glUniform1i(glUni_SamplerYFront, FRONT_CAMERA); 
		glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA);
		#if PARAM_FISHEYE_BLUR_FUNCTION
			glBindTexture(GL_TEXTURE_2D,blur->textureID[FISHEYE_FRONT_BLUR_TEXTURE]);
		#else
			glBindTexture(GL_TEXTURE_2D, g_texYId[0]);
		#endif

		glUniform1i(glGetUniformLocation(programHandleMain[0], "TextureAlpha"), MAIN_ALPHA);
		glActiveTexture(GL_TEXTURE0 + MAIN_ALPHA);
    	glBindTexture(GL_TEXTURE_2D, g_tex_alpha[0]);
	
		glUniform1i(glGetUniformLocation(programHandleMain[0], "TextureColor"), MAIN_COLOR2);
		glActiveTexture(GL_TEXTURE0 + MAIN_COLOR2);
    	glBindTexture(GL_TEXTURE_2D, g_tex_color2[0]);


		glUniform1f(glGetUniformLocation(programHandleMain[0], "avg_L"),    	 g_avmBBConfig.avg_part1[0]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "avg_R"),    	 g_avmBBConfig.avg_part2[0]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "diff_L"),   	 g_avmBBConfig.diff_part1[0]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "diff_R"),   	 g_avmBBConfig.diff_part2[0]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "gamma_L"),  	 g_avmBBConfig.gammaL[0]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "gamma_R"),  	 g_avmBBConfig.gammaR[0]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "gamma_angle"),   g_avmBBConfig.gamma_angle[0]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "tip_homo_top"),  g_avmBBConfig.homo[0]);

		glBindVertexArray(VAO[0]);

		glUniform1i(glGetUniformLocation(programHandleMain[0], "windows"), AVM_3D);

		glUniform1i(glGetUniformLocation(programHandleMain[0], "cameraPosition"), cameraPosition);

		glUniform1f(glGetUniformLocation(programHandleMain[0], "yRervsed"), yReverse);

		glUniform1i(glGetUniformLocation(programHandleMain[0], "opendoorMode"), avm_info->draw_opendoor_mode);
		glUniform3fv(glGetUniformLocation(programHandleMain[0], "opendoorColor"), 1, avm_info->opendoor_color);

		if (windows == AVM_2D)
			glDrawArrays(GL_TRIANGLES, 0, avm3D[0].num_2d);
		else
			glDrawArrays(GL_TRIANGLES, 0, avm3D[0].num); // F_index_num
		glBindVertexArray(0);
		break;
	case 1:
		glUseProgram(programHandleMain[0]); 

		glUni_SamplerYFront = glGetUniformLocation(programHandleMain[0], "Texture");
		glUniform1i(glUni_SamplerYFront, BACK_CAMERA); // 指定samplerY對GL_TEXTURE 5 進行採樣
		glActiveTexture(GL_TEXTURE0 + BACK_CAMERA);
		#if PARAM_FISHEYE_BLUR_FUNCTION
			glBindTexture(GL_TEXTURE_2D,blur->textureID[FISHEYE_REAR_BLUR_TEXTURE]);
		#else
			glBindTexture(GL_TEXTURE_2D, g_texYId[1]);
		#endif

		glUniform1f(glGetUniformLocation(programHandleMain[0], "avg_L"),    	 g_avmBBConfig.avg_part1[1]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "avg_R"),    	 g_avmBBConfig.avg_part2[1]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "diff_L"),   	 g_avmBBConfig.diff_part1[1]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "diff_R"),   	 g_avmBBConfig.diff_part2[1]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "gamma_L"),  	 g_avmBBConfig.gammaL[1]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "gamma_R"),  	 g_avmBBConfig.gammaR[1]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "gamma_angle"),   g_avmBBConfig.gamma_angle[1]);
		glUniform1f(glGetUniformLocation(programHandleMain[0], "tip_homo_top"),  g_avmBBConfig.homo[1]);

		glBindVertexArray(VAO[1]);
		//(avminfo_door[4] == 1)?3:AVM_3D
		glUniform1i(glGetUniformLocation(programHandleMain[0], "windows"), (avminfo_door[4] == 1 && avm_info->draw_opendoor_mode != -1)?3:AVM_3D);

		glUniform1i(glGetUniformLocation(programHandleMain[0], "cameraPosition"), cameraPosition);

		glUniform1f(glGetUniformLocation(programHandleMain[0], "yRervsed"), yReverse);
		glUniform1i(glGetUniformLocation(programHandleMain[0], "opendoorMode"), avm_info->draw_opendoor_mode);
		glUniform3fv(glGetUniformLocation(programHandleMain[0], "opendoorColor"), 1, avm_info->opendoor_color);
		if (windows == AVM_2D)
			glDrawArrays(GL_TRIANGLES, 0, avm3D[1].num_2d);
		else
			glDrawArrays(GL_TRIANGLES, 0, avm3D[1].num); 
		glBindVertexArray(0);
		break;
	case 2:
		glUseProgram(programHandleMain[1]);

		glUni_matrixLocation = glGetUniformLocation(programHandleMain[1], "transformatiomMatrix");
		glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, rotateReslut); // Rotate3

		// glUniform3fv(glGetUniformLocation(programHandleMain[1], "rgbtotal_diffLR"), 1, rgb_diff13);

		glUniform1i(glGetUniformLocation(programHandleMain[1], "version"),       cur_info->BB_VERSION);
		glUniform1i(glGetUniformLocation(programHandleMain[1], "HOMO_flag"),     1);

		glUniform1f(glGetUniformLocation(programHandleMain[1], "diff"),   		 g_avmBBConfig.diff[2]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "avg"),    		 g_avmBBConfig.avg[2]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "diff_F"),   	 g_avmBBConfig.diff_part1[2]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "diff_B"),   	 g_avmBBConfig.diff_part2[2]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "gamma_L"),  	 g_avmBBConfig.gammaL[2]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "gamma_R"),  	 g_avmBBConfig.gammaR[2]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "gamma_angle"),   g_avmBBConfig.gamma_angle[2]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "tip_homo_top"),  g_avmBBConfig.homo[2]);



		glUni_SamplerYRight = glGetUniformLocation(programHandleMain[1], "Texture");
		glUniform1i(glUni_SamplerYRight, LEFT_CAMERA); 
		glActiveTexture(GL_TEXTURE0 + LEFT_CAMERA);
		#if PARAM_FISHEYE_BLUR_FUNCTION
			glBindTexture(GL_TEXTURE_2D,blur->textureID[FISHEYE_LEFT_BLUR_TEXTURE]);
		#else
			glBindTexture(GL_TEXTURE_2D, g_texYId[2]);
		#endif

		glUniform1i(glGetUniformLocation(programHandleMain[1], "TextureColor"), MAIN_COLOR2);
		glActiveTexture(GL_TEXTURE0 + MAIN_COLOR2);
    	glBindTexture(GL_TEXTURE_2D, g_tex_color[0]);

		glBindVertexArray(VAO[2]);
		glUniform1i(glGetUniformLocation(programHandleMain[1], "windows"), 
		(avminfo_door[1] == 1 || avminfo_door[3] == 1)?((avm_info->draw_opendoor_mode != -1)?3:AVM_3D):AVM_3D);
		glUniform1i(glGetUniformLocation(programHandleMain[1], "cameraPosition"), cameraPosition);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "yRervsed"), yReverse);
		glUniform1i(glGetUniformLocation(programHandleMain[1], "opendoorMode"), avm_info->draw_opendoor_mode);
		glUniform3fv(glGetUniformLocation(programHandleMain[1], "opendoorColor"), 1, avm_info->opendoor_color);
		if (windows == AVM_2D)
			glDrawArrays(GL_TRIANGLES, 0, avm3D[2].num_2d);
		else
			glDrawArrays(GL_TRIANGLES, 0, avm3D[2].num); 

		glBindVertexArray(0);
		break;
	case 3:
		glUseProgram(programHandleMain[1]); 
		glUniform1i(glGetUniformLocation(programHandleMain[1], "version"),       cur_info->BB_VERSION);
		glUniform1i(glGetUniformLocation(programHandleMain[1], "HOMO_flag"),     1);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "diff"),   		 g_avmBBConfig.diff[3]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "avg"),    		 g_avmBBConfig.avg[3]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "diff_F"),   	 g_avmBBConfig.diff_part1[3]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "diff_B"),   	 g_avmBBConfig.diff_part2[3]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "gamma_L"),  	 g_avmBBConfig.gammaL[3]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "gamma_R"),  	 g_avmBBConfig.gammaR[3]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "gamma_angle"),   g_avmBBConfig.gamma_angle[3]);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "tip_homo_top"),  g_avmBBConfig.homo[3]);

		glUni_SamplerYRight = glGetUniformLocation(programHandleMain[1], "Texture");
		glUniform1i(glUni_SamplerYRight, RIGHT_CAMERA); 
		glActiveTexture(GL_TEXTURE0 + RIGHT_CAMERA);
		#if PARAM_FISHEYE_BLUR_FUNCTION
			glBindTexture(GL_TEXTURE_2D,blur->textureID[FISHEYE_RIGHT_BLUR_TEXTURE]);
		#else
			glBindTexture(GL_TEXTURE_2D, g_texYId[3]);
		#endif

		glUniform1i(glGetUniformLocation(programHandleMain[1], "TextureColor"), MAIN_COLOR2);
		glActiveTexture(GL_TEXTURE0 + MAIN_COLOR2);
    	glBindTexture(GL_TEXTURE_2D, g_tex_color[0]);

		glBindVertexArray(VAO[3]);

		glUniform1i(glGetUniformLocation(programHandleMain[1], "windows"), 
		(avminfo_door[0] == 1 || avminfo_door[2] == 1)?((avm_info->draw_opendoor_mode != -1)?3:AVM_3D):AVM_3D);
		glUniform1i(glGetUniformLocation(programHandleMain[1], "cameraPosition"), cameraPosition);
		glUniform1f(glGetUniformLocation(programHandleMain[1], "yRervsed"), yReverse);
		glUniform3fv(glGetUniformLocation(programHandleMain[1], "opendoorColor"), 1, avm_info->opendoor_color);
		if (windows == AVM_2D)
			glDrawArrays(GL_TRIANGLES, 0, avm3D[3].num_2d);
		else
			glDrawArrays(GL_TRIANGLES, 0, avm3D[3].num); 

		glBindVertexArray(0);
		break;
	}
}

void draw_3d_avm(float *rotateReslut, avm3d_windows_t windows, int cameraPosition)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	#if (PLAFORM == NT98690)
	glFrontFace(GL_CCW);
	#else
	glFrontFace(GL_CW);
	#endif
	// glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,GL_ONE, GL_ONE);
	// glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA,GL_ONE, GL_ONE);
	draw_3d_avm_around(rotateReslut, 0, cameraPosition, CAM_NUM_LEFT);
	draw_3d_avm_around(rotateReslut, 0, cameraPosition, CAM_NUM_RIGHT);

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	draw_3d_avm_around(rotateReslut, 0, cameraPosition, CAM_NUM_FRONT);
	draw_3d_avm_around(rotateReslut, 0, cameraPosition, CAM_NUM_BACK);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

// test_shadow
void draw_car_shadow(float *rotateReslut)
{
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	//glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(programHandleCarShadow);

	glUni_matrixLocation = glGetUniformLocation(programHandleCarShadow, "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, rotateReslut);
	Scope scope =
	{
		{0, 0},
		{1, 1},
	};
	glUniform2fv(glGetUniformLocation(programHandleCarShadow, "startTexCoord"), 1, &scope.start[0]);
	glUniform2fv(glGetUniformLocation(programHandleCarShadow, "endTexCoord"), 1, &scope.end[0]);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "baseColorFlag"), 0);
	glUniform1f(glGetUniformLocation(programHandleCarShadow, "yRervsed"), yReverse);
	glUniform3fv(glGetUniformLocation(programHandleCarShadow, "color"), 1, chassisColor);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), glCarShadowWindow);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "flag_blur"), CarShadowBlurFlag);
	glBindVertexArray(VAOCarShadow[0]);

	if (glCarShadowWindow == 1)
	{
		glUniform1i(glGetUniformLocation(programHandleCarShadow, "Texture"), CAR_SHADOW);
		glActiveTexture(GL_TEXTURE0 + CAR_SHADOW);
		glBindTexture(GL_TEXTURE_2D, g_CarShadowtexId[0]);
	}


	glDrawArrays(GL_TRIANGLES, 0, carShadow.num); 
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

void draw_car_shadow_seethrough(float *rotateReslut, Scope scope)
{
	//g_shadowImgId[0]
	int windows = (autosys.current_info.seethrough_frameline_flag == 1)?3:1;
	int seethroughTextureID = autosys.program_context[SEETHROUGH].textureID[FBO_SEETHROUGH_TEXTURE];
	float *seethroughColor = &autosys.system_info.seetrough_frame_lineColor[0];
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(programHandleCarShadow);

	glUni_matrixLocation = glGetUniformLocation(programHandleCarShadow, "transformatiomMatrix");
	//shift, rotate, scale by user(4to1)
	float transfermatrix_uv[16] =
	{
		1.0f, 0.0f,	0.0f, 0.0f,
		0.0f, 1.0f,	0.0f, 0.0f,
		0.0f, 0.0f,	1.0f, 0.0f,
		0.0f, 0.0f,	0.0f, 1.0f,
	};


	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, rotateReslut);


	glUniform2fv(glGetUniformLocation(programHandleCarShadow, "startTexCoord"), 1, &scope.start[0]);
	glUniform2fv(glGetUniformLocation(programHandleCarShadow, "endTexCoord"), 1, &scope.end[0]);
	
	glUniform1f(glGetUniformLocation(programHandleCarShadow, "yRervsed"), yReverse);
	glUniform3fv(glGetUniformLocation(programHandleCarShadow, "color"), 1, seethroughColor);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), windows);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "baseColorFlag"), 1);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "flag_blur"), CarShadowBlurFlag);
	glBindVertexArray(VAOCarShadow[1]);

	glUniform1i(glGetUniformLocation(programHandleCarShadow, "Texture"), CAR_SHADOW);
	glActiveTexture(GL_TEXTURE0 + CAR_SHADOW);
	//glBindTexture(GL_TEXTURE_2D, g_shadowImgId[0]);
	glBindTexture(GL_TEXTURE_2D, seethroughTextureID);

	glUniform1i(glGetUniformLocation(programHandleCarShadow, "seethrough_frame"), UI_TEST1_enable);
	glActiveTexture(GL_TEXTURE0 + UI_TEST1_enable);
	glBindTexture(GL_TEXTURE_2D, g_shadowImgId[0]);


	glDrawArrays(GL_TRIANGLES, 0, 6); 
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

void draw_car_shadow_process_seethrough(float *rotateReslut, Scope scope)
{
	//int seethroughTextureID = autosys.program_context[SEETHROUGH].textureID[FBO_SEETHROUGH_TEXTURE];
	int seethroughTextureID = autosys.program_context[SEETHROUGH].textureID[FBO3_TEXTURE];
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(programHandleCarShadow);

	float transfermatrix[16] =
	{
		1.0f, 0.0f,	0.0f, 0.0f,
		0.0f, 1.0f,	0.0f, 0.0f,
		0.0f, 0.0f,	1.0f, 0.0f,
		0.0f, 0.0f,	0.0f, 1.0f,
	};

	glUni_matrixLocation = glGetUniformLocation(programHandleCarShadow, "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, transfermatrix);

	glUniform2fv(glGetUniformLocation(programHandleCarShadow, "startTexCoord"), 1, &scope.start[0]);
	glUniform2fv(glGetUniformLocation(programHandleCarShadow, "endTexCoord"), 1, &scope.end[0]);
	
	glUniform1f(glGetUniformLocation(programHandleCarShadow, "yRervsed"), yReverse);
	glUniform3fv(glGetUniformLocation(programHandleCarShadow, "color"), 1, chassisColor);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), 2);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "flag_blur"), CarShadowBlurFlag);
	glBindVertexArray(VAOCarShadow[2]);

	glUniform1i(glGetUniformLocation(programHandleCarShadow, "Texture"), CAR_SHADOW);
	glActiveTexture(GL_TEXTURE0 + CAR_SHADOW);
	glBindTexture(GL_TEXTURE_2D, seethroughTextureID);


	glDrawArrays(GL_TRIANGLES, 0, 6); 
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

void draw_Boarder(cam_num_t cameraNumber)
{
	#if 0
	glEnable(GL_DEPTH_TEST);
	float transfermatrix[16] =
		{
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
		};

	glUseProgram(programHandleCarShadow);
	// glUniform4fv(glGetUniformLocation(programHandle_UI[0], "color"), 1, ptr_color);

	// glBindVertexArray(VAO_ui_View2d[1]);

	glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), 3);
	glUni_matrixLocation = glGetUniformLocation(programHandleCarShadow, "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, &transfermatrix[0]); //

	glBindVertexArray(VAOCarShadow[2]);

	for (int ix = 0; ix < 4; ix++)
	{
		// glDrawArrays(GL_TRIANGLE_STRIP, ix*4, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, ix * 4, 4);
	}

	// glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);
	#endif
}

// void draw_icon(cam_num_t cameraNumber)
// {
// 	int enable_flag = 0;
// 	float transfermatrix[16] =
// 	{
// 		1.0f, 0.0f, 0.0f, 0.0f,
// 		0.0f, 1.0f, 0.0f, 0.0f,
// 		0.0f, 0.0f, 1.0f, 0.0f,
// 		0.0f, 0.0f, 0.0f, 1.0f,
// 	};
// 	int uiTex_l = 0;
// 	int uiTex_r = 0;
// 	//char ui_path_filename[][15] = {"oda_left","oda_right","bsd_left","bsd_right","ldw_left","ldw_right"};
// 	switch(cameraNumber)
// 	{
// 		case CAM_NUM_FRONT:

// 			uiTex_l = ldw_left;
// 			uiTex_r = ldw_right;
// 		break;
// 		case CAM_NUM_BACK:
// 			uiTex_l = bsd_left;
// 			uiTex_r = bsd_right;
// 			// uiTex_r = oda_left;
// 		break;
// 		case CAM_NUM_LEFT:
// 		break;
// 		case CAM_NUM_RIGHT:
// 		break;
// 	}

// 	// glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_enable_Texture"), UI_TEST1_enable + uiTex_l);
// 	// glActiveTexture(GL_TEXTURE0 + UI_TEST1_enable + uiTex_l);
// 	// glBindTexture(GL_TEXTURE_2D, g_UI_enable_texId[uiTex_l]);
// 	// glDrawArrays(GL_TRIANGLES, 0, 6);
//     // glBindVertexArray(0);

// 	// glUseProgram(programHandleCarShadow);
// 	// glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), 4);
// 	// glUniform1i(glGetUniformLocation(programHandleCarShadow, "enable_status"), enable_flag);
// 	// glUniform1f(glGetUniformLocation(programHandleCarShadow, "yRervsed"), 1.0f);

// 	// glUni_matrixLocation = glGetUniformLocation(programHandleCarShadow, "transformatiomMatrix");
// 	// glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, &transfermatrix[0]); //
//     // glBindVertexArray(VAOCarShadow[4]);

// 	// glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_enable_Texture"), UI_TEST1_enable + uiTex_r);
// 	// glActiveTexture(GL_TEXTURE0 + UI_TEST1_enable + uiTex_r);
// 	// glBindTexture(GL_TEXTURE_2D, g_UI_enable_texId[uiTex_r]);

// 	// if (oda_alert[0] == 1 || oda_alert[1] == 1){
// 	// 	uiTex_l = oda_right;
// 	// 	uiTex_r = oda_left;
// 	// }

// 	// uiTex_l = bsd_right;
// 	// uiTex_r = bsd_left;

// 	glUseProgram(programHandleCarShadow);
// 	glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), 4);
// 	glUniform1f(glGetUniformLocation(programHandleCarShadow, "yRervsed"), 1.0f);

// 	glUni_matrixLocation = glGetUniformLocation(programHandleCarShadow, "transformatiomMatrix");
// 	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, &transfermatrix[0]); //

// 	if (cameraNumber == CAM_NUM_FRONT){
// 		//left_side
// 		glBindVertexArray(VAOCarShadow[3]);
// 		if(ldw_alert[0] == 1)
// 		{
// 			enable_flag = 1;
// 			glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_enable_Texture"), UI_TEST1_enable + uiTex_l);
// 			glActiveTexture(GL_TEXTURE0 + UI_TEST1_enable + uiTex_l);
// 			glBindTexture(GL_TEXTURE_2D, g_UI_enable_texId[uiTex_l]);
// 		}
// 		else
// 		{
// 			glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_disable_Texture"), UI_TEST1_disable + uiTex_l);
// 			glActiveTexture(GL_TEXTURE0 + UI_TEST1_disable + uiTex_l);
// 			glBindTexture(GL_TEXTURE_2D, g_UI_disable_texId[uiTex_l]);
// 		}
// 		glUniform1i(glGetUniformLocation(programHandleCarShadow, "enable_status"), enable_flag);
// 		glDrawArrays(GL_TRIANGLES, 0, 6);
// 		glBindVertexArray(0);
// 		//right_side
// 		enable_flag = 0;
// 		glBindVertexArray(VAOCarShadow[4]);
// 		if(ldw_alert[1] == 1)
// 		{
// 			enable_flag = 1;
// 			glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_enable_Texture"), UI_TEST1_enable + uiTex_r);
// 			glActiveTexture(GL_TEXTURE0 + UI_TEST1_enable + uiTex_r);
// 			glBindTexture(GL_TEXTURE_2D, g_UI_enable_texId[uiTex_r]);
// 		}
// 		else
// 		{
// 			glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_disable_Texture"), UI_TEST1_disable + uiTex_r);
// 			glActiveTexture(GL_TEXTURE0 + UI_TEST1_disable + uiTex_r);
// 			glBindTexture(GL_TEXTURE_2D, g_UI_disable_texId[uiTex_r]);
// 		}
// 		glUniform1i(glGetUniformLocation(programHandleCarShadow, "enable_status"), enable_flag);
// 		glDrawArrays(GL_TRIANGLES, 0, 6);
// 		glBindVertexArray(0);
// 	}
// 	else if (cameraNumber == CAM_NUM_BACK){
// 		//left_side
// 		glBindVertexArray(VAOCarShadow[3]);
// 		if (oda_alert[1] == 1){
// 			uiTex_l = oda_left;
// 		}
// 		else{
// 			uiTex_l = bsd_left;
// 		}
// 		if (oda_alert[0] == 1){
// 			uiTex_r = oda_right;
// 		}
// 		else{
// 			uiTex_r = bsd_right;
// 		}

// 		if(bsd_alert[1] == 1 || oda_alert[1] == 1)
// 		{
// 			enable_flag = 1;
// 			glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_enable_Texture"), UI_TEST1_enable + uiTex_l);
// 			glActiveTexture(GL_TEXTURE0 + UI_TEST1_enable + uiTex_l);
// 			glBindTexture(GL_TEXTURE_2D, g_UI_enable_texId[uiTex_l]);
// 		}
// 		else
// 		{
// 			glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_disable_Texture"), UI_TEST1_disable + uiTex_l);
// 			glActiveTexture(GL_TEXTURE0 + UI_TEST1_disable + uiTex_l);
// 			glBindTexture(GL_TEXTURE_2D, g_UI_disable_texId[uiTex_l]);
// 		}
// 		glUniform1i(glGetUniformLocation(programHandleCarShadow, "enable_status"), enable_flag);
// 		glDrawArrays(GL_TRIANGLES, 0, 6);
// 		glBindVertexArray(0);
// 		//right_side
// 		enable_flag = 0;
// 		glBindVertexArray(VAOCarShadow[4]);
// 		if(bsd_alert[0] == 1 || oda_alert[0] == 1)
// 		{
// 			enable_flag = 1;
// 			glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_enable_Texture"), UI_TEST1_enable + uiTex_r);
// 			glActiveTexture(GL_TEXTURE0 + UI_TEST1_enable + uiTex_r);
// 			glBindTexture(GL_TEXTURE_2D, g_UI_enable_texId[uiTex_r]);
// 		}
// 		else
// 		{
// 			glUniform1i(glGetUniformLocation(programHandleCarShadow, "ui_disable_Texture"), UI_TEST1_disable + uiTex_r);
// 			glActiveTexture(GL_TEXTURE0 + UI_TEST1_disable + uiTex_r);
// 			glBindTexture(GL_TEXTURE_2D, g_UI_disable_texId[uiTex_r]);
// 		}
// 		glUniform1i(glGetUniformLocation(programHandleCarShadow, "enable_status"), enable_flag);
// 		glDrawArrays(GL_TRIANGLES, 0, 6);
// 		glBindVertexArray(0);
// 	}

// }

// void draw_ADAS_icon(cam_num_t cameraNumber)
// {
// 	int flag = 1;
// 	glClear(GL_DEPTH_BUFFER_BIT);

// 	if (mod_alert[cameraNumber] == 1)
// 	{
// 		draw_Boarder(cameraNumber);
// 	}
// 	if (cameraNumber == CAM_NUM_FRONT || cameraNumber == CAM_NUM_BACK)
// 	{
// 		draw_icon(cameraNumber);
// 	}
// }


void draw_camera_warning(int position)
{
#if 0
		float transfermatrix[16] =
		{
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
		};

	glUseProgram(programHandleCarShadow);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), 2);
	glUni_matrixLocation = glGetUniformLocation(programHandleCarShadow, "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, &transfermatrix[0]); //



	glUniform1i(glGetUniformLocation(programHandleCarShadow, "Texture_adas_ims"), CAR_SHADOW);
	glActiveTexture(GL_TEXTURE0 + CAR_SHADOW);
	glBindTexture(GL_TEXTURE_2D, g_camera_warning[0]);


	glBindVertexArray(VAOCameraWarning[position]);

	// for (int ix = 0; ix < 4; ix++)
	// {
	// 	glDrawArrays(GL_TRIANGLE_STRIP, ix * 4, 4);
	// }

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	#endif
}

void draw_cam_correct_2d(float *rotateReslut, cam_view_t detemineFisheye, cam_num_t cameraNumber, int style)
{

	int trianglesNumber = 0;

	int camProNum = 0;
	switch (cameraNumber)
	{
	case CAM_NUM_FRONT:
	case CAM_NUM_BACK:
		camProNum = 0;
		break;
	case CAM_NUM_LEFT:
	case CAM_NUM_RIGHT:
	case CAM_NUM_LEFT_BSD:
	case CAM_NUM_RIGHT_BSD:
		camProNum = 1;
		break;
	}
	glUseProgram(programHandleMain[camProNum]);

	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "windows"), CORRECT_CAMERA);
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "cameraPosition"), 0);
	glUniform1f(glGetUniformLocation(programHandleMain[camProNum], "yRervsed"), yReverse);

	glUni_matrixLocation = glGetUniformLocation(programHandleMain[camProNum], "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, rotateReslut);									
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Texture"), FRONT_CAMERA + cameraNumber);
	glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA + cameraNumber);
	glBindTexture(GL_TEXTURE_2D, g_texYId[cameraNumber]);
	/*upload texture*/
	if(flag_sceneview_updata_count[cameraNumber] != style && true)
	{
		char read_file_vec_2d_path[1024];
		char read_file_vec_2d_flash_path[1024];
		char *cam_name[] = {"f", "b", "l", "r"};

		sprintf(read_file_vec_2d_path, "EVSTable/sceneView/obj_%s_%d.bin", cam_name[cameraNumber], style);
		get_file_path(read_file_vec_2d_path, 1, read_file_vec_2d_flash_path, sizeof(read_file_vec_2d_flash_path));
		if(FlashReadDataStatus(read_file_vec_2d_flash_path) == 0)
		{
			read_vert_text_bin(read_file_vec_2d_path, &view2d[cameraNumber][style]);
		}
		else
		{
			AVM_LOGI("[ReadFile] %s in flash.\n", read_file_vec_2d_path);
			read_scene_vert_text_bin_wCRC(read_file_vec_2d_path, &view2d[cameraNumber][style]);
		}

		glBindVertexArray(VAOViewCorrect2d[cameraNumber]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[cameraNumber][0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, view2d[cameraNumber][style].num * 3 * sizeof(float), view2d[cameraNumber][style].vertice);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_correct[cameraNumber][1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, view2d[cameraNumber][style].num * 2 * sizeof(float), view2d[cameraNumber][style].texture);
	}
	glBindVertexArray(VAOViewCorrect2d[cameraNumber]);
	//glBindVertexArray(VAOViewCorrect2d[cameraNumber * 5 + style]);
	trianglesNumber = view2d[cameraNumber][style].num;
	glDrawArrays(GL_TRIANGLES, 0, trianglesNumber);
	glBindVertexArray(0);
	if(flag_sceneview_updata_count[cameraNumber] != style && true)
	{
		flag_sceneview_updata_count[cameraNumber] = style;
		if(view2d[cameraNumber][style].num != 0)
		{
			free(view2d[cameraNumber][style].vertice);
			free(view2d[cameraNumber][style].texture);
		}
	}
}


/* ADAS AND IMS */
void draw_adas_ims(cam_view_t detemineFisheye, cam_num_t cameraNumber)
{
	int trianglesNumber;

	glUseProgram(programHandleCarShadow);

	float transfermatrix[16] =
		{
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
		};

	glUni_matrixLocation = glGetUniformLocation(programHandleCarShadow, "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, transfermatrix);

	// glUniform1i(glGetUniformLocation(programHandleCarShadow, "Texture"), ADAS_IMS);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "Texture_adas_ims"), ADAS_IMS);
	glUniform1f(glGetUniformLocation(programHandleCarShadow, "yRervsed"), 1.0f);
	glActiveTexture(GL_TEXTURE0 + ADAS_IMS);
	glBindTexture(GL_TEXTURE_2D, g_texYId[7]);
	// glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), glCarShadowWindow);
	glUniform1i(glGetUniformLocation(programHandleCarShadow, "windows"), 2);

	if (detemineFisheye == CAM_VIEW_FISHEYE)
	{ // 4to1 fisheye Enter this
		// glBindVertexArray(VAOView2d[0]);
		glBindVertexArray(VAOCarShadow[1]);
	}
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void draw_cam_2d_raw_yinverse(cam_view_t detemineFisheye, cam_num_t cameraNumber, void *cameraParameter)
{
	int trianglesNumber = 0;

	int camProNum = 0;
	switch (cameraNumber)
	{
	case CAM_NUM_FRONT:
	case CAM_NUM_BACK:
		camProNum = 0;
		break;
	case CAM_NUM_LEFT:
	case CAM_NUM_RIGHT:
	case CAM_NUM_LEFT_BSD:
	case CAM_NUM_RIGHT_BSD:
		camProNum = 1;
		break;
	}

	Scope scope =
	{
		{0.0f, 0.0f},
		{1.0f, 1.0f},
	};
	

	float transfermatrix[16] =
	{
		1.0f,  0.0f, 0.0f,	0.0f,
		0.0f,  1.0f, 0.0f,	0.0f,
		0.0f,  0.0f, 1.0f,	0.0f,
		0.0f,  0.0f, 0.0f,	1.0f,
	};

	glUseProgram(programHandleMain[camProNum]);

	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "windows"), CAMERA);
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "cameraPosition"), NORMAL);
	glUniform1f(glGetUniformLocation(programHandleMain[camProNum], "yRervsed"), yReverse);

	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Texture"), FRONT_CAMERA + cameraNumber); // 指定samplerY對GL_TEXTURE 4 進行採樣
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Mirror"), 0);
	glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA + cameraNumber);
	glBindTexture(GL_TEXTURE_2D, g_texYId[cameraNumber]);

	glUni_matrixLocation = glGetUniformLocation(programHandleMain[camProNum], "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, &transfermatrix[0]); 
	glUniform2fv(glGetUniformLocation(programHandleMain[camProNum], "startTexCoord"), 1, &scope.start[0]);
	glUniform2fv(glGetUniformLocation(programHandleMain[camProNum], "endTexCoord"), 1, &scope.end[0]);

	if (detemineFisheye == CAM_VIEW_FISHEYE)
	{
		glBindVertexArray(VAOView2d[camProNum]); // 2D camera fish view
		trianglesNumber = 6;
	}

	glDrawArrays(GL_TRIANGLES, 0, trianglesNumber);
	glBindVertexArray(0);
}

void draw_cam_2d(cam_view_t detemineFisheye, cam_num_t cameraNumber, void *cameraParameter)
{
	// Verify o3 & o0 answer is different
	// sacle will became "0"

	stCamera *data;
	stCamera base =
		{
			.para.shift_x = 0,
			.para.shift_y = 0,
			.para.shift_z = 0,
			.para.start_x = 0,
			.para.start_y = 0,
			.para.end_y = imageHeight,
			.para.end_x = imageWidth,
		};

	data = (cameraParameter != NULL) ? (stCamera *)cameraParameter : &base;

	// if (cameraParameter != NULL)
	// {
	// 	data = (stCamera *)cameraParameter;
	// }
	// else
	// {
	// 	data = &base;
	// }

	int trianglesNumber = 0;

	int camProNum = 0;
	switch (cameraNumber)
	{
	case CAM_NUM_FRONT:
	case CAM_NUM_BACK:
		camProNum = 0;
		break;
	case CAM_NUM_LEFT:
	case CAM_NUM_RIGHT:
	case CAM_NUM_LEFT_BSD:
	case CAM_NUM_RIGHT_BSD:
		camProNum = 1;
		break;
	}

	int mirror = (cameraNumber == CAM_NUM_BACK) ? 2 : 0;

	float start_x = data->para.start_x, start_y = data->para.start_y;
	float end_x = (data->para.end_x <= 0) ? imageWidth : data->para.end_x, end_y = (data->para.end_y <= 0) ? imageHeight : data->para.end_y;

	Scope scope =
		{
			{start_x / (float)imageWidth, start_y / (float)imageHeight},
			{end_x / (float)imageWidth, end_y / (float)imageHeight},
		};

	if (cameraNumber == CAM_NUM_BACK)
	{
		// for mirror used
		scope.start[0] = 1.0f - scope.start[0];
		scope.end[0] = 1.0f - scope.end[0];
	}
	
	float sacle = (float)(data->para.shift_z <= 0.0) ? 1.0 : data->para.shift_z;

	float transfermatrix[16] =
		{
			1.0f * sacle,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f * sacle,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f + data->para.shift_x,
			0.0f + data->para.shift_y,
			0.0f,
			1.0f,
		};

	glUseProgram(programHandleMain[camProNum]);

	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "windows"), CAMERA);
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "cameraPosition"), NORMAL);
	glUniform1f(glGetUniformLocation(programHandleMain[camProNum], "yRervsed"), yReverse);

	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Texture"), FRONT_CAMERA + cameraNumber); // 指定samplerY對GL_TEXTURE 4 進行採樣
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Mirror"), mirror);
	glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA + cameraNumber);
	glBindTexture(GL_TEXTURE_2D, g_texYId[cameraNumber]);

	glUni_matrixLocation = glGetUniformLocation(programHandleMain[camProNum], "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, &transfermatrix[0]); //
	glUniform2fv(glGetUniformLocation(programHandleMain[camProNum], "startTexCoord"), 1, &scope.start[0]);
	glUniform2fv(glGetUniformLocation(programHandleMain[camProNum], "endTexCoord"), 1, &scope.end[0]);

	if (detemineFisheye == CAM_VIEW_FISHEYE)
	{
		glBindVertexArray(VAOView2d[camProNum + mirror]); // 2D camera fish view
		trianglesNumber = 6;
	}
	else if (detemineFisheye == CAM_VIEW_CORRECT)
	{
		// glBindVertexArray(VAOViewCorrect2d[cameraNumber]);
		// trianglesNumber = view2d[cameraNumber].num;
	}
	else if (detemineFisheye == CAM_VIEW_FISHEYE_TRANS)
	{
		glBindVertexArray(VAOViewUnique2d[cameraNumber]);
		switch (cameraNumber)
		{
		case CAM_NUM_FRONT: // Alley view
			trianglesNumber = 6;
			break;
		case CAM_NUM_BACK: // DR back
			trianglesNumber = 6;
			break;
		case CAM_NUM_LEFT: // Left tire
			trianglesNumber = 6;
			break;
		case CAM_NUM_RIGHT: // Right tire
			trianglesNumber = 6;
			break;
		case CAM_NUM_LEFT_BSD: // Left tire
			trianglesNumber = 6;
			break;
		case CAM_NUM_RIGHT_BSD: // Right tire
			trianglesNumber = 6;
			break;
		default:
			break;
		}
	}
	else if (detemineFisheye == CAM_VIEW_ADAS)
	{
		glBindVertexArray(VAOViewAdas[cameraNumber]);
		switch (cameraNumber)
		{
		case CAM_NUM_FRONT:
			trianglesNumber = 6; // stopngo
			break;
		case CAM_NUM_LEFT_BSD:
			trianglesNumber = 6; // oda l
			break;
		case CAM_NUM_RIGHT_BSD:
			trianglesNumber = 6; // oda r
			break;
		default:
			break;
		}
	}
	else
	{
		trianglesNumber = 6;
	}

	glDrawArrays(GL_TRIANGLES, 0, trianglesNumber);
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Mirror"), 0); // reset status
	glBindVertexArray(0);
}


void draw_cam_fish(cam_view_t detemineFisheye, cam_num_t cameraNumber, cropped_view *cameraParameter, float fishRotateAngle)
{
	int trianglesNumber = 0;

	int camProNum = 0;
	switch (cameraNumber)
	{
	case CAM_NUM_FRONT:
	case CAM_NUM_BACK:
		camProNum = 0;
		break;
	case CAM_NUM_LEFT:
	case CAM_NUM_RIGHT:
	case CAM_NUM_LEFT_BSD:
	case CAM_NUM_RIGHT_BSD:
		camProNum = 1;
		break;
	}

	int windows        = CAMERA;
	int cameraPosition = NORMAL;
	int mirror = (cameraNumber == CAM_NUM_BACK) ? 2 : 0;

	float start_x = cameraParameter->start_x, start_y = cameraParameter->start_y;
	float end_x = (cameraParameter->end_x <= 0) ? imageWidth : cameraParameter->end_x, end_y = (cameraParameter->end_y <= 0) ? imageHeight : cameraParameter->end_y;

	Scope scope =
		{
			{start_x / (float)imageWidth, start_y / (float)imageHeight},
			{end_x / (float)imageWidth, end_y / (float)imageHeight},
		};
// AVM_LOGI("scope end [%f,%f] para[%f,%f], zoom[%f]\n", 
// scope.end[0],scope.end[1],
// cameraParameter->end_x,
// cameraParameter->end_y,
// cameraParameter->shift_z);
	if (cameraNumber == CAM_NUM_BACK)
	{
		// for mirror used
		scope.start[0] = 1.0f - scope.start[0];
		scope.end[0]   = 1.0f - scope.end[0];
	}

	float sacle = (float)(cameraParameter->shift_z <= 0.0) ? 1.0 : cameraParameter->shift_z;

	float transfermatrix[16] =
		{
		1.0f * sacle,0.0f,0.0f,	0.0f,
		0.0f,1.0f * sacle,0.0f, 0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.0f + cameraParameter->shift_x,0.0f + cameraParameter->shift_y,0.0f,1.0f,
		};

	if(detemineFisheye == CAM_VIEW_FISHEYE_CYLINDRICAL)
	{
		camProNum      = 1;
		windows        = 2;
		cameraPosition = 4;
		int switchPosition = 0;
    	float transferM[16], scaleM[16], transM[16];
    	CalcFisheyeMatrices(cameraNumber, transfermatrix, cameraParameter, transferM, scaleM, transM, &switchPosition);
		glUseProgram(programHandleMain[camProNum]);
		glUniformMatrix4fv(glGetUniformLocation(programHandleMain[camProNum], "ScopeArea.scaleMatrix"), 1, GL_FALSE, scaleM);
    	glUniformMatrix4fv(glGetUniformLocation(programHandleMain[camProNum], "ScopeArea.translationMatrix"), 1, GL_FALSE, transM);
		memcpy(transfermatrix, transferM, sizeof(float) * 16);
	}
	//cameraRotate
	{
		float rotateMatrix[16], resulttransfermatrix[16];
		myCreateRotate4(2, fishRotateAngle, rotateMatrix);
		myMatrixMultiplyby4(resulttransfermatrix, rotateMatrix, transfermatrix);
		memcpy(transfermatrix, resulttransfermatrix, sizeof(float) * 16);

	}
	glUseProgram(programHandleMain[camProNum]);

	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "windows"), windows);
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "cameraPosition"), cameraPosition);
	glUniform1f(glGetUniformLocation(programHandleMain[camProNum], "yRervsed"), yReverse);

	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Texture"), FRONT_CAMERA + cameraNumber); // 指定samplerY對GL_TEXTURE 4 進行採樣
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Mirror"), mirror);
	glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA + cameraNumber);
	glBindTexture(GL_TEXTURE_2D, g_texYId[cameraNumber]);

	glUni_matrixLocation = glGetUniformLocation(programHandleMain[camProNum], "transformatiomMatrix");
	glUniformMatrix4fv(glUni_matrixLocation, 1, GL_FALSE, &transfermatrix[0]); //
	glUniform2fv(glGetUniformLocation(programHandleMain[camProNum], "startTexCoord"), 1, &scope.start[0]);
	glUniform2fv(glGetUniformLocation(programHandleMain[camProNum], "endTexCoord"), 1, &scope.end[0]);

	if (detemineFisheye == CAM_VIEW_FISHEYE)
	{
		glBindVertexArray(VAOView2d[camProNum + mirror]); // 2D camera fish view
		trianglesNumber = 6;
	}
	else if (detemineFisheye == CAM_VIEW_CORRECT)
	{
		// glBindVertexArray(VAOViewCorrect2d[cameraNumber]);
		// trianglesNumber = view2d[cameraNumber][style].num;
	}
	else if (detemineFisheye == CAM_VIEW_FISHEYE_TRANS)
	{
		glBindVertexArray(VAOViewUnique2d[cameraNumber]);
		switch (cameraNumber)
		{
		case CAM_NUM_FRONT: // Alley view
			trianglesNumber = 6;
			break;
		case CAM_NUM_BACK: // DR back
			trianglesNumber = 6;
			break;
		case CAM_NUM_LEFT: // Left tire
			trianglesNumber = 6;
			break;
		case CAM_NUM_RIGHT: // Right tire
			trianglesNumber = 6;
			break;
		case CAM_NUM_LEFT_BSD: // Left tire
			trianglesNumber = 6;
			break;
		case CAM_NUM_RIGHT_BSD: // Right tire
			trianglesNumber = 6;
			break;
		default:
			break;
		}
	}
	else if (detemineFisheye == CAM_VIEW_ADAS)
	{
		glBindVertexArray(VAOViewAdas[cameraNumber]);
		switch (cameraNumber)
		{
		case CAM_NUM_FRONT:
			trianglesNumber = 6; // stopngo
			break;
		case CAM_NUM_LEFT_BSD:
			trianglesNumber = 6; // oda l
			break;
		case CAM_NUM_RIGHT_BSD:
			trianglesNumber = 6; // oda r
			break;
		default:
			break;
		}
	}
	else if(detemineFisheye == CAM_VIEW_FISHEYE_CYLINDRICAL)
	{
		glBindVertexArray(VAO_Cylindrical2d[cameraNumber]);
		trianglesNumber = Cylindrical2d[cameraNumber].num;
	}
	else
	{
		trianglesNumber = 6;
	}

	glDrawArrays(GL_TRIANGLES, 0, trianglesNumber);
	glUniform1i(glGetUniformLocation(programHandleMain[camProNum], "Mirror"), 0); // reset status
	glBindVertexArray(0);
}


void screenshot_file(void)
{
	FILE *fpFWrite;
	FILE *fpBWrite;
	FILE *fpLWrite;
	FILE *fpRWrite;

	int fileSize;
	system("mkdir /media/flash/userdata/image");
	fpFWrite = fopen("/media/flash/userdata/image/front.yuv", "wb");
	fpBWrite = fopen("/media/flash/userdata/image/back.yuv", "wb");
	fpLWrite = fopen("/media/flash/userdata/image/left.yuv", "wb");
	fpRWrite = fopen("/media/flash/userdata/image/right.yuv", "wb");

	fileSize = imageWidth * imageHeight * 2;

	fwrite(cameraFileFrontBuf, 1, fileSize, fpFWrite);
	fwrite(cameraFileBackBuf, 1, fileSize, fpBWrite);
	fwrite(cameraFileLeftBuf, 1, fileSize, fpLWrite);
	fwrite(cameraFileRightBuf, 1, fileSize, fpRWrite);

	fclose(fpFWrite);
	fclose(fpBWrite);
	fclose(fpLWrite);
	fclose(fpRWrite);
}
