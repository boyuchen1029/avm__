/**
 *******************************************************************************
 * @file : program_car.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211116 0.1.0 Linda.
 *******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/resource.h>
#include <unistd.h>
#include "../../system.h"
#include "user_control/ui.h"
#include "avm/GLHeader.h"
#include "avm/camera_matrix.h"
#include "canbus.h"
#include "program/program_car.h"
#include "avm/fp_source.h"
#include "avm/maths.h"
#include "avm/avm_support_funcs.h"
#include "avm/ui_memory_func.h"
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "avm/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include "avm/stb_image_resize.h"
#include "../../autosys/autosys.h"
#include "../../autosys/autosys_IVI.h"

// #include CAR_BODY
// #include TIRE_LB
// #include TIRE_RB
// #include TIRE_LF
// #include TIRE_RF
// #include VERTICE_NUM

// #include Glass_h

#include "gl/glShaderUtils.h"
#include "gl/glFunUtils.h"
#include FRAGSHADER
#include VERTSHADER

// GL reverse line define
static GLuint programHandleCar[2]  = {0};
static GLuint programHandleCarOIT  = 0;
static GLuint programHandleOITMain = 0;

// car model texture

static unsigned char * carTexSpecular;
static unsigned char * carTexGlossisess;

GLuint** VAOCar;
int Car_idx = 0;
int lightNum[2];
float CarSize[2];
float* lightPos[2];
float* Brightness[2];
// GLint glUniMyTextureSampler = 0;

GLint glAttrCarVertex = 0;
GLint glAttrCarUV = 0;
GLint glAttrCarNor = 0;

GLuint vertexbufferCar[2][MAX_CARMODEL_OBJ_NUM] = {0}; //0:3D Car, 1: 2D Car

GLuint uvbufferCar[2][MAX_CARMODEL_OBJ_NUM] = {0};
GLuint norbufferCar[2][MAX_CARMODEL_OBJ_NUM] = {0};


//Car2D
GLuint VAOCar2d[2];
GLuint vertbuffer2d[2] = {0}; //0:LB, 1:RB, 2:LF, 3:RF
GLuint textbuffer2d[2] = {0};
//Car3D image(2D)
int car_obj_flag[MAX_CARMODEL_OBJ_NUM] = {0};
int texture_3D_mode = 1;
static GLuint g_texCar3d[MAX_PAGE_NUM];
static GLuint g_texCover3d[MAX_PAGE_NUM];
static GLuint g_existedCover3d[MAX_PAGE_NUM];
static GLuint g_memCarBodyIx;
GLuint VAOCar3d[1];
GLuint vertbuffer3d[1] = {0}; //0:LB, 1:RB, 2:LF, 3:RF
GLuint textbuffer3d[1] = {0};


GLuint quadVAO;
GLuint quadVBO;
// specular
static GLuint  g_texCar2d;
static GLuint* g_texCar[2];
static GLuint g_texCarSpecular[1];
static GLuint g_texCarGlossisess[1];
static GLuint g_texSky[1];
static GLuint CarTotal[2] = {0};
static GLuint  g_texStaticCar[2];

static car_coord_3d_t* CarVert[2];

int Car_2d_W = 0;
int Car_2d_H = 0;

MEM_OCCUPY mem_stat2;

//Lamp
PACK_CAR_LAMP car_lampStatus[2];
char LAMP_dir[CAR_MODEL_NUMBER][256];
bool m_flag_flowing_lamp_uesd = false;
//end
struct timeval start2, end2;
static float originalMatrix[4][4] =
{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
};

//load 2d carModel axis parameter
float carModel2D_axis_density[2] = {0};
bool tmp_tireDepth = true;

/* only for testing 20240216*/
unsigned char *TestIMGData[MAX_CARMODEL_OBJ_NUM];
GLuint  Test_texCar2d[MAX_CARMODEL_OBJ_NUM] = {0};
GLuint  textureIds[MAX_CARMODEL_OBJ_NUM]; 
GLuint  pboIds[MAX_CARMODEL_OBJ_NUM]; 
void* pboadress[MAX_CARMODEL_OBJ_NUM];
struct timeval start1, end1;
void decode_floats_from_rgba(const char* image_path, float* float_data, int width, int height) {
 
//  int channels;
//     unsigned char* rgba_data = stbi_load(image_path, &width, &height, &channels, 4); 
//     if (!rgba_data) {
//         printf("Failed to load image\n");
//         return;
//     }
//     for (int i = 0; i < width * height; i++) {
//         uint32_t rgba_int =  ((uint32_t)rgba_data[i * 4 + 3] << 24) |
//                              ((uint32_t)rgba_data[i * 4 + 2] << 16) |
//                              ((uint32_t)rgba_data[i * 4 + 1] << 8) |
//                              ((uint32_t)rgba_data[i * 4]);
//         float_data[i] = *(float*)&rgba_int;
//     }

//     free(rgba_data);
}

void read_TESTIMG_func()
{
	gettimeofday(&start2, NULL);
	float* buffer = (float*)malloc(22044 * 2 * sizeof(float));
	FILE* fp = fopen("raw_alpha.bin", "rb");
    if(buffer == NULL || fp == NULL) return;
	fread(buffer, 22044*2, sizeof(float), fp);
	fclose(fp);
	printf("\033[31m bin buffer x[0] = %f, x[1] = %f, x[2] = %f, x[3] = %f, x[4] = %f\n \033[0m", buffer[0], buffer[2], buffer[4], buffer[6], buffer[8]);
	gettimeofday(&end2, NULL);
	printf("readbin time = %f\n",calculate_time(start2, end2));

	gettimeofday(&start2, NULL);
	int width, height, channels;
    //stbi_set_flip_vertically_on_load(1);
	unsigned char *data = stbi_load("outputX.png", &width, &height, &channels, 4); 
	memcpy(buffer, data, sizeof(float) * 22044);
	printf("\033[31m xpicture x[0] = %f, x[1] = %f, x[2] = %f, x[3] = %f, x[4] = %f\n \033[0m", buffer[0], buffer[1],buffer[2],buffer[3], buffer[4]);
	data = stbi_load("outputY.png", &width, &height, &channels, 4); 
	memcpy(buffer, data, sizeof(float) * 22044);
	printf("\033[31m xpicture y[0] = %f, y[1] = %f\n \033[0m", buffer[0], buffer[1]);
	gettimeofday(&end2, NULL);
	printf("decode png time = %f\n",calculate_time(start2, end2));
    free(buffer);
}




void* CPU_mapping_GPU_func_one(void *ptr)
{
    int start = (int)ptr;
    for(int ix = start; ix < start + 1 ; ix++)
    {
        if(pboadress[ix])
        {
            AVM_LOGI("=====================================\n");
            AVM_LOGI("Loading PBO is %d\n",ix);
            memcpy(pboadress[ix] ,TestIMGData[ix], sizeof(unsigned char*) * 1024*1024*4);
            AVM_LOGI("Loading PBO is %d\n",ix);
            AVM_LOGI("=====================================\n");
        }
        AVM_LOGI("UnLoading PBO is %d\n",ix);
    }
    pthread_exit(NULL);
}

void* CPU_mapping_GPU_func_two(void *ptr)
{
    int start = (int)ptr;
    for(int ix = start; ix < start + 2 ; ix++)
    {
        if(pboadress[ix])
        {
            AVM_LOGI("=====================================\n");
            memcpy(pboadress[ix] ,TestIMGData[ix], sizeof(unsigned char*) * 1024*1024*4);
            AVM_LOGI("Loading PBO is %d\n",ix);
            AVM_LOGI("=====================================\n");
        }
        AVM_LOGI("UnLoading PBO is %d\n",ix);
    }
    pthread_exit(NULL);
}

void loadingTESTIMG()
{
    return;
}
/*end*/

void init_car_program(void)
{
    AVM_LOGI("-----------------------------\n");
    // #if READ_OFFLINE_SHADER
    // LoadBinary2Program(&programHandleCar[0],"shader_program.bin_format_36805_1");
    // if(programHandleCar[0] == GL_NONE)
	// {
	// 	AVM_LOGI(LOG_RED("[ERROR]")"GL Loading CAR offline file error!!!\n");
	// 	AVM_LOGI("[WARNING] Using online program!!!\n");
	// 	programHandleCar[0]   = create_program(vertexShader_car_main, fragmentShader_car_main, 1); 
	// }
    // #else
    programHandleCar[0] = create_program(vertexShader_car_main, fragmentShader_car_main,1);
    write_offline_shader_program(programHandleCar[0], 1);
    // #endif
    programHandleCarOIT  = create_program(vertexShader_car_main, fragmentShader_car_main_oit,14);
    programHandleOITMain = create_program(vertShader_composite_oit, fragmentShader_composite_oit,15);
    AVM_LOGI("AVM program_Car[0]: %d \n",    programHandleCar[0]);
    AVM_LOGI("AVM program_CarOIT[0]: %d \n", programHandleCarOIT);
    AVM_LOGI("AVM program_OITMain[0]: %d \n", programHandleOITMain);
    AVM_LOGI("-----------------------------\n");
    gettimeofday(&start1, NULL);
}

void car_mem_malloc(void)
{

}

// void assign_lampdata(int id, char* item, int GLtex)
// {
//     AVM_LOGI("[*] assign the Lamp data\n");
//     //car_lampStatus
//     int class_num =  Classify_LAMP_max;
//     for(int ix = 0; ix < class_num; ix++)
//     {
//         if(strstr(item, car_lampStatus[id].Lamp_STD_obj_name[ix]))
//         {
//             car_lampStatus[id].Lamp_gl_ID[ix] = GLtex;
//             AVM_LOGI("[**] recording the [%s] in lampGLID [%d]\n", item, GLtex);
//             break;
//         };
//     }
// }

void assign_lampdata_formobj(int id, char* item, int GLtex)
{
    AVM_LOGI("[*] assign the Lamp data\n");
    //car_lampStatus
    int class_num =  Classify_LAMP_max;
    int lamp_main = 0;
    for(int ix = 0; ix < class_num; ix++)
    {
        if(strstr(item, car_lampStatus[id].Lamp_STD_obj_name[ix]))
        {
            for(int side = 0; side < Classify_LAMP_mini_max; side++)
            {
                if( strstr(item, Lamp_classify_name_mini[side]) )
                {
                    int lame_index = Classify_LAMP_mini_max * (lamp_main) + side;
                    car_lampStatus[id].Lamp_gl_ID[lame_index] = g_texCar[Car_idx][GLtex];
                    AVM_LOGI("[**] recording the [%s] in lampGLID [%d]\n", item, car_lampStatus[id].Lamp_gl_ID[lame_index]);
                    break;
                }
            }
        };
        lamp_main++;
    }
}

/*--------------------------*/
/*accelerator to init_carTex*/
/*--------------------------*/
int read_car_tex_accelerator_flag = 0;
int m_TexTotal = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    char IMGPATH[50][1024];
    unsigned char *imgdata[50];
    int w[50];
    int h[50];
    int strip[50];

    // gl
    float*  vertice[50];
    float*  texture[50];
    float*  normal[50];
    int     num[50];
    // 
    int envoke;
    int cur_read;
    int queue;
    int upload;
}ReadCAR_STACK;

typedef struct 
{
    void *src;
    void *dst;
    int size;
    /* data */
}Thread_mapping;


ReadCAR_STACK cartex_stack;
//int IMG_STACK[100] = {0};
void* CarTex_IMG_READ_func(void *ptr)
{
    while(1)
    {
        if(cartex_stack.envoke == 1) break;
        usleep(10);
    }
    stbi_set_flip_vertically_on_load(0);
    while(1)
    {
        pthread_mutex_lock(&lock);
        int current_index = cartex_stack.cur_read;
        int queue_index = cartex_stack.queue;
        pthread_mutex_unlock(&lock);
        if(current_index < car_lampStatus[Car_idx].maxGenIMG)
        {

            if(current_index >= queue_index) 
            {
                usleep(10); 
                continue;
            }
            AVM_LOGI("[***]current_index is [%d], queue_index[%d]\n", current_index, queue_index);

            int w1, h1, n;
            AVM_LOGI("[*****]CarTex_IMG_READ_func Read file   %s\n", cartex_stack.IMGPATH[current_index]);
            pthread_mutex_lock(&lock);
            cartex_stack.imgdata[current_index] = stbi_load(cartex_stack.IMGPATH[current_index], &w1, &h1, &n, 0);
            AVM_LOGI("img Size w = %d, h =%d\n", w1, h1);
            cartex_stack.w[current_index] = w1;
            cartex_stack.h[current_index] = h1;
            cartex_stack.strip[current_index] = n;
            cartex_stack.cur_read = cartex_stack.cur_read + 1;
            pthread_mutex_unlock(&lock);
        }
        else
        {
            break;
        }
    }

    AVM_LOGI("[****] CarTex_IMG_READ_func EXIT\n");

    pthread_exit(NULL);

}

void *CarTex_READ_BIN_func(void *ptr)
{
    FILE *fpBin = (FILE *)ptr;
    unsigned char * car;
    char* name;
    int TexTotal;
    int MaxPixels;
    fread(&TexTotal, 1, 1*sizeof(int), fpBin);
    fread(&MaxPixels, 1, 1*sizeof(int), fpBin);
    AVM_LOGI("Tex Total %d \n", TexTotal);
    AVM_LOGI("Tex Max Pixel %d \n", MaxPixels);
    g_texCar[Car_idx] = (GLuint*)malloc(TexTotal * sizeof(GLuint));
    name = malloc(sizeof(char)* 60);
    car = (unsigned char *)calloc(MaxPixels * sizeof(unsigned char), sizeof(unsigned char));
    car_lampStatus[Car_idx].maxGenIMG = TexTotal;
    cartex_stack.envoke = 1;
    m_TexTotal = TexTotal;
    for(int ix=0;ix<TexTotal;ix++)
    {
        int id = 0;
        int nameLen = 0;
        int w, h;

        fread(&id, 1, 1*sizeof(int), fpBin);
        fread(&nameLen, 1, 1*sizeof(int), fpBin);
        memset(name,0,sizeof(char)*60);
        fread(name, sizeof(char), nameLen, fpBin);
        fread(&w, 1, 1*sizeof(int), fpBin);
        fread(&h, 1, 1*sizeof(int), fpBin);
        //w = 4000; h = 4000;
        AVM_LOGI("Tex Id %d Len %d Name %s w %d h %d\n", id, nameLen, name, w, h);

        for(int iy=nameLen;iy>0;iy--)
        {
            if(name[iy]=='.')
            {
                name[iy]='.';
                name[iy+1]='p';
                name[iy+2]='n';
                name[iy+3]='g';
                break;
            }
            else
            {
                name[iy] = '\0';
            }
        }

        char dir[1024] = "\0";
        if (Car_idx == 0)
        {
            sprintf(dir, "%sEVSTable/car_model/%s", EVSTable_path[0],name);
        }
        else
        {
            sprintf(dir, "%sEVSTable/car_model/modelB/%s", EVSTable_path[0], name);
        }
        FILE * fpCar = fopen(dir,"rb");
        if(fpCar == NULL)
        {
            for (int iy = nameLen; iy > 0; iy--)
            {
                if (name[iy] == '.')
                {
                    name[iy] = '.';
                    name[iy + 1] = 'r';
                    name[iy + 2] = 'g';
                    name[iy + 3] = 'b';
                    break;
                }
                else
                {
                    name[iy] = '\0';
                }
            }
            if (Car_idx == 0)
            {
                sprintf(dir, "%sEVSTable/car_model/%s", EVSTable_path[0], name);
            }
            else
            {
                sprintf(dir, "%sEVSTable/car_model/modelB/%s", EVSTable_path[0], name);
            }
            fpCar = fopen(dir, "rb");

            fread(car, 1, w * h * 3, fpCar);

            glBindTexture(GL_TEXTURE_2D, g_texCar[Car_idx][ix]);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, car);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            fclose(fpCar);
        }
        else
        {
            if (Car_idx == 0)
            {
                sprintf(dir, "%sEVSTable/car_model/%s", EVSTable_path[0], name);
            }
            else
            {
                sprintf(dir, "%sEVSTable/car_model/modelB/%s", EVSTable_path[0], name);
            }

            AVM_LOGI("Main Thread Read file location [%s]\n", dir);


            memcpy(&cartex_stack.IMGPATH[cartex_stack.queue],dir,1024);
            pthread_mutex_lock(&lock);
            cartex_stack.queue = cartex_stack.queue + 1;
            pthread_mutex_unlock(&lock);
            fclose(fpCar);
            continue;

        }
    }

    free(car);
    free(name);

    AVM_LOGI("[****] CarTex_READ_BIN_func EXIT\n");
    pthread_exit(NULL);

}

static void init_carTex(FILE *fpBin)
{

/*
Thread main => upload img
Thread 2 => readbin
Thread 3 => loading raw img
*/
    cartex_stack.cur_read = 0;
    cartex_stack.queue = 0;
    cartex_stack.upload = 0;
    cartex_stack.envoke = 0;
    pthread_t T2 = 0,T3 = 0;
    m_TexTotal = 0;
    pthread_create(&T2, NULL, CarTex_READ_BIN_func, (void*)fpBin);
    pthread_create(&T3, NULL, CarTex_IMG_READ_func, (void*)0);

    while(1)
    {
        if(m_TexTotal > 0)
        {
            glGenTextures(m_TexTotal, g_texCar[Car_idx]);
            break;
        }
        usleep(100); 
    }
    AVM_LOGI("[****] Main Thread 1 Exit\n");
    while(1)
    {
        int ix =  cartex_stack.upload;
        if(ix < car_lampStatus[Car_idx].maxGenIMG)
        {
            if(ix >= (cartex_stack.cur_read ) ) // -1 > -1, -1 > 0
            {
                usleep(100);
                continue;
            }

            int current_index = ix ;

            int w1 = cartex_stack.w[current_index];
            int h1 = cartex_stack.h[current_index];

            if(w1 >500 && h1 >500)
            {
                int width_resize = 500, height_resize = 500;
                unsigned char *output_pixels = (unsigned char *)malloc(width_resize * height_resize * cartex_stack.strip[current_index]);
                int ret = stbir_resize_uint8(cartex_stack.imgdata[current_index], w1, h1, 0, output_pixels, width_resize, height_resize, 0, cartex_stack.strip[current_index]);
                if (ret == 0)
                {
                    fprintf(stderr, "fail to resize image:!!!!\n");
                    return -1;
                }
                glBindTexture(GL_TEXTURE_2D, g_texCar[Car_idx][current_index]);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_resize, height_resize, 0, GL_RGB, GL_UNSIGNED_BYTE, output_pixels);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                stbi_image_free(cartex_stack.imgdata[current_index]);
                free(output_pixels);
            }
            else
            {

                glBindTexture(GL_TEXTURE_2D, g_texCar[Car_idx][current_index]);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, cartex_stack.imgdata[current_index]);
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
                stbi_image_free(cartex_stack.imgdata[current_index]);
            }


            AVM_LOGI("Upload Textrue2D Car file in [%d]\n", ix);
            cartex_stack.upload = cartex_stack.upload + 1;
        }
        else break;
    }

    AVM_LOGI("[****] Main Thread 2 Exit\n");
    pthread_join(T2, NULL);
    pthread_join(T3, NULL);
}


static void init_carTex_1(FILE *fpBin)
{

    unsigned char * car;
    char* name;
    int TexTotal;
    int MaxPixels;
    fp_read_data_safty(&TexTotal, 1, 1*sizeof(int), fpBin);
    fp_read_data_safty(&MaxPixels, 1, 1*sizeof(int), fpBin);

    AVM_LOGI("Tex Total %d \n", TexTotal);
    AVM_LOGI("Tex Max Pixel %d \n", MaxPixels);

    g_texCar[Car_idx] = (GLuint*)malloc(TexTotal * sizeof(GLuint));
    glGenTextures(TexTotal, g_texCar[Car_idx]);
        

    name = malloc(sizeof(char)* 60);
    car = (unsigned char *)calloc(MaxPixels * sizeof(unsigned char), sizeof(unsigned char));

    car_lampStatus[Car_idx].maxGenIMG = TexTotal;
    
	GLuint physical = ~0U;
    for(int ix=0;ix<TexTotal;ix++)
    {
        int id = 0;
        int nameLen = 0;
        int w, h;

        fp_read_data_safty(&id, 1, 1*sizeof(int), fpBin);
        fp_read_data_safty(&nameLen, 1, 1*sizeof(int), fpBin);
        memset(name,0,sizeof(char)*60);
        fp_read_data_safty(name, sizeof(char), nameLen, fpBin);
        fp_read_data_safty(&w, 1, 1*sizeof(int), fpBin);
        fp_read_data_safty(&h, 1, 1*sizeof(int), fpBin);
        //w = 4000; h = 4000;
        AVM_LOGI("Tex Id %d Len %d Name %s w %d h %d\n", id, nameLen, name, w, h);

        for(int iy=nameLen;iy>0;iy--)
        {
            if(name[iy]=='.')
            {
                name[iy]='.';
                name[iy+1]='p';
                name[iy+2]='n';
                name[iy+3]='g';
                break;
            }
            else
            {
                name[iy] = '\0';
            }
        }

        char dir[1024] = "\0";
        if (Car_idx == 0)
        {
            sprintf(dir, "EVSTable/car_model/%s",name);
        }
        else
        {
            sprintf(dir, "EVSTable/car_model/modelB/%s", name);
        }
        FILE * fpCar = fp_source_app(dir, "rb");
        if(fpCar == NULL)
        {
            for (int iy = nameLen; iy > 0; iy--)
            {
                if (name[iy] == '.')
                {
                    name[iy] = '.';
                    name[iy + 1] = 'r';
                    name[iy + 2] = 'g';
                    name[iy + 3] = 'b';
                    break;
                }
                else
                {
                    name[iy] = '\0';
                }
            }
            if (Car_idx == 0)
            {
                sprintf(dir, "%sEVSTable/car_model/%s", EVSTable_path[0], name);
            }
            else
            {
                sprintf(dir, "%sEVSTable/car_model/modelB/%s", EVSTable_path[0], name);
            }
            fpCar = fp_source_app(dir, "rb");

            fp_read_data_safty(car, 1, w * h * 3, fpCar);

            glBindTexture(GL_TEXTURE_2D, g_texCar[Car_idx][ix]);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, car);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            fp_close_data_safty(fpCar);
        }
        else
        {
            if (Car_idx == 0)
            {
                sprintf(dir, "EVSTable/car_model/%s", name);
            }
            else
            {
                sprintf(dir, "EVSTable/car_model/modelB/%s", name);
            }

            //AVM_LOGI("Read file   %s\n", dir);
            int w1, h1, n;
            unsigned char *data1 = NULL;
            fp_read_image_app(dir, &data1, &w1, &h1, &n, 3, 0);

            if(w1 >500 && h1 >500 || strstr(name, "Lamp"))
            {
                int width_resize = 500, height_resize = 500;
                if(strstr(name, "Lamp"))
                {
                    width_resize  = 256;
                    height_resize = 256;
                }
                unsigned char *output_pixels = (unsigned char *)malloc(width_resize * height_resize * n);
                int ret = stbir_resize_uint8(data1, w1, h1, 0, output_pixels, width_resize, height_resize, 0, n);
                if (ret == 0)
                {
                    fprintf(stderr, "fail to resize image:!!!!\n");
                    return -1;
                }

                // fread(car, 1, w* h * 3, fpCar);
                //AVM_LOGI("Read file end\n");

                glBindTexture(GL_TEXTURE_2D, g_texCar[Car_idx][ix]);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_resize, height_resize, 0, GL_RGB, GL_UNSIGNED_BYTE, output_pixels);
                if(strstr(name, "licence") != NULL || strstr(name, "Lamp") != NULL)
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                stbi_image_free(data1);
                free(output_pixels);
            }
            else if(data1 != NULL)
            {
                //AVM_LOGI("Read file end\n");

                glBindTexture(GL_TEXTURE_2D, g_texCar[Car_idx][ix]);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
                if(strstr(name, "licence") != NULL)
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                stbi_image_free(data1);
            }
            else
            {
                SET_STATUS_CALIBRATION_IVI(car_model_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
            }
        }
    }

    free(car);
    free(name);
}

/*--------------------------*/
/*accelerator to init_carModel*/
/*--------------------------*/


void *CarModel_READ_BIN_func(void *ptr)
{
    FILE *fpBin = (FILE *)ptr;
    int MaxNum;
    pthread_mutex_lock(&lock);
    fread(&CarTotal[Car_idx], sizeof(int), 1, fpBin);
    fread(&MaxNum, sizeof(int), 1, fpBin);
    pthread_mutex_unlock(&lock);
    AVM_LOGI("Car Total %d \n", CarTotal[Car_idx]);
    AVM_LOGI("Car Max Num %d \n", MaxNum);
    CarVert[Car_idx] = (car_coord_3d_t *)malloc(CarTotal[Car_idx] * sizeof(car_coord_3d_t));
    VAOCar[Car_idx] = (GLuint *)malloc(CarTotal[Car_idx] * sizeof(GLuint));
    char* name = malloc(sizeof(char)* 60);
    m_TexTotal = 1;
    for(int ix=0;ix<CarTotal[Car_idx];ix++)
    { 
        
        int num = 0;
        int nameLen = 0;
        fread(&nameLen, sizeof(int), 1, fpBin);
        memset(name,0,sizeof(char)*60);
        fread(name, sizeof(char), nameLen, fpBin);
        fread(&CarVert[Car_idx][ix].TexId, sizeof(int), 1, fpBin);
        //CarVert[ix].TexId = 0;
        fread(&num, sizeof(int), 1, fpBin);

        cartex_stack.vertice[ix] = (float *)calloc( (num+20) * 3 * sizeof(float), sizeof(float));
        cartex_stack.texture[ix] = (float *)calloc( (num+20) * 2 * sizeof(float), sizeof(float));
        cartex_stack.normal[ix]  = (float *)calloc( (num+20) * 3 * sizeof(float), sizeof(float));

        CarVert[Car_idx][ix].num = num;

        CarVert[Car_idx][ix].shift = (float *)calloc(3 * sizeof(float), sizeof(float));
        int Totalmemory = num * 8 * sizeof(float);
        
        fread(cartex_stack.vertice[ix], sizeof(float), num*3, fpBin);
        fread(cartex_stack.texture[ix], sizeof(float), num*2, fpBin);
        fread(cartex_stack.normal[ix] , sizeof(float), num*3, fpBin);
        for(int iy=nameLen;iy>0;iy--)
        {
            if(name[iy]=='E')
            {
                name[iy] = '\0';
                break;
            }
            else
            {
                name[iy] = '\0';
            }
        }

        if(strstr(name, "window") || strstr(name, "glass"))
        {
            CarVert[Car_idx][ix].name = MODEL_GLASS;
        }
        else if(strstr(name, "wheel"))
        {
            int flag = 0;
            if (strstr(name, "front") || 
                strstr(name, "Front") || 
                strstr(name, "F") || 
                strstr(name, "f"))
            {
                if (strstr(name, "_R") ||
                    strstr(name, "_r"))
                {
                    CarVert[Car_idx][ix].name = MODEL_WHEEL_FR;
                }
                else
                {
                    CarVert[Car_idx][ix].name = MODEL_WHEEL_FL;
                }
            }
            else if (strstr(name, "rear") ||
                     strstr(name, "back") ||
                     strstr(name, "Back") ||
                     strstr(name, "b") ||
                     strstr(name, "B"))
            {
                if (strstr(name, "_R") ||
                    strstr(name, "_r"))
                {
                    CarVert[Car_idx][ix].name = MODEL_WHEEL_BR;
                }
                else
                {
                    CarVert[Car_idx][ix].name = MODEL_WHEEL_BL;
                }
            }

            // if(strstr(name, "_R")||
            //    strstr(name, "_r"))
            // {
            //     CarVert[ix].name++;
            //                     printf(" CarVert[ix].name = %d\n", CarVert[ix].name);
            // }
        }
        else if(strstr(name, "lamp"))
        {
            CarVert[Car_idx][ix].name = MODEL_LAMP;
            assign_lampdata_formobj(Car_idx, name, CarVert[Car_idx][ix].TexId);
        }
        else if(strstr(name, "door"))
        {
            CarVert[Car_idx][ix].name = MODEL_DOOR;
        }
        else if(strstr(name, "vehicle"))
        {
	        CarVert[Car_idx][ix].name = MODEL_CAR;
            if(Car_idx == 0 && strstr(name, "vehicle")) g_memCarBodyIx = ix;
        }
        else if(strstr(name, "licence"))
        {
            CarVert[Car_idx][ix].name = MODEL_LICENCE;
        }
        else if(strstr(name, "interior"))
        {
            CarVert[Car_idx][ix].name = MODEL_INTERIOR;
        }
        else if(strstr(name, "concealer"))
        {
            CarVert[Car_idx][ix].name = MODEL_CONCEALER;
        }
        else
        {
            CarVert[Car_idx][ix].name = MODEL_OTHER;
        }


        float* vertice = cartex_stack.vertice[ix];
        float* texture = cartex_stack.texture[ix];
        float* normal = cartex_stack.normal[ix];
        AVM_LOGI("CarTire ch %d vNum %d Name %s TexId %d \nvertice (%lf, %lf, %lf)\ntexture (%lf, %lf)\nNormal  (%lf, %lf, %lf)\nShift  (%lf, %lf, %lf)\n",
        ix, num, name, CarVert[Car_idx][ix].TexId,
        vertice[(num - 1) * 3], vertice[(num - 1) * 3 + 1], vertice[(num - 1) * 3 + 2],
        texture[(num - 1) * 2], texture[(num - 1) * 2 + 1],
        normal[(num - 1) * 3], normal[(num - 1) * 3 + 1], normal[(num - 1) * 3 + 2],
        CarVert[Car_idx][ix].shift[0], CarVert[Car_idx][ix].shift[1], CarVert[Car_idx][ix].shift[2]);
        fread(CarVert[Car_idx][ix].shift, sizeof(float), 3, fpBin);
        pthread_mutex_lock(&lock);
        cartex_stack.num[ix] = num;
        cartex_stack.queue = cartex_stack.queue + 1;
        pthread_mutex_unlock(&lock);
    }
    free(name);
    pthread_exit(NULL);
}

void *Mutiple_thread_CPU_mapping_GPU_func(void *ptr)
{
    Thread_mapping *data = (Thread_mapping*) ptr;
    memcpy(data->dst,data->src,data->size);
    AVM_LOGI("finish the thread mapping for size [%d]\n", data->size);
    pthread_exit(NULL);
}

static void init_carModel_1(FILE *fpBin)
{
    int MaxNum;
    fp_read_data_safty(&CarTotal[Car_idx], sizeof(int), 1, fpBin);
    fp_read_data_safty(&MaxNum, sizeof(int), 1, fpBin);

    AVM_LOGI("Car Total %d \n", CarTotal[Car_idx]);
    AVM_LOGI("Car Max Num %d \n", MaxNum);

    CarVert[Car_idx] = (car_coord_3d_t *)malloc(CarTotal[Car_idx] * sizeof(car_coord_3d_t));
    VAOCar[Car_idx] = (GLuint *)malloc(CarTotal[Car_idx] * sizeof(GLuint));
    glGenVertexArrays(CarTotal[Car_idx], VAOCar[Car_idx]);


    char* name;
    float* vertice;
    float* texture;
    float* normal;

    name = malloc(sizeof(char)* 60);

    vertice = (float *)calloc(MaxNum * 3 * sizeof(float), sizeof(float));
    texture = (float *)calloc(MaxNum * 2 * sizeof(float), sizeof(float));
    normal = (float *)calloc(MaxNum * 3 * sizeof(float), sizeof(float));

    for(int ix=0;ix<CarTotal[Car_idx];ix++)
    { 
        int num = 0;
        int nameLen = 0;

        fp_read_data_safty(&nameLen, sizeof(int), 1, fpBin);
        memset(name,0,sizeof(char)*60);
        fp_read_data_safty(name, sizeof(char), nameLen, fpBin);
        fp_read_data_safty(&CarVert[Car_idx][ix].TexId, sizeof(int), 1, fpBin);
        //CarVert[ix].TexId = 0;
        fp_read_data_safty(&num, sizeof(int), 1, fpBin);
        CarVert[Car_idx][ix].num = num;

        CarVert[Car_idx][ix].shift = (float *)calloc(3 * sizeof(float), sizeof(float));
        int Totalmemory = num * 8 * sizeof(float);
        
        fp_read_data_safty(vertice, sizeof(float), num*3, fpBin);
        fp_read_data_safty(texture, sizeof(float), num*2, fpBin);
        fp_read_data_safty(normal, sizeof(float), num*3, fpBin);


        for(int iy=nameLen;iy>0;iy--)
        {
            if(name[iy]=='E')
            {
                name[iy] = '\0';
                break;
            }
            else
            {
                name[iy] = '\0';
            }
        }

        if(strstr(name, "window") || strstr(name, "glass"))
        {
            CarVert[Car_idx][ix].name = MODEL_GLASS;
        }
        else if(strstr(name, "wheel"))
        {
            int flag = 0;
            if (strstr(name, "front") || 
                strstr(name, "Front") || 
                strstr(name, "F") || 
                strstr(name, "f"))
            {
                if (strstr(name, "_R") ||
                    strstr(name, "_r"))
                {
                    CarVert[Car_idx][ix].name = MODEL_WHEEL_FR;
                }
                else
                {
                    CarVert[Car_idx][ix].name = MODEL_WHEEL_FL;
                }
            }
            else if (strstr(name, "rear") ||
                     strstr(name, "back") ||
                     strstr(name, "Back") ||
                     strstr(name, "b") ||
                     strstr(name, "B"))
            {
                if (strstr(name, "_R") ||
                    strstr(name, "_r"))
                {
                    CarVert[Car_idx][ix].name = MODEL_WHEEL_BR;
                }
                else
                {
                    CarVert[Car_idx][ix].name = MODEL_WHEEL_BL;
                }
            }

            // if(strstr(name, "_R")||
            //    strstr(name, "_r"))
            // {
            //     CarVert[ix].name++;
            //                     printf(" CarVert[ix].name = %d\n", CarVert[ix].name);
            // }
        }
        else if(strstr(name, "lamp"))
        {
            CarVert[Car_idx][ix].name = MODEL_LAMP;
            assign_lampdata_formobj(Car_idx, name, CarVert[Car_idx][ix].TexId);
        }
        else if(strstr(name, "door"))
        {
            CarVert[Car_idx][ix].name = MODEL_DOOR;
        }
        else if(strstr(name, "vehicle"))
        {
	        CarVert[Car_idx][ix].name = MODEL_CAR;
            if(Car_idx == 0 && strstr(name, "vehicle")) g_memCarBodyIx = ix;
        }
        else if(strstr(name, "licence"))
        {
            CarVert[Car_idx][ix].name = MODEL_LICENCE;
        }
        else if(strstr(name, "interior"))
        {
            //interiorE"
            CarVert[Car_idx][ix].name = MODEL_INTERIOR;
        }
        else if(strstr(name, "concealer"))
        {
            CarVert[Car_idx][ix].name = MODEL_CONCEALER;
        }
        else
        {
	        CarVert[Car_idx][ix].name = MODEL_OTHER;
        }

        AVM_LOGI("CarTire ch %d vNum %d Name %s TexId %d \nvertice (%lf, %lf, %lf)\ntexture (%lf, %lf)\nNormal  (%lf, %lf, %lf)\nShift  (%lf, %lf, %lf)\n",
            ix, num, name, CarVert[Car_idx][ix].TexId,
            vertice[(num - 1) * 3], vertice[(num - 1) * 3 + 1], vertice[(num - 1) * 3 + 2],
            texture[(num - 1) * 2], texture[(num - 1) * 2 + 1],
            normal[(num - 1) * 3], normal[(num - 1) * 3 + 1], normal[(num - 1) * 3 + 2],
            CarVert[Car_idx][ix].shift[0], CarVert[Car_idx][ix].shift[1], CarVert[Car_idx][ix].shift[2]);

        
        glGenBuffers(1, &vertexbufferCar[Car_idx][ix]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCar[Car_idx][ix]);
        glBufferData(GL_ARRAY_BUFFER, num * 3 * sizeof(float), vertice, GL_STATIC_DRAW);

        glGenBuffers(1, &uvbufferCar[Car_idx][ix]);
        glBindBuffer(GL_ARRAY_BUFFER, uvbufferCar[Car_idx][ix]);
        glBufferData(GL_ARRAY_BUFFER, num * 2 * sizeof(float), texture, GL_STATIC_DRAW);

        glGenBuffers(1, &norbufferCar[Car_idx][ix]);
        glBindBuffer(GL_ARRAY_BUFFER, norbufferCar[Car_idx][ix]);
        glBufferData(GL_ARRAY_BUFFER, num * 3 * sizeof(float), normal, GL_STATIC_DRAW);

        glBindVertexArray(VAOCar[Car_idx][ix]);
            glEnableVertexAttribArray(glAttrCarVertex);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCar[Car_idx][ix]);
            glVertexAttribPointer(glAttrCarVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glEnableVertexAttribArray(glAttrCarUV);
            glBindBuffer(GL_ARRAY_BUFFER, uvbufferCar[Car_idx][ix]);
            glVertexAttribPointer(glAttrCarUV, 2, GL_FLOAT, GL_FALSE, 0,(void*)0);

            glEnableVertexAttribArray(glAttrCarNor);
            glBindBuffer(GL_ARRAY_BUFFER, norbufferCar[Car_idx][ix]);
            glVertexAttribPointer(glAttrCarNor, 3, GL_FLOAT, GL_FALSE, 0,(void*)0);
        glBindVertexArray(0);
        AVM_LOGI("DEBUG - %d\n", ix);

        fp_read_data_safty(CarVert[Car_idx][ix].shift, sizeof(float), 3, fpBin);
    }
    
    free(vertice);
    free(texture);
    free(normal);
    
    free(name);

}


static void init_carModel(FILE *fpBin)
{
    /*
    Thread main => upload vert
    Thread 2 => readbin
    */
    m_TexTotal = 0;
    cartex_stack.cur_read = 0;
    cartex_stack.queue = 0;
    cartex_stack.upload = 0;
    cartex_stack.envoke = 0;
    pthread_t T2 = 0;
    pthread_create(&T2, NULL, CarModel_READ_BIN_func, (void*)fpBin);

    while(1)
    {
        if(m_TexTotal > 0)
        {
            glGenVertexArrays(CarTotal[Car_idx], VAOCar[Car_idx]);
            break;
        }
        usleep(100); 
    }
    AVM_LOGI("[****] Main Thread 1 Exit\n");
   
   while(1)
   {
        pthread_mutex_lock(&lock);
        int ix =  cartex_stack.upload;
        pthread_mutex_unlock(&lock);
        if(ix < (int)CarTotal[Car_idx])
        {
            if(ix >= (cartex_stack.queue) ) //12, 
            {
                usleep(10);
                continue;
            }

            #if 1
            int current_index = ix;

            int num = cartex_stack.num[current_index];
            gettimeofday(&start2, NULL);

            glGenBuffers(1, &vertexbufferCar[Car_idx][current_index]);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCar[Car_idx][current_index]);
            glBufferData(GL_ARRAY_BUFFER, num * 3 * sizeof(float), NULL, GL_STATIC_DRAW);


            pthread_t UploadVert = 0;
            Thread_mapping upVert_data;
            void *ptr1 = glMapBufferRange(GL_ARRAY_BUFFER,0,num * 3 * sizeof(float),GL_MAP_WRITE_BIT);
            upVert_data.src = cartex_stack.vertice[current_index];
            upVert_data.dst = ptr1;
            upVert_data.size = num * 3 * sizeof(float);
            pthread_create(&UploadVert, NULL, Mutiple_thread_CPU_mapping_GPU_func, (void*)&upVert_data);


            glGenBuffers(1, &uvbufferCar[Car_idx][current_index]);
            glBindBuffer(GL_ARRAY_BUFFER, uvbufferCar[Car_idx][current_index]);
            glBufferData(GL_ARRAY_BUFFER, num * 2 * sizeof(float), NULL, GL_STATIC_DRAW);


            pthread_t UploadTexutre = 0;
            Thread_mapping upTexutre_data;

            void *ptr2 = glMapBufferRange(GL_ARRAY_BUFFER,0,num * 2 * sizeof(float),GL_MAP_WRITE_BIT);
            upTexutre_data.src = cartex_stack.texture[current_index];
            upTexutre_data.dst = ptr2;
            upTexutre_data.size = num * 2 * sizeof(float);
            pthread_create(&UploadTexutre, NULL, Mutiple_thread_CPU_mapping_GPU_func, (void*)&upTexutre_data);


            glGenBuffers(1, &norbufferCar[Car_idx][current_index]);
            glBindBuffer(GL_ARRAY_BUFFER, norbufferCar[Car_idx][current_index]);
            glBufferData(GL_ARRAY_BUFFER, num * 3 * sizeof(float), NULL, GL_STATIC_DRAW);

            pthread_t Uploadnormal = 0;
            Thread_mapping upnormal_data;
            void *ptr3 = glMapBufferRange(GL_ARRAY_BUFFER,0,num * 3 * sizeof(float),GL_MAP_WRITE_BIT);
            upnormal_data.src = cartex_stack.normal[current_index];
            upnormal_data.dst = ptr3;
            upnormal_data.size = num * 3 * sizeof(float);
            pthread_create(&Uploadnormal, NULL, Mutiple_thread_CPU_mapping_GPU_func, (void*)&upnormal_data);

            pthread_join(UploadVert,NULL);
            pthread_join(UploadTexutre,NULL);
            pthread_join(Uploadnormal,NULL);


            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCar[Car_idx][current_index]);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, uvbufferCar[Car_idx][current_index]);
            glUnmapBuffer(GL_ARRAY_BUFFER);

            glBindVertexArray(VAOCar[Car_idx][current_index]);
            {
                glEnableVertexAttribArray(glAttrCarVertex);
                glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCar[Car_idx][current_index]);
                glVertexAttribPointer(glAttrCarVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

                glEnableVertexAttribArray(glAttrCarUV);
                glBindBuffer(GL_ARRAY_BUFFER, uvbufferCar[Car_idx][current_index]);
                glVertexAttribPointer(glAttrCarUV, 2, GL_FLOAT, GL_FALSE, 0,(void*)0);

                glEnableVertexAttribArray(glAttrCarNor);
                glBindBuffer(GL_ARRAY_BUFFER, norbufferCar[Car_idx][current_index]);
                glVertexAttribPointer(glAttrCarNor, 3, GL_FLOAT, GL_FALSE, 0,(void*)0);
            }
            glBindVertexArray(0);

            AVM_LOGI("MainThread Loading CarModel coordinate - %d\n", current_index);

            free(cartex_stack.vertice[current_index]);
            free(cartex_stack.texture[current_index]);
            free(cartex_stack.normal[current_index]);
            gettimeofday(&end2, NULL);
            printf("InitCar id [%d] , upload vert&texture&norm times is [%f]\n",ix, calculate_time(start2, end2));
            cartex_stack.upload = cartex_stack.upload + 1;
            #endif

        }
        else 
        {
            AVM_LOGI("Leave ix = [%d] & CarTotal[Car_idx] = [%d]\n",ix,  CarTotal[Car_idx]);
            break;
        }
   }

    AVM_LOGI("[****] Main Thread 2 Exit\n");
    pthread_join(T2, NULL);
}

static void init_light(FILE *fpBin)
{
    // int lightNum;
    // float* lightPos;
    // float* Brightness;

    fp_read_data_safty(&lightNum[Car_idx], 1, 1*sizeof(int), fpBin);
    AVM_LOGI("lightNum %d\n",lightNum[Car_idx]);

    lightPos[Car_idx] = (float *)calloc(lightNum[Car_idx] * 3 * sizeof(float), sizeof(float));
    Brightness[Car_idx] = (float *)calloc(lightNum[Car_idx] * sizeof(float), sizeof(float));

    fp_read_data_safty(lightPos[Car_idx], sizeof(float), lightNum[Car_idx]*3, fpBin);
    fp_read_data_safty(Brightness[Car_idx], sizeof(float), lightNum[Car_idx], fpBin);

    //AVM_LOGI("light id %d \nPosition (%lf, %lf, %lf)\nbright %lf\n", ix, light[ix].pos[0], light[ix].pos[1], light[ix].pos[2], light[ix].bright);
    for(int ix=0;ix<lightNum[Car_idx];ix++)
    {
        // light[ix].pos = (float *)calloc(3 * sizeof(float), sizeof(float));
        // fread(light[ix].pos, sizeof(float), 3, fpBin);
        // fread(&light[ix].bright, sizeof(float), 1, fpBin);
        int index = ix*3;
        AVM_LOGI("light id %d \nPosition (%lf, %lf, %lf)\nbright %lf\n", ix, lightPos[Car_idx][index+0], lightPos[Car_idx][index+1], lightPos[Car_idx][index+2], Brightness[Car_idx][ix]);
    }

    glUniform1i(glGetUniformLocation(programHandleCar[0], "lightNum"), lightNum[Car_idx]);
    glUniform1fv(glGetUniformLocation(programHandleCar[0], "bright"), lightNum[Car_idx], Brightness[Car_idx]);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "lightPos"), lightNum[Car_idx], lightPos[Car_idx]);

    // free(lightPos);
    // free(Brightness);
}

static void init_skyTex(FILE *fpBin)
{
    int SkyTotal;
    char* name;
    int MaxPixel;
    unsigned char * Sky;
    fp_read_data_safty(&SkyTotal, 1, 1*sizeof(int), fpBin);
    fp_read_data_safty(&MaxPixel, 1, 1*sizeof(int), fpBin);

    glGenTextures(1, &g_texSky[0]);
    glActiveTexture(GL_TEXTURE0 + CAR_SKY);
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_texSky[0]);

    Sky = (unsigned char *)malloc(MaxPixel * sizeof(unsigned char));
    name = malloc(sizeof(char)* 60);

    for(int ix=0;ix<SkyTotal;ix++)
    {
        int nameLen = 0;
        int w, h;

        fp_read_data_safty(&nameLen, 1, 1*sizeof(int), fpBin);
        memset(name,0,sizeof(char)*60);
        fp_read_data_safty(name, sizeof(char), nameLen, fpBin);
        fp_read_data_safty(&w, 1, 1*sizeof(int), fpBin);
        fp_read_data_safty(&h, 1, 1*sizeof(int), fpBin);
                
        // printf("name = %s\n",name);
        for(int iy=nameLen;iy>0;iy--)
        {
            if(name[iy]=='.' )
            {
                name[iy]='.';
                name[iy+1]='r';
                name[iy+2]='g';
                name[iy+3]='b';
                break;
            }
            else
            {
                name[iy] = '\0';
            }
        }

        AVM_LOGI("Tex Id %d Len %d Name %s w %d h %d\n", ix, nameLen, name, w, h);
    
        char dir[1024] = "\0";

        sprintf(dir, "EVSTable/car_model/skybox/%s",name);
        FILE * fpCar = fp_source_app(dir,"rb");
        if(fpCar == NULL) 
        {
            for (int iy = nameLen; iy > 0; iy--)
            {
                if (name[iy] == '.')
                {
                    name[iy] = '.';
                    name[iy + 1] = 'p';
                    name[iy + 2] = 'n';
                    name[iy + 3] = 'g';
                    break;
                }
                else
                {
                    name[iy] = '\0';
                }
            }
            sprintf(dir, "EVSTable/car_model/skybox/%s",name);

            AVM_LOGI("Skybox image no RGB file, use PNG file to read!\n");
            // sprintf(dir, "EVSTable/car_model/skybox/%s", name);
            // fpCar = fopen(dir,"rb");
            int w1,h1,n;
            unsigned char *data1 = NULL;
            fp_read_image_app(dir, &data1, &w1, &h1, &n, 3, 0);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + ix, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
            stbi_image_free(data1);

        }
        else
        {
            fp_read_data_safty(Sky, 1, w * h * 3, fpCar);
            fp_close_data_safty(fpCar);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + ix, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, Sky);
        }
        // glTexDirectVIV(GL_TEXTURE_CUBE_MAP_POSITIVE_X + ix, w, h, GL_VIV_NV21, (GLvoid **)&Sky);
    }
    free(name);
    free(Sky);
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glUniform1i(glGetUniformLocation(programHandleCar[0], "skybox"), CAR_SKY);
    glActiveTexture(GL_TEXTURE0 + CAR_SKY);
    glBindTexture(GL_TEXTURE_2D, g_texSky[0]);
}

static void init_car2d(FILE *fpBin)
{
    int w, h;
    unsigned char * data;
    glGenVertexArrays(1, VAOCar2d);

    fp_read_data_safty(&w, 1, 1*sizeof(int), fpBin);
    fp_read_data_safty(&h, 1, 1*sizeof(int), fpBin);
    AVM_LOGI("AVM2d img W %d H %d\n", w, h);

    data = (unsigned char *)malloc(w*h*4*sizeof(unsigned char));
    
    CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;
    char init_car2d_path[1024];
    if(car_info->color_style == 0)
    {
        sprintf(init_car2d_path,"EVSTable/car_model/car_2d/car_2d_A(%d)_B(%d)_C(%d)_D(%d)_E(%d).png",
        car_info->car2d_status[0],
        car_info->car2d_status[1],
        car_info->car2d_status[2],
        car_info->car2d_status[3],
        car_info->car2d_status[4]);
    }
    else
    {
        sprintf(init_car2d_path,"EVSTable/car_model/color_style/%d/car_2d/car_2d_A(%d)_B(%d)_C(%d)_D(%d)_E(%d).png",
        car_info->color_style,
        car_info->car2d_status[0],
        car_info->car2d_status[1],
        car_info->car2d_status[2],
        car_info->car2d_status[3],
        car_info->car2d_status[4]);
    }
    int w1, h1, n;
    unsigned char *data1 = NULL;
    fp_read_image_app(init_car2d_path, &data1, &w1, &h1, &n, 4, 1);
    if(data1 != NULL)
    {
        glGenTextures(1, &g_texCar2d);
        glBindTexture(GL_TEXTURE_2D, g_texCar2d);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w1, h1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_image_free(data1);
    }
    else 
    {
        AVM_LOGE(LOG_RED("[ERROR]") "No carmodel top2d img\n");
        SET_STATUS_CALIBRATION_IVI(car_model_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
    }


    AVM_LOGI("2D IMG (CarModel) axis density is following :\n");
    AVM_LOGI("carModel2D_axis_density [%f,%f]\n",carModel2D_axis_density[0],carModel2D_axis_density[1]);

    if (carModel2D_axis_density[0] == 0)
        carModel2D_axis_density[0] = 1;
    if (carModel2D_axis_density[1] == 0)
        carModel2D_axis_density[1] = 1;
    float vertexCoord2dAvm[3*6] = 
    {
        -1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f,
        -1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
        -1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f
    };

    float vertexCoord2dAvm2[3*6] = 
    {
        -1.00000f,-1.00000f,0.00000f,
         1.00000f,-1.00000f,0.00000f,
         1.00000f, 1.00000f,0.00000f,
         1.00000f, 1.00000f,0.00000f,
        -1.00000f,-1.00000f,0.00000f,
        -1.00000f, 1.00000f,0.00000f
    };

    static float fragmentCoord2dAvm[2*6] =
    {
        0.00000, 0.00000, 
        1.00000, 0.00000,
        1.00000, 1.00000,
        1.00000, 1.00000,
        0.00000, 0.00000,
        0.00000, 1.00000,
    };

    glGenBuffers(1, &vertbuffer2d[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuffer2d[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoord2dAvm, GL_STATIC_DRAW);

    glGenBuffers(1, &textbuffer2d[0]);
    glBindBuffer(GL_ARRAY_BUFFER, textbuffer2d[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoord2dAvm, GL_STATIC_DRAW);

    glGenBuffers(1, &vertbuffer2d[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuffer2d[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoord2dAvm2, GL_STATIC_DRAW);

    glGenBuffers(1, &textbuffer2d[1]);
    glBindBuffer(GL_ARRAY_BUFFER, textbuffer2d[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoord2dAvm, GL_STATIC_DRAW);

    glBindVertexArray(VAOCar2d[0]);
        glEnableVertexAttribArray(glAttrCarVertex);
        glBindBuffer(GL_ARRAY_BUFFER, vertbuffer2d[0]);
        glVertexAttribPointer(glAttrCarVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(glAttrCarUV);
        glBindBuffer(GL_ARRAY_BUFFER, textbuffer2d[0]);
        glVertexAttribPointer(glAttrCarUV, 2, GL_FLOAT, GL_FALSE, 0,(void*)0);
    glBindVertexArray(0);

    glBindVertexArray(VAOCar2d[1]);
        glEnableVertexAttribArray(glAttrCarVertex);
        glBindBuffer(GL_ARRAY_BUFFER, vertbuffer2d[1]);
        glVertexAttribPointer(glAttrCarVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(glAttrCarUV);
        glBindBuffer(GL_ARRAY_BUFFER, textbuffer2d[1]);
        glVertexAttribPointer(glAttrCarUV, 2, GL_FLOAT, GL_FALSE, 0,(void*)0);
    glBindVertexArray(0);

    //free(data);
}

static void init_carSize(FILE *fpBin)
{
    size_t expectedItems = 1;
    float f_rx_buffer[50];
	size_t readItems = fp_read_data_safty(&f_rx_buffer, sizeof(float), expectedItems, fpBin);
    //CarSize[0] = 1.0f;CarSize[1] = 1.0f;

    if(readItems == expectedItems)
    {
        CarSize[0] = f_rx_buffer[0];
        CarSize[1] = f_rx_buffer[0];
        if(CarSize[0] > 5.0f || CarSize[0] < 0.0f) CarSize[0] = 1.0f;
        AVM_LOGI("CarModel Real Size is %f,%f\n",CarSize[0],CarSize[1]);
    }
    else { CarSize[0] = 1.0f;CarSize[1] = 1.0f;}
}


static void init_car3d()
{
    /*
    static GLuint g_texCar3d;
GLuint VAOCar3d[5];
GLuint vertbuffer3d[1] = {0}; //0:LB, 1:RB, 2:LF, 3:RF
GLuint textbuffer3d[1] = {0};
    
    */
   #if 0
    int w, h;
    unsigned char * data;
    glGenVertexArrays(1, VAOCar3d);
    FILE * fpCar3d = fopen("/media/flash/userdata/EVSTable/car_model/car_3d.rgba","rb");
    w = 858;
    h = 720;

    data = (unsigned char *)malloc(w*h*4*sizeof(unsigned char));
    
    // FILE * fpCar3d = fopen("/media/flash/userdata/EVSTable/car_model/car_3d.rgba","rb");
    // if(fpCar3d == NULL)
    // {
    //     AVM_LOGI("flash no 2Dcar data, so try to read USB....");
    //     fpCar3d = fp_source_rb("EVSTable/car_model/car_3d.rgba");
    // }
    fread(data, 1, w* h * 4, fpCar3d);
    fclose(fpCar3d);


    glGenTextures(1, &g_texCar3d);

    glBindTexture(GL_TEXTURE_2D, g_texCar3d);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    static float vertexCoord2dAvm[3*6] = 
    {
        -1.000000,-1.000000,0.000000,
         1.000000,-1.000000,0.000000,
         1.000000, 1.000000,0.000000,
         1.000000, 1.000000,0.000000,
        -1.000000,-1.000000,0.000000,
        -1.000000, 1.000000,0.000000,
    };

    static float fragmentCoord2dAvm[2*6] =
    {
        0.00000, 0.00000, 
        1.00000, 0.00000,
        1.00000, 1.00000,
        1.00000, 1.00000,
        0.00000, 0.00000,
        0.00000, 1.00000,
    };

    glGenBuffers(1, &vertbuffer3d[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuffer3d[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoord2dAvm, GL_STATIC_DRAW);

    glGenBuffers(1, &textbuffer3d[0]);
    glBindBuffer(GL_ARRAY_BUFFER, textbuffer3d[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoord2dAvm, GL_STATIC_DRAW);

    glBindVertexArray(VAOCar3d[0]);
        glEnableVertexAttribArray(glAttrCarVertex);
        glBindBuffer(GL_ARRAY_BUFFER, vertbuffer3d[0]);
        glVertexAttribPointer(glAttrCarVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(glAttrCarUV);
        glBindBuffer(GL_ARRAY_BUFFER, textbuffer3d[0]);
        glVertexAttribPointer(glAttrCarUV, 2, GL_FLOAT, GL_FALSE, 0,(void*)0);
    glBindVertexArray(0);

    free(data);
    #endif
    int w, h;
    unsigned char * data;
    glGenVertexArrays(1, VAOCar3d);
    glGenTextures(MAX_PAGE_NUM, &g_texCar3d);
    glGenTextures(MAX_PAGE_NUM, &g_texCover3d);
    AVM_LOGI("---------------------------------------\n");
    //FILE * fpCar3d = fopen("/media/flash/userdata/EVSTable/car_model/car_3d.rgba","rb");
     w = 858;
     h = 720;
    char path[1024];
    FILE * fpCar3d;
    AVM_LOGI("current loading 3d2d carimg ......\n");
    for(int i = 0; i < MAX_PAGE_NUM; i++)
    {
        int w1,h1,n;
        sprintf(path,"%sEVSTable/car_model/3d2dtext/car_3d_%d.png",EVSTable_path[0],i);
        if (access(path, F_OK) != -1) 
        {
            AVM_LOGI("3d2dtext file exists: %s\n", path);
        } 
        else 
        {
            //AVM_LOGI("3d2dtext file does not exist: %s\n", path);
            continue;
        }

        stbi_set_flip_vertically_on_load(1);
        unsigned char *data1 = stbi_load(path, &w1, &h1, &n, 0);

        // sprintf(path,"EVSTable/car_model/3d2dtext/car_3d_%d.rgba",i);
        
        // fpCar3d = fopen(path, "rb");
        // if(fpCar3d == NULL) continue;
        // AVM_LOGI("reading the %s ...\n", path);
        // fread(&w, sizeof(int),1,fpCar3d);
        // fread(&h, sizeof(int),1,fpCar3d);
        // data = (unsigned char *)malloc(w*h*4*sizeof(unsigned char));
        // fread(data, 1, w* h * 4, fpCar3d);
        // fclose(fpCar3d);
        

        glBindTexture(GL_TEXTURE_2D, g_texCar3d[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w1, h1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_image_free(data1);
        /*--------loading coverImg---------*/
        #if 0
        sprintf(path,"EVSTable/car_model/recimg/car_3d_%d.rgba",i);
        fpCar3d = fopen(path, "rb");
        if(fpCar3d == NULL) continue;
        AVM_LOGI("reading the %s ...\n", path);
        fread(&w, sizeof(int),1,fpCar3d);
        fread(&h, sizeof(int),1,fpCar3d);
        data = (unsigned char *)malloc(w*h*4*sizeof(unsigned char));
        fread(data, 1, w* h * 4, fpCar3d);
        fclose(fpCar3d);
        

        glBindTexture(GL_TEXTURE_2D, g_texCover3d[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		free(data);
        #endif
        sprintf(path,"%sEVSTable/car_model/recimg/car_3d_%d.png",EVSTable_path[0],i);
        if (access(path, F_OK) != -1) 
        {
            g_existedCover3d[i] = 1 ;
            AVM_LOGI("recoverIMG file exists: %s\n", path);
        } 
        else 
        {
            //g_existedCover3d[i] = 
            //AVM_LOGI("recoverIMG file does not exist: %s\n", path);
            continue;
        }
        stbi_set_flip_vertically_on_load(1);
        data1 = stbi_load(path, &w1, &h1, &n, 0);
        glBindTexture(GL_TEXTURE_2D, g_texCover3d[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w1, h1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data1);

    }

    static float vertexCoord2dAvm[3*6] = 
    {
        -1.000000,-1.000000,0.000000,
         1.000000,-1.000000,0.000000,
         1.000000, 1.000000,0.000000,
         1.000000, 1.000000,0.000000,
        -1.000000,-1.000000,0.000000,
        -1.000000, 1.000000,0.000000,
    };

    static float fragmentCoord2dAvm[2*6] =
    {
        0.00000, 0.00000, 
        1.00000, 0.00000,
        1.00000, 1.00000,
        1.00000, 1.00000,
        0.00000, 0.00000,
        0.00000, 1.00000,
    };

    glGenBuffers(1, &vertbuffer3d[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuffer3d[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoord2dAvm, GL_STATIC_DRAW);

    glGenBuffers(1, &textbuffer3d[0]);
    glBindBuffer(GL_ARRAY_BUFFER, textbuffer3d[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoord2dAvm, GL_STATIC_DRAW);

    glBindVertexArray(VAOCar3d[0]);
        glEnableVertexAttribArray(glAttrCarVertex);
        glBindBuffer(GL_ARRAY_BUFFER, vertbuffer3d[0]);
        glVertexAttribPointer(glAttrCarVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(glAttrCarUV);
        glBindBuffer(GL_ARRAY_BUFFER, textbuffer3d[0]);
        glVertexAttribPointer(glAttrCarUV, 2, GL_FLOAT, GL_FALSE, 0,(void*)0);
    glBindVertexArray(0);
    AVM_LOGI("---------------------------------------\n");

}

void init_lampdata()
{

    AVM_LOGI("[*] init the Lamp Status\n");
    
    for(int i = 0; i < 2; i++)
    {
        memset(LAMP_dir[i], '\0', sizeof(LAMP_dir[i]));
        switch(i)
        {
            case 0:
                sprintf(LAMP_dir[i], "%sEVSTable/car_model/", EVSTable_path[0]);
            break;
            case 1:
                sprintf(LAMP_dir[i], "%sEVSTable/car_model/modelB/", EVSTable_path[0]);
            break;
            default:
                sprintf(LAMP_dir[i], "%sEVSTable/car_model/modelB/", EVSTable_path[0]);
            break;
        }
        
    }

    for(int i = 0; i < CAR_MODEL_NUMBER; i++)
    {
        car_lampStatus[i].file_path = LAMP_dir[i];
        car_lampStatus[i].Lamp_STD_obj_name = (char**)malloc(sizeof(char**)*Classify_LAMP_max);
        for(int j = 0; j < Classify_LAMP_max; j++)
        {
            car_lampStatus[i].Lamp_STD_obj_name[j] = Lamp_Classify_Name[j];
        }

        car_lampStatus[i].Lamp_all_obj_name = (char**)malloc(Lamp_obj_max * sizeof(char**));
        for(int j = 0 ; j <  Lamp_obj_max; j++)
        {
            car_lampStatus[i].Lamp_all_obj_name[j] = (char*)malloc(100* sizeof(char*));
        }
        memset(car_lampStatus[i].Lamp_gl_ID,-1,sizeof(int)*Lamp_obj_max);
        memset(car_lampStatus[i].Lamp_TexUnit,-1,sizeof(int)*Lamp_obj_max);
    }
}

void carmodel_default_value(int Car_idx)
{
    int TexTotal = 10;

    g_texCar[Car_idx] = (GLuint*)malloc(TexTotal * sizeof(GLuint));
    CarTotal[Car_idx] = TexTotal;
    glGenTextures(TexTotal, g_texCar[Car_idx]);

    //AVM_LOGI("Car Total %d \n", CarTotal[Car_idx]);
    //AVM_LOGI("Car Max Num %d \n", MaxNum);

	CarVert[Car_idx]    = (car_coord_3d_t *)malloc(CarTotal[Car_idx] * sizeof(car_coord_3d_t));
    VAOCar[Car_idx]     = (GLuint *)malloc(CarTotal[Car_idx] * sizeof(GLuint));

    // AVM_LOGI("lightNum %d\n",lightNum[Car_idx]);
    lightNum[Car_idx]   = 2;
    lightPos[Car_idx]   = (float *)calloc(lightNum[Car_idx] * 3 * sizeof(float), sizeof(float));
    Brightness[Car_idx] = (float *)calloc(lightNum[Car_idx] * sizeof(float), sizeof(float));
	
}

void read_car_bin()
{ 
    AVM_LOGI("=====start to read Carmodel bin==========\n");
    init_lampdata();
    VAOCar = (GLuint **)malloc(2 * sizeof(GLuint));
    FILE *fpBin;
    char path[1024];
    char model[][10] = {"","modelB"};
    texture_3D_mode = 1;
    
    for(Car_idx = 0; Car_idx < 2; Car_idx++)
    {
        sprintf(path,"EVSTable/car_model/%s/CarModel.bin", model[Car_idx]);
        fpBin = fp_source_app(path, "rb");
        if(fpBin == NULL && Car_idx == 0)
        {
            AVM_LOGI(LOG_RED("[ERROR]")" Loading modelA in USB data error\n");
            AVM_LOGI("[WARNING] Using default value!!!\n");
            carmodel_default_value(Car_idx);
            AVM_LOGI("no modelB in USB data, so current mode is complete carmodel\n");
            texture_3D_mode = 0;
            SET_STATUS_CALIBRATION_IVI(car_model_file, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
            continue;
        }
        else if(fpBin == NULL && Car_idx == 1)
        {
            AVM_LOGI("has modelB in USB data, so current mode is img carmodel\n");
            continue;
        }

        init_carTex_1(fpBin);
        init_carModel_1(fpBin);
        init_light(fpBin);

        if(Car_idx == 0)
        {
	        init_skyTex(fpBin);
            init_car2d(fpBin);
            init_carSize(fpBin);
        }
        fp_close_data_safty(fpBin);
    }
    AVM_LOGI("=============end Carmodel bin==============\n");

    if(texture_3D_mode == 1)
    {
        // init_car3d();
    }
  
}

void read_car_model()
{
    // FILE * fpCarSpecular;
    // FILE * fpCarglossisess;
  
    // fpCarSpecular = fp_source_rb("EVSTable/car_model/luxgen_urx/car_model_luxgen_urx_specular.rgb");
    // fpCarglossisess = fp_source_rb("EVSTable/car_model/luxgen_urx/car_model_luxgen_urx_glossisess.rgb");
  
    // carTexSpecular = (unsigned char *)malloc(CAR_TEXTURE_W * CAR_TEXTURE_H * 3 * sizeof(unsigned char));

    // fread(carTexSpecular, 1, CAR_TEXTURE_W * CAR_TEXTURE_H * 3, fpCarSpecular);
    // fclose(fpCarSpecular);

    // carTexGlossisess = (unsigned char *)malloc(CAR_TEXTURE_W * CAR_TEXTURE_H * 3 * sizeof(unsigned char));

    // fread(carTexGlossisess, 1, CAR_TEXTURE_W * CAR_TEXTURE_H * 3, fpCarglossisess);
    // fclose(fpCarglossisess);

}

void open_gl_2dcarModel_parameter(axis_density axis_density)
{
    
    carModel2D_axis_density[0] = axis_density.x;
    carModel2D_axis_density[1] = axis_density.y;

    if (carModel2D_axis_density[0] == 0)
        carModel2D_axis_density[0] = 1;
    if (carModel2D_axis_density[1] == 0)
        carModel2D_axis_density[1] = 1;


    
    AVM_LOGI("[*****] 2D img axis density  [%f, %f]\n", axis_density.x,axis_density.y);
    AVM_LOGI("[*****] set img axis density [%f, %f]\n", carModel2D_axis_density[0],carModel2D_axis_density[1]);

    //* Read 2D car model map, if data is exist*/
    char mapPath[1024];

    sprintf(mapPath,"/EVSTable/config/Car_setting/Top2d_setting.config");
    FILE* fpBin = fp_source_app(mapPath, "r");
    Rectangle layout = {0};
    float UV[8] = {0};
    float vertexCoord2dAvm[3*6] = 
    {
        -1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f,
        -1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
        -1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f
    };

    float fragmentCoord2dAvm[2*6] =
    {
        0.00000, 0.00000, 
        1.00000, 0.00000,
        1.00000, 1.00000,
        1.00000, 1.00000,
        0.00000, 0.00000,
        0.00000, 1.00000,
    };
    if(fpBin == NULL)
    {
        AVM_LOGI(LOG_RED("[ERROR]")" Loading model2D in USB data error\n");
    }
    else
    {
        char Top2DIMG[1024];
        sprintf(Top2DIMG,"EVSTable/car_model/car_2d/car_2d_A(0)_B(0)_C(0)_D(0)_E(0).png");
        int w1, h1, n;
        unsigned char *data1 = NULL;
        fp_read_image_app(Top2DIMG, &data1, &w1, &h1, &n, 4, 1);
        stbi_image_free(data1);
        AVM_LOGI("[*****] Top2D Using map config\n");
        char line[100] = {0};
        while(fgets(line, sizeof(line), fpBin))
        {
            if(strncmp(line, "Index,X,Y,Width,Height", 22) == 0)
            {
                fgets(line, sizeof(line), fpBin);
                sscanf(line, "0,%d,%d,%d,%d", &layout.X, &layout.Y, &layout.W, &layout.H);
            }
            else if(strncmp(line, "Index,u0,v0,u1,v1,u2,v2,u3,v3", 29) == 0)
            {
                fgets(line, sizeof(line), fpBin);
                sscanf(line, "0,%f,%f,%f,%f,%f,%f,%f,%f", &UV[0], &UV[1], &UV[2], &UV[3], &UV[4], &UV[5], &UV[6], &UV[7]);
            }
        }
        PointF p1 = {.X = 0.0f, .Y = 0.0f}; 
        PointF p2 = {.X = 0.0f, .Y = 0.0f};
        PointF p3 = {.X = 0.0f, .Y = 0.0f};
        PointF p4 = {.X = 0.0f, .Y = 0.0f};
        p1.X = ((float)(layout.X) / (float)w1 - 0.5f) * 2.0f;
        p1.Y = ((float)(layout.Y) / (float)h1 - 0.5f) * 2.0f;
        p2.X = ((float)(layout.X + layout.W) / (float)w1 - 0.5f) * 2.0f;
        p2.Y = ((float)(layout.Y) / (float)h1 - 0.5f) * 2.0f;
        p3.X = ((float)(layout.X + layout.W) / (float)w1 - 0.5f) * 2.0f;
        p3.Y = ((float)(layout.Y + layout.H) / (float)h1 - 0.5f) * 2.0f;
        p4.X = ((float)(layout.X) / (float)w1 - 0.5f) * 2.0f;
        p4.Y = ((float)(layout.Y + layout.H) / (float)h1 - 0.5f) * 2.0f;

        vertexCoord2dAvm[0]  = p1.X * carModel2D_axis_density[0]; vertexCoord2dAvm[1]  = p1.Y * carModel2D_axis_density[1];
        vertexCoord2dAvm[3]  = p2.X * carModel2D_axis_density[0]; vertexCoord2dAvm[4]  = p2.Y * carModel2D_axis_density[1];
        vertexCoord2dAvm[6]  = p3.X * carModel2D_axis_density[0]; vertexCoord2dAvm[7]  = p3.Y * carModel2D_axis_density[1];
        vertexCoord2dAvm[9]  = p3.X * carModel2D_axis_density[0]; vertexCoord2dAvm[10] = p3.Y * carModel2D_axis_density[1];
        vertexCoord2dAvm[12] = p1.X * carModel2D_axis_density[0]; vertexCoord2dAvm[13] = p1.Y * carModel2D_axis_density[1];
        vertexCoord2dAvm[15] = p4.X * carModel2D_axis_density[0]; vertexCoord2dAvm[16] = p4.Y * carModel2D_axis_density[1];


        fragmentCoord2dAvm[0] = UV[0]; fragmentCoord2dAvm[1] = UV[1];
        fragmentCoord2dAvm[2] = UV[2]; fragmentCoord2dAvm[3] = UV[3];
        fragmentCoord2dAvm[4] = UV[6]; fragmentCoord2dAvm[5] = UV[7];
        fragmentCoord2dAvm[6] = UV[6]; fragmentCoord2dAvm[7] = UV[7];
        fragmentCoord2dAvm[8] = UV[0]; fragmentCoord2dAvm[9] = UV[1];
        fragmentCoord2dAvm[10] = UV[4]; fragmentCoord2dAvm[11] = UV[5];
    }
    glBindVertexArray(VAOCar2d[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuffer2d[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 3 * sizeof(float), vertexCoord2dAvm);
    glBindBuffer(GL_ARRAY_BUFFER, textbuffer2d[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 2 * sizeof(float), fragmentCoord2dAvm);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void open_gl_carModel_parameter(AXIS_DENSITY axis_density)
{
    
    carModel2D_axis_density[0] = axis_density.model2D[0];
    carModel2D_axis_density[1] = axis_density.model2D[1];

        if (carModel2D_axis_density[0] == 0)
        carModel2D_axis_density[0] = 1;
    if (carModel2D_axis_density[1] == 0)
        carModel2D_axis_density[1] = 1;


    
    AVM_LOGI("[*****] 2D img axis density  [%f, %f]\n",  axis_density.model2D[0],axis_density.model2D[1]);
    AVM_LOGI("[*****] set img axis density [%f, %f]\n", carModel2D_axis_density[0],carModel2D_axis_density[1]);


    float vertexCoord2dAvm[3*6] = 
    {
        -1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f,
         1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f,
        -1.00000f * carModel2D_axis_density[0],-1.00000f * carModel2D_axis_density[1] ,0.00000f,
        -1.00000f * carModel2D_axis_density[0], 1.00000f * carModel2D_axis_density[1] ,0.00000f
    };

    glBindVertexArray(VAOCar2d[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuffer2d[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 3 * sizeof(float), vertexCoord2dAvm);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void open_gl_init_car(void)
{
    glUseProgram(programHandleCar[0]);

    glAttrCarVertex = glGetAttribLocation(programHandleCar[0], "position");
    glAttrCarUV = glGetAttribLocation(programHandleCar[0], "texCoords");
    glAttrCarNor = glGetAttribLocation(programHandleCar[0], "normal");

    read_car_bin();
    
    glGenTextures(1, &g_texCarSpecular[0]);
    glGenTextures(1, &g_texCarGlossisess[0]);
    AVM_LOGI("DEBUG - 1\n");


    glUniform1i(glGetUniformLocation(programHandleCar[0], "diffuseMap"), CAR_TEXTURE);
    glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
    AVM_LOGI("DEBUG - 1\n");
    

    // glUniform1i(glGetUniformLocation(programHandleCar[0], "specularMap"), CAR_SPECULAR);
    // glActiveTexture(GL_TEXTURE0 + CAR_SPECULAR);
    // glBindTexture(GL_TEXTURE_2D, g_texCarSpecular[0]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CAR_TEXTURE_W, CAR_TEXTURE_H, 0, GL_RGB, GL_UNSIGNED_BYTE, carTexSpecular);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    // free(carTexSpecular);

    // glUniform1i(glGetUniformLocation(programHandleCar[0], "glossisessMap"), CAR_GLOSSISESS);
    // glActiveTexture(GL_TEXTURE0 + CAR_GLOSSISESS);
    // glBindTexture(GL_TEXTURE_2D, g_texCarGlossisess[0]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CAR_TEXTURE_W, CAR_TEXTURE_H, 0, GL_RGB, GL_UNSIGNED_BYTE, carTexGlossisess);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // free(carTexGlossisess);


    glGenTextures(2, &g_texStaticCar);
    glBindTexture(GL_TEXTURE_2D, g_texStaticCar[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
             PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT,
             0, GL_RGBA, GL_HALF_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, g_texStaticCar[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F,
             PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT,
             0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    

    AVM_LOGI("open_gl_init_car end\n");
}
int init_once = 0;
static void static_gpu_transfer()
{
    //if(init_once == 1) return;
    float yReverse = -1.0;
    int light_mode  = autosys.system_info.light_mode;
    int skybox_mode = autosys.system_info.sky_mode;
    glUseProgram(programHandleCar[0]);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "yRervsed"), yReverse);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "CarSize"), CarSize[0]);
    glUniform1i( glGetUniformLocation(programHandleCar[0], "lightMode"), light_mode);
    glUniform1i( glGetUniformLocation(programHandleCar[0], "skyMode"), skybox_mode);
    glUniform1i( glGetUniformLocation(programHandleCar[0], "lightNum"), lightNum[0]);
    init_once = 1;
    // glUniform1fv(glGetUniformLocation(programHandleCar[0], "bright")  , lightNum[0], Brightness[0]);
    // glUniform3fv(glGetUniformLocation(programHandleCar[0], "lightPos"), lightNum[0], lightPos[0]);
}
car_model_t preitem = MODEL_DOOR;
int preVaoTexture = -1;
static void dynamic_gpu_transfer(car_model_t item, int vaoCar, float alpha,  
float RotateAngle, float carAngle, AVM_CARMODEL *CarmodelContext)
{
    glUseProgram(programHandleCar[0]);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if(preVaoTexture != vaoCar)
    {
        glUniform1i(glGetUniformLocation(programHandleCar[0], "diffuseMap"), CAR_TEXTURE);
        glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, g_texCar[Car_idx][CarVert[Car_idx][vaoCar].TexId]);
        glUniform3fv(glGetUniformLocation(programHandleCar[0], "Shift"), 1, CarVert[Car_idx][vaoCar].shift);
        preVaoTexture = vaoCar;
    }

    if(preitem != item)
    {

        glUniform1f(glGetUniformLocation(programHandleCar[0], "alpha"), alpha);
        glUniform1i(glGetUniformLocation(programHandleCar[0], "Self_luminous_status"), 0);
        preitem = item;
        int flag = CarVert[Car_idx][vaoCar].name - 1;
        switch(preitem)
        {
            case MODEL_WHEEL_BL :
            case MODEL_WHEEL_BR :
            case MODEL_WHEEL_FL :
            case MODEL_WHEEL_FR :
            glUniform1i(glGetUniformLocation(programHandleCar[0], "flowing_lamp_uesd"), 0);
            glUniform1i(glGetUniformLocation(programHandleCar[0], "windows"), 1);
            // MVPRotate
            float LoadformMatrix_move[4][4];
            myIdentity(LoadformMatrix_move);
            myRotate(LoadformMatrix_move, 1.0, 0.0, 0, RotateAngle);
            float tireRudderMatrix[4][4];
            myIdentity(tireRudderMatrix);
            cal_tire_rudder_rotation_matrix(&tireRudderMatrix[0][0], carAngle, flag);
            myMultMatrix(LoadformMatrix_move, LoadformMatrix_move, tireRudderMatrix);
            //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "RUDDER_TIRE"), 1, GL_FALSE, &tireRudderMatrix[0][0]);

            glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "MVPRotate"), 1, GL_FALSE, &LoadformMatrix_move[0][0]);

            break;
            case MODEL_LAMP:
            glUniform1i(glGetUniformLocation(programHandleCar[0], "Self_luminous_status"), 1);
            glUniform1f( glGetUniformLocation(programHandleCar[0], "power")   , CarmodelContext->power.lamp);
            glUniform1i(glGetUniformLocation(programHandleCar[0], "flowing_lamp_uesd"), 1);
            glUniform1i(glGetUniformLocation(programHandleCar[0], "windows"), 0);
            GLint glUniMVPRotate = 0;
            glUniMVPRotate = glGetUniformLocation(programHandleCar[0], "MVPRotate");
            glUniformMatrix4fv(glUniMVPRotate, 1, GL_FALSE, &originalMatrix[0][0]);
            break;
            //m_flag_flowing_lamp_uesd
            //glUniform1f( glGetUniformLocation(programHandleCar[0], "power")   , CarmodelContext->power.lamp);
            case MODEL_GLASS :
            case MODEL_DOOR:
            case MODEL_CAR:
            case MODEL_LICENCE:
            case MODEL_INTERIOR:
            case MODEL_CONCEALER:
            case MODEL_OTHER:
            glUniform1i(glGetUniformLocation(programHandleCar[0], "flowing_lamp_uesd"), 0);
            glUniform1i(glGetUniformLocation(programHandleCar[0], "windows"), 0);
            //MVPRotate
            glUniMVPRotate = 0;
            glUniMVPRotate = glGetUniformLocation(programHandleCar[0], "MVPRotate");
            glUniformMatrix4fv(glUniMVPRotate, 1, GL_FALSE, &originalMatrix[0][0]);
            break;
        }
    }

}
static void load_gpu(camera_para_t para, AVM_CARMODEL *CarmodelContext)
{

    int light_mode =autosys.system_info.light_mode;
    float tmp[9] =
    {
        1.0f / CarSize[0], 0.0f, 0.0f,
        0.0f, 1.0f / CarSize[0], 0.0f,
        0.0f, 0.0f, 1.0f / CarSize[0],
    };

    if (CarmodelContext->axis_density.x == 0)
       CarmodelContext->axis_density.x = 1;
    if (CarmodelContext->axis_density.y == 0)
        CarmodelContext->axis_density.y = 1;
    if (CarmodelContext->axis_density.z == 0)
        CarmodelContext->axis_density.z = 1;

    glUseProgram(programHandleCar[0]);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "viewPos"), 1, para.exPara.camera);
    glUniformMatrix3fv(glGetUniformLocation(programHandleCar[0], "ModelToWorld"), 1, GL_FALSE, tmp);

    int glUniAxis = glGetUniformLocation(programHandleCar[0], "clipworld");
    
    float clipword[16];
    float AxisMatrix[16] =
    {
        CarmodelContext->axis_density.x, 0.0f, 0.0f, 0.0f,
        0.0f, CarmodelContext->axis_density.y, 0.0f, 0.0f,
        0.0f, 0.0f, CarmodelContext->axis_density.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    myMultiplyMat4_opengl(clipword, para.Rotate, AxisMatrix);
    // memcpy(clipword,para.Rotate, 16*sizeof(float));
    // clipword[0] *= CarmodelContext->axis_density.x;
    // clipword[5] *= CarmodelContext->axis_density.y;
    // clipword[10] *= CarmodelContext->axis_density.z;
    glUniformMatrix4fv(glUniAxis, 1, GL_FALSE, clipword);

    float* brightness = &autosys.avm_page.page[autosys.avm_page.current_page].set.carmodel.light.para.lightBright;
    float* position   = &autosys.avm_page.page[autosys.avm_page.current_page].set.carmodel.light.para.lightPos;
    float* lightColor = &autosys.avm_page.page[autosys.avm_page.current_page].set.carmodel.light.para.lightColor;
    for(int ix = 0; ix < 2 ; ix++)
    {
        if(ix < lightNum[0])
        {
            Brightness[0][ix] = brightness[ix];
            Brightness[0][ix] = brightness[ix];
            lightPos[0][(ix * 3) + 0] = position[(ix*3) + 0];
            lightPos[0][(ix * 3) + 1] = position[(ix*3) + 1];
            lightPos[0][(ix * 3) + 2] = position[(ix*3) + 2];
            // printf("lightPos[0][%d] = %f\n", (ix * 3) + 0, lightPos[0][(ix * 3) + 0]);
            // printf("lightPos[0][%d] = %f\n", (ix * 3) + 1, lightPos[0][(ix * 3) + 1]);
            // printf("lightPos[0][%d] = %f\n", (ix * 3) + 2, lightPos[0][(ix * 3) + 2]); 
            // printf("Brightness[0][%d] = %f\n", ix, Brightness[0][ix]); 
        }
    }

    if(light_mode == 0)
    {
        float x = lightPos[0][0];
        float y = lightPos[0][1];
        float z = lightPos[0][2];
        float dstX = para.Rotate[0] * x + para.Rotate[1] * y + para.Rotate[2] * z  + para.Rotate[3] * 1.0f;
        float dstY = para.Rotate[4] * x + para.Rotate[5] * y + para.Rotate[6] * z  + para.Rotate[7] * 1.0f;
        float dstZ = para.Rotate[8] * x + para.Rotate[9] * y + para.Rotate[10] * z + para.Rotate[11] * 1.0f;
        lightPos[0][0] = dstX;
        lightPos[0][1] = dstY;
        lightPos[0][2] = dstZ;
    }
    glUniform1fv(glGetUniformLocation(programHandleCar[0], "bright")  , lightNum[0], Brightness[0]);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "lightPos"), lightNum[0], lightPos[0]);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "lightColor"), 1, lightColor);

}

void upload_car_tex_color(int color_ix)
{
    char path[1024];

    if(color_ix == 0)
    {
        sprintf(path,"%sEVSTable/car_model/vehicleSurface_Color.png",EVSTable_path[0]);
    }
    else
    {
        sprintf(path,"%sEVSTable/car_model/color_style/%d/vehicleSurface_Color.png",EVSTable_path[0],color_ix);
    }


    if (access(path, F_OK) != -1) 
    {
        AVM_LOGI("CarModelbin colorIMG file exists: %s\n", path);
    } 
    else 
    {
        AVM_LOGI("CarModelbin colorIMG file does not exist: %s\n", path);
        return;
    }

    int w1, h1,n;
    stbi_set_flip_vertically_on_load(0);
    unsigned char *real_opt;
    unsigned char *data1 = stbi_load(path, &w1, &h1, &n, 0);
    int width_resize = 500, height_resize = 500;
    unsigned char *output_pixels = (unsigned char *)malloc(width_resize * height_resize * n);
    if(w1 >500 && h1 >500)
    {
        int ret = stbir_resize_uint8(data1, w1, h1, 0, output_pixels, width_resize, height_resize, 0, n);
        if (ret == 0)
        {
            fprintf(stderr, "fail to resize image:!!!!\n");
            return -1;
        }
        real_opt = output_pixels;
        w1 = width_resize;
        h1 = height_resize;

        // stbi_image_free(data1);
        // free(output_pixels);
    }else{real_opt = data1; }

    glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, g_texCar[0][CarVert[0][g_memCarBodyIx].TexId]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w1, h1, GL_RGB, GL_UNSIGNED_BYTE, real_opt);
    stbi_image_free(data1);
    free(output_pixels);
}

void upload_car_2d_tex_color(int color_ix)
{
    CAR_MODEL_INFO * car_info = &autosys.current_info.carmodel_info;
    char init_car2d_path[1024];
    if(car_info->color_style == 0)
    {
        sprintf(init_car2d_path,"%sEVSTable/car_model/car_2d/car_2d_A(%d)_B(%d)_C(%d)_D(%d)_E(%d).png",
        EVSTable_path[0],
        car_info->car2d_status[0],
        car_info->car2d_status[1],
        car_info->car2d_status[2],
        car_info->car2d_status[3],
        car_info->car2d_status[4]);
    }
    else
    {
        sprintf(init_car2d_path,"%sEVSTable/car_model/color_style/%d/car_2d/car_2d_A(%d)_B(%d)_C(%d)_D(%d)_E(%d).png",
        EVSTable_path[0],
        car_info->color_style,
        car_info->car2d_status[0],
        car_info->car2d_status[1],
        car_info->car2d_status[2],
        car_info->car2d_status[3],
        car_info->car2d_status[4]);
    }


    //sprintf(init_car2d_path,"%sEVSTable/car_model/car_2d.png",EVSTable_path);
    if (access(init_car2d_path, F_OK) != -1) 
    {
        AVM_LOGI("car_2d colorIMG file exists: %s\n", init_car2d_path);
    }
    else 
    {
        AVM_LOGI("car_2d file does not exist: %s\n", init_car2d_path);
        return;
    }


    int w1, h1, n;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data1 = stbi_load(init_car2d_path, &w1, &h1, &n, 0);

    glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, g_texCar2d);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w1, h1, GL_RGBA, GL_UNSIGNED_BYTE, data1);
    stbi_image_free(data1);
}

int lamp_flowing_setup(CAR_LAMP_UPLOAD_INFO *info, int part, int lampType, int *cur_idx)
{
    int flowing_max_length    = autosys.current_info.carmodel_info.flowing_lamp_max_length;
    float flowing_next_time_gap = (float)autosys.current_info.carmodel_info.flowing_lamp_speed * 3.0f / 100.0f;
    int flowing_OK = 0;
    switch(lampType)
    {
        case FrontLamp:
        break;
        case RearLamp:
        break;
        case SideMirrorLamp:
        break;


        default:
        break;
    }

#if 0
    if(info->flowing_enable == 1)
    {
        flowing_OK = 1;
        info->flowing_time_count++;
        if(info->flowing_time_count >= flowing_next_time_gap)
        {
            info->flowing_lamp_id = (info->flowing_lamp_id + 1 )% flowing_max_length;
            *cur_idx    = info->flowing_lamp_id + 1;
            info->flowing_time_count = 0;
        }
    }
    else
    {
        info->flowing_lamp_id = 0;
    }
#else
    if(info->flowing_enable == 1)
    {
        flowing_OK = 1;
        info->flowing_time_count++;
        gettimeofday(&info->time_end, NULL);
        float dtime = calculate_time(info->time_start, info->time_end);
        if(dtime >= (float)flowing_next_time_gap)
        {
            gettimeofday(&info->time_start, NULL);
            info->flowing_lamp_id = (info->flowing_lamp_id + 1 )% flowing_max_length;
            *cur_idx    = info->flowing_lamp_id + 1;
            info->flowing_time_count = 0;
        }
    }
    else
    {
        info->flowing_lamp_id = 0;
    }
#endif
    return flowing_OK;
}

int check_car_lamp_status(CAR_LAMP_UPLOAD_INFO *info, int id, char* Lamp_Name)
{
    char dst_file[1024];
    int cur_idx            = info->cur_idx;
    sprintf(dst_file, "%s%s/%s_%d.png", car_lampStatus[id].file_path,"flowing", Lamp_Name, cur_idx);
    //AVM_LOGI("Check the [%s] img to lamp status....\n", dst_file);
    if(access(dst_file, F_OK) != -1)
    {
        info->flag_flowing_img_exist = 1;
        return 1;
    }
    else
    {
        info->flag_flowing_img_exist = 0;
        info->flowing_lamp_id = 0;
        info->flowing_time_count = 0;
        info->cur_idx = 1;
        info->pre_idx = 0;
        return 0;
    }
}
#pragma region lamp flowing
/*
region lamp flowing
*/
typedef struct 
{
    int id[5];
    int index;
    int nextIndex;
    int w;
    int h;
    int buffer;
    /* data */
}pbo_t;

pbo_t lamp_flowing[3][2];
void lamp_flowing_init(pbo_t *context, float lampSizeW, float lampSizeH, int w, int h)
{
	context->index=0;
	context->nextIndex = 1;
	context->w = w;
	context->h = h;
	glGenBuffers(2, context->id);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context->id[0]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, w * h * 3, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context->id[1]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, w * h * 3, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
void set_lamp_flowing_start(pbo_t *context, unsigned char *bufferIMG, GLuint tex)
{
    int cur  = context->index;     
    int next = context->nextIndex; 

    GLsizeiptr size = (GLsizeiptr)(context->w * context->h * 3);

    glBindTexture(GL_TEXTURE_2D, tex);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context->id[cur]);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0,
                    context->w, context->h,
                    GL_RGB, GL_UNSIGNED_BYTE,
                    (const GLvoid*)0);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context->id[next]);
    GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        memcpy(ptr, bufferIMG, size);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    context->index     = next;
    context->nextIndex = cur;
}
void init_flowing_init()
{   
    for(int idx = 0; idx < 3; idx++)
    {
        if(g_car_lamp_upload_info[idx][0].flowing_enable == 1)
        {
            lamp_flowing_init(&lamp_flowing[idx][0],256,128,256,128 );
            lamp_flowing_init(&lamp_flowing[idx][1],256,128,256,128 );
        }
    }
}
#pragma endregion
void upload_car_lamp_status(int part, int id, int status)
{
    if (status < 0) return;

    int another = 0;
    if (status >= TotalLampIMG)
    {
        status -= TotalLampIMG;
        another = 1;
    }

    int* cur_idx            = &g_car_lamp_upload_info[part][another].cur_idx;
    int* pre_idx            = &g_car_lamp_upload_info[part][another].pre_idx;

    uint8_t flowing_lamp_enable = autosys.current_info.carmodel_info.flowing_lamp_enable;
    uint8_t Using_flowing_lamp  = 0;
    int choosen_obj             = 0;
    int flowing_max_length      = autosys.current_info.carmodel_info.flowing_lamp_max_length;
    int LampType = 0;
    if (status < FrontLamp)
    {
        choosen_obj = 0 * Classify_LAMP_mini_max + another;
        LampType = FrontLamp;
    }
    else if (status < RearLamp)
    {
        LampType = RearLamp;
        choosen_obj = 1 * Classify_LAMP_mini_max + another;
    }
    else if (status < SideMirrorLamp)
    {
        LampType = SideMirrorLamp;
        choosen_obj = 2 * Classify_LAMP_mini_max + another;
    }
    else { return; }

    if(car_lampStatus[id].Lamp_gl_ID[choosen_obj] < 0) { /*AVM_LOGI("[*****] No Lamp_obj_information\n");*/ return; }

    char dst_file[1024];
    char* Lamp_Name = Lamp_Status_Name[status];

    if (Lamp_Name == NULL || strlen(Lamp_Name) == 0) {
        return;
    }
    if(lamp_flowing_setup(&g_car_lamp_upload_info[part][another], part, LampType, cur_idx) == 1)
    {
        if(*cur_idx == *pre_idx) return;
        int fileExist = check_car_lamp_status(&g_car_lamp_upload_info[part][another], id, Lamp_Name);
        sprintf(dst_file, "%s%s/%s_%d.png", car_lampStatus[id].file_path,"flowing", Lamp_Name, *cur_idx);
        if(fileExist == 0)
        {
            sprintf(dst_file, "%s%s.png",car_lampStatus[id].file_path, Lamp_Name);
            //AVM_LOGI("Reload the [%s] img to lamp status....\n", dst_file);
            m_flag_flowing_lamp_uesd = false;
        }
        else
        {
            m_flag_flowing_lamp_uesd = true;
        }

        *pre_idx = *cur_idx;

    }
    else
    {
        sprintf(dst_file, "%s%s.png",car_lampStatus[id].file_path, Lamp_Name);
        m_flag_flowing_lamp_uesd = false;
    }
    //sprintf(dst_file, "%s%s.png", car_lampStatus[id].file_path,Lamp_Status_Name[status]);
    //AVM_LOGI("Upload the [%s] img to lamp status....\n", dst_file);

    if (access(dst_file, F_OK) != -1) 
    {

    }
    else 
    {
        //AVM_LOGI("[****] Lamp Status file does not exist: %s\n", dst_file);
        return;
    }

    int w1, h1,n;
    stbi_set_flip_vertically_on_load(0);
    unsigned char *data1 = stbi_load(dst_file, &w1, &h1, &n, 0);
    
    int x_offset = 0, y_offset = 0;
    int x_width = 0; int y_width = 0;
    if(m_flag_flowing_lamp_uesd == true)
    {
        sprintf(dst_file, "%s%s/%s.txt", car_lampStatus[id].file_path,"flowing", Lamp_Name);
        FILE *fp = fopen(dst_file, "r");
        if (!fp) {
            //AVM_LOGI("Cannot open ROI file: %s\n", dst_file);
        } 
        else
        {
            char line[100]= {0};
            if (fgets(line, sizeof(line), fp) != NULL)
            {
                if (sscanf(line, "%d,%d,%d,%d", &x_offset, &y_offset, &x_width, &y_width) != 4)
                {
                    printf("ROI format error: %s\n", line);
                }
            }
            fclose(fp);
        }
    }
    // else
    {
        glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, car_lampStatus[id].Lamp_gl_ID[choosen_obj]);

        int row_bytes = w1 * 3;
        if (row_bytes % 4 != 0)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        if(w1 >256 && h1 >256)
        {
            unsigned char *output_pixels = (unsigned char *)malloc(256 * 256 * 3);
            int ret = stbir_resize_uint8(data1, w1, h1, 0, output_pixels, 256, 256, 0, n);
            if (ret == 0)
            {
                fprintf(stderr, "fail to resize image:!!!!\n");
            }
            stbi_image_free(data1);
            data1 = output_pixels;
            w1 = 256;
            h1 = 256; 
        }
            glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, y_offset, w1, h1, GL_RGB, GL_UNSIGNED_BYTE, data1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (row_bytes % 4 != 0) 
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
    }
    stbi_image_free(data1);

}

unsigned char *imgdata1[MAX_PAGE_NUM];
unsigned char *imgdata2[MAX_PAGE_NUM];
int img1used[MAX_PAGE_NUM] = {0};
int img2used[MAX_PAGE_NUM] = {0};
int img_loadOK[4] = {0};
int car_png_imgw_h[MAX_PAGE_NUM][2] = {0};
typedef struct 
{
    int id;
    int start;
    int end;
    int color_ix;
}stCarModelReload;

void* read_car_model_img_func(void *ptr)
{
    stCarModelReload *reload = (stCarModelReload*)ptr;
    int color_ix = reload->color_ix;
    char path[1024];
    for(int i = reload->start; i < reload->end; i++)
    {
        int w1, h1, n;
        if(color_ix == 0)
        {
            sprintf(path,"%sEVSTable/car_model/3d2dtext/car_3d_%d.png",EVSTable_path[0],i);
        }
        else
        {
            sprintf(path,"%sEVSTable/car_model/color_style/%d/3d2dtext/car_3d_%d.png",EVSTable_path[0],color_ix,i);
        }

        if (access(path, F_OK) == -1) continue;
        AVM_LOGI("3d2dtext file exists: %s\n", path);

        stbi_set_flip_vertically_on_load(1);
        imgdata1[i] = stbi_load(path, &w1, &h1, &n, 0);
        img1used[i] = 1;
        car_png_imgw_h[i][0] = w1;
        car_png_imgw_h[i][1] = h1;

        if(color_ix == 0)
        {
            sprintf(path,"%sEVSTable/car_model/recimg/car_3d_%d.png",EVSTable_path[0],i);
        }
        else
        {
            sprintf(path,"%sEVSTable/car_model/color_style/%d/recimg/car_3d_%d.png",EVSTable_path[0],color_ix,i);
        }

        if (access(path, F_OK) == -1) continue;
        
        g_existedCover3d[i] = 1 ;
        AVM_LOGI("recoverIMG file exists: %s\n", path);

        stbi_set_flip_vertically_on_load(1);
        imgdata2[i] = stbi_load(path, &w1, &h1, &n, 0);
        img2used[i] = 1;
    }
    
    img_loadOK[reload->id] = 1;
    AVM_LOGI("ColorImg thread id %d is finished\n", reload->id);
}

void upload_car_3d2dtex_cover_img_color(int color_ix)
{
    memset(img1used, 0, sizeof(int) * MAX_PAGE_NUM);
    memset(img2used, 0, sizeof(int) * MAX_PAGE_NUM);
    memset(img_loadOK, 0, sizeof(int) * 3);

    pthread_t p1 = 0, p2 = 0, p3 = 0, p4 = 0;
    stCarModelReload reload1 =
    {
        .color_ix = color_ix,
        .start = 0,
        .end = MAX_PAGE_NUM / 3,
        .id = 0,
    };
    stCarModelReload reload2 =
    {
        .color_ix = color_ix,
        .start = MAX_PAGE_NUM / 3,
        .end   = MAX_PAGE_NUM / 3 *2,
        .id = 1,
    };
    stCarModelReload reload3 =
    {
        .color_ix = color_ix,
        .start = MAX_PAGE_NUM / 3 * 2,
        .end = MAX_PAGE_NUM,
        .id = 2,
    };
    // stCarModelReload reload4 =
    // {
    //     .color_ix = color_ix,
    //     .start = MAX_PAGE_NUM / 4 * 3,
    //     .end = MAX_PAGE_NUM ,
    //     .id = 3,
    // };
    pthread_create(&p1, NULL, read_car_model_img_func, (void*)&reload1);
    pthread_create(&p2, NULL, read_car_model_img_func, (void*)&reload2);
    pthread_create(&p3, NULL, read_car_model_img_func, (void*)&reload3);
    //pthread_create(&p4, NULL, read_car_model_img_func, (void*)&reload4);
    pthread_detach(p1); pthread_detach(p2); pthread_detach(p3); // pthread_detach(p4);
    int isfinish = 0;
    while(1)
    {
        for(int i = 0; i < MAX_PAGE_NUM; i++)
        {
            if(img1used[i] == 1)
            {
                img1used[i] = 2;
                glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
                glBindTexture(GL_TEXTURE_2D, g_texCar3d[i]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, car_png_imgw_h[i][0], car_png_imgw_h[i][1], GL_RGBA, GL_UNSIGNED_BYTE, imgdata1[i]);
                stbi_image_free(imgdata1[i]);
            }

            if(img2used[i] == 1)
            {
                img2used[i] = 2;
                glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
                glBindTexture(GL_TEXTURE_2D, g_texCover3d[i]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, car_png_imgw_h[i][0], car_png_imgw_h[i][1], GL_RGBA, GL_UNSIGNED_BYTE, imgdata2[i]);
                stbi_image_free(imgdata2[i]);
            }
        }

        if(img_loadOK[0] && img_loadOK[1] && img_loadOK[2])
        {
            if(isfinish == 1) break;
            isfinish = 1;
        }
    }
}



void upload_selection_car_color(int _ix)
{
    AVM_LOGI("---------------------------------------\n");
    AVM_LOGI("Upload......\n");
    upload_car_tex_color(_ix);
    upload_car_2d_tex_color(_ix);
    //upload_car_3d2dtex_cover_img_color(_ix);
    AVM_LOGI("Upload finish\n");
}

static void draw_car_body(int vaoCar, float alpha)
{
    glBindVertexArray(VAOCar[Car_idx][vaoCar]);
    glDrawArrays(GL_TRIANGLES, 0, CarVert[Car_idx][vaoCar].num);
    glBindVertexArray(0);
}

static void draw_tire_each(int tireNum, float RotateAngle, float carAngle, float alpha)
{
    //int flag = CarVert[Car_idx][tireNum].name - 1;
    glDepthMask(tmp_tireDepth);
    glBindVertexArray(VAOCar[Car_idx][tireNum]);

    glDrawArrays(GL_TRIANGLES, 0, CarVert[Car_idx][tireNum].num);
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(true);

}

static void draw_car_body_oit(int vaoCar, float alpha)
{
    glDepthMask(false);
    glBindVertexArray(VAOCar[Car_idx][vaoCar]);
    glDrawArrays(GL_TRIANGLES, 0, CarVert[Car_idx][vaoCar].num);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(true);
}
static void draw_tire_each_oit(int tireNum, float RotateAngle, float carAngle, float alpha)
{
    glDepthMask(false);
    glBindVertexArray(VAOCar[Car_idx][tireNum]);

    glDrawArrays(GL_TRIANGLES, 0, CarVert[Car_idx][tireNum].num);
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(true);
}

void draw_tire(camera_para_t para,  Car_item_Alpha *alpha, float Rotateangle, float carAngle)
{
    #if 0
    float alphaa = 1.0;
    load_gpu(para);
    glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	// glFrontFace(GL_CW);
    for(int ix=0;ix<CarTotal[Car_idx];ix++)
    {
        if(CarVert[Car_idx][ix].name >= MODEL_WHEEL_BL && CarVert[Car_idx][ix].name <= MODEL_WHEEL_FR)
        {
            if (car_obj_flag[ix] == 1)
            {
                draw_tire_each(ix, Rotateangle, carAngle, alpha->item.tires);
            }
        }
    }
    glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
    #endif
}
void setting_CarModel_display(int ix, const int *flag)
{
    Car_idx = ix;

    if (flag != NULL)
    {
        for (int i = 0; i < 20; ++i)
        {
            car_obj_flag[i] = flag[i];
        }
    }
    else
    {
        for (int i = 0; i < 40; ++i)
        {
            car_obj_flag[i] = 1;
        }
    }
}

#if 1
void draw_car(camera_para_t para, OPENGL_3D_CARMODEL_CONTEXT CarmodelContext)
{ 

}
#endif



int check_carModel_door_status(int ix, int *car3d_status)
{
    int used_item = 0;
    for (int active_obj = 0; active_obj < 5; active_obj++)
    {
        int *pList = NULL;
        if (car3d_status[active_obj] == 0)
        {
            pList = autosys.system_info.carModel_obj_map[active_obj * 2];
        }
        else
        {
            pList = autosys.system_info.carModel_obj_map[active_obj * 2 + 1];
        }
        for(int iy = 0; iy < 5; iy++)
        {
            if (ix == pList[iy]) used_item = 1;
        }
        if(autosys.system_info.carModel_item == 0) used_item = 1; // normal carmodel
    }
    return used_item;
}

int is_DrawItemMode(int ix , int *car3d_status, AVM_CARMODEL *CarmodelContext, car_model_t item)
{

    int display_ix = 0; //offroad used
    int flag1 = 0;
    int flag2 = 0;
    int flag3 = 0;
    switch(item)
    {
        case MODEL_GLASS :
        display_ix = 2;
        flag2 = !check_carModel_door_status(ix, car3d_status);
        break;
        case MODEL_WHEEL_BL :
        case MODEL_WHEEL_BR :
        case MODEL_WHEEL_FL :
        case MODEL_WHEEL_FR :
        display_ix = 1;
        break;
        case MODEL_LAMP:
        display_ix = 0;
        flag2 = !check_carModel_door_status(ix, car3d_status);
        break;
        case MODEL_DOOR:
        flag2 = !check_carModel_door_status(ix, car3d_status);
        display_ix = 0;
        break;
        case MODEL_LICENCE:
        case MODEL_CAR:
        display_ix = 0;
        break;
        default :
        display_ix = 0;
        break;
    }


    if(CarmodelContext->OffroadMode_displayItem != NULL)
    {
        if(CarmodelContext->OffroadMode_displayItem[display_ix] != 1) flag1 = 1;
    }

    flag3 = !car_obj_flag[ix];
    return (flag1 || flag2 || flag3);

}

float get_carModel_alpha(AVM_CARMODEL *CarmodelContext)
{
    float using_alpha = 0.0f;
    if(g_3D_carModel_alpha_value > 100 && (autosys.seethrough.seethrough_switch == 0))
    {
        using_alpha = ((float)(g_3D_carModel_alpha_value - 100))/100.0f;
    }
    else if((autosys.seethrough.seethrough_switch == 1))
    {
        using_alpha = autosys.seethrough.alpha_3d_car_value;
    }
    else
    {
        using_alpha = CarmodelContext->alpha.item.car;
    }
    return using_alpha;
}

int turn_on_alpha_by_user = 1;
float calculate_alpha(int ix, AVM_CARMODEL *CarmodelContext, float process, float *seethrough_alpha)
{
    float using_alpha = 0.0f;

    if (CarVert[Car_idx][ix].name == MODEL_GLASS)
    {
        if (autosys.seethrough.seethrough_switch == 0)
        {
            using_alpha = CarmodelContext->alpha.item.windows;
        }
        else
        {
            #if PARAM_CARMODEL_ALPHA_DEPEND_SEEHTROUGH
            float delta = CarmodelContext->alpha.item.windows - autosys.seethrough.alpha_3d_car_windows_value;
            *seethrough_alpha = CarmodelContext->alpha.item.windows - delta * process;
            #else
            *seethrough_alpha = autosys.seethrough.alpha_3d_car_windows_value;
            #endif
            using_alpha = *seethrough_alpha;
        }
    }
    else if (CarVert[Car_idx][ix].name >= MODEL_WHEEL_BL && CarVert[Car_idx][ix].name <= MODEL_WHEEL_FR)
    {
        if (autosys.seethrough.seethrough_switch == 0)
        {
            using_alpha = CarmodelContext->alpha.item.tires;
        }
        else
        {
            #if PARAM_CARMODEL_ALPHA_DEPEND_SEEHTROUGH
            float delta = CarmodelContext->alpha.item.tires - autosys.seethrough.alpha_3d_car_tire_value;
            *seethrough_alpha = CarmodelContext->alpha.item.tires - delta * process;
            #else
            *seethrough_alpha = autosys.seethrough.alpha_3d_car_tire_value;
            #endif
            using_alpha = *seethrough_alpha;
        }
    }
    else if(CarVert[Car_idx][ix].name == MODEL_LAMP)
    {
        if(g_3D_carModel_alpha_value > 100 && (autosys.seethrough.seethrough_switch == 0)&& turn_on_alpha_by_user)
        {
            using_alpha = ((float)(g_3D_carModel_alpha_value - 100))/100.0f;
        }
        else if(((autosys.seethrough.seethrough_switch == 1)))
        {
            #if PARAM_CARMODEL_ALPHA_DEPEND_SEEHTROUGH
            float delta = CarmodelContext->alpha.item.car - autosys.seethrough.alpha_3d_car_value;
            float seethrough_alpha =  CarmodelContext->alpha.item.car - delta*process;
            #else
            *seethrough_alpha = autosys.seethrough.alpha_3d_car_value;
            #endif
            using_alpha = *seethrough_alpha;
        }
        else
        {
            using_alpha = CarmodelContext->alpha.item.car;
        }
    }
    else if(CarVert[Car_idx][ix].name == MODEL_DOOR)
    {
        if(g_3D_carModel_alpha_value > 100 && (autosys.seethrough.seethrough_switch == 0)&& turn_on_alpha_by_user)
        {
            //draw_car_body(ix, ((float)(g_3D_carModel_alpha_value - 100))/100.0f);
            using_alpha = ((float)(g_3D_carModel_alpha_value - 100))/100.0f;
        }
        else if((autosys.seethrough.seethrough_switch == 1))
        {
            #if PARAM_CARMODEL_ALPHA_DEPEND_SEEHTROUGH
            float delta = CarmodelContext->alpha.item.car - autosys.seethrough.alpha_3d_car_value;
            float seethrough_alpha =  CarmodelContext->alpha.item.car - delta*process;
            #else
            *seethrough_alpha = autosys.seethrough.alpha_3d_car_value;
            #endif
            using_alpha = *seethrough_alpha;
        }
        else 
        {
            using_alpha = CarmodelContext->alpha.item.car;
        }

    }
    else if(CarVert[Car_idx][ix].name == MODEL_CAR || CarVert[Car_idx][ix].name == MODEL_CONCEALER)
    {
        if(g_3D_carModel_alpha_value > 100 && (autosys.seethrough.seethrough_switch == 0) && turn_on_alpha_by_user)
        {
            using_alpha = ((float)(g_3D_carModel_alpha_value - 100))/100.0f;
        }
        else if((autosys.seethrough.seethrough_switch == 1))
        {
            #if PARAM_CARMODEL_ALPHA_DEPEND_SEEHTROUGH
            float delta = CarmodelContext->alpha.item.car - autosys.seethrough.alpha_3d_car_value;
            seethrough_alpha =  CarmodelContext->alpha.item.car - delta*process;
            #else
            *seethrough_alpha = autosys.seethrough.alpha_3d_car_value;
            #endif
            using_alpha = *seethrough_alpha;
        }
        else
        {
            using_alpha = CarmodelContext->alpha.item.car;
        }
    }
    else if(CarVert[Car_idx][ix].name == MODEL_INTERIOR)
    {
        if(g_3D_carModel_alpha_value > 100 && (autosys.seethrough.seethrough_switch == 0) && turn_on_alpha_by_user)
        {
            using_alpha = ((float)(g_3D_carModel_alpha_value - 100))/100.0f;
        }
        else if((autosys.seethrough.seethrough_switch == 1))
        {
            #if PARAM_CARMODEL_ALPHA_DEPEND_SEEHTROUGH
            float delta = CarmodelContext->alpha.item.car - autosys.seethrough.alpha_3d_car_value;
            seethrough_alpha =  CarmodelContext->alpha.item.car - delta*process;
            #else
            *seethrough_alpha = autosys.seethrough.alpha_3d_car_value;
            #endif
            using_alpha = *seethrough_alpha;
        }
        else
        {
            using_alpha = CarmodelContext->alpha.item.car_interior;
        }
    }
    else if(CarVert[Car_idx][ix].name == MODEL_LICENCE)
    {
        if(g_3D_carModel_alpha_value > 100 && (autosys.seethrough.seethrough_switch == 0) && turn_on_alpha_by_user)
        {
            using_alpha = ((float)(g_3D_carModel_alpha_value - 100))/100.0f;
        }
        else if((autosys.seethrough.seethrough_switch == 1))
        {
            #if PARAM_CARMODEL_ALPHA_DEPEND_SEEHTROUGH
            float delta = CarmodelContext->alpha.item.car - autosys.seethrough.alpha_3d_car_value;
            seethrough_alpha =  CarmodelContext->alpha.item.car - delta*process;
            #else
            *seethrough_alpha = autosys.seethrough.alpha_3d_car_value;
            #endif
            using_alpha = *seethrough_alpha;
        }
        else
        {
            using_alpha = CarmodelContext->alpha.item.car;
        }
    }

    return using_alpha;
}
#pragma region OIT_SESSTION {
/**============================================================================
 *                     OIT SESSTION START
 *=============================================================================*/
FBO_t* fbo_oit;

void init_vert_oit()
{
    float quadVertices[] = {
        // position   // texCoord
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f,  -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f,   1.0f,  1.0f, 1.0f,
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1,&quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    if(g_texStaticCar[0] == 0 || g_texStaticCar[1] == 0)
    {
        int tex_w = PARAM_FXAA_PANEL_WIDTH;
        int tex_h = PARAM_FXAA_PANEL_HEIGHT;
        if(fbo_oit != NULL && fbo_oit->width > 0 && fbo_oit->height > 0)
        {
            tex_w = fbo_oit->width;
            tex_h = fbo_oit->height;
        }

        glGenTextures(2, g_texStaticCar);

        glBindTexture(GL_TEXTURE_2D, g_texStaticCar[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, tex_w, tex_h, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, g_texStaticCar[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, tex_w, tex_h, 0, GL_RED, GL_HALF_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void set_OIT_Idx(FBO_t* src)
{
    fbo_oit = src;
}

static void static_gpu_transfer_oit()
{
    float yReverse = -1.0;
    int light_mode  = autosys.system_info.light_mode;
    int skybox_mode = autosys.system_info.sky_mode;
    glUseProgram(programHandleCarOIT);
    glUniform1f(glGetUniformLocation(programHandleCarOIT, "yRervsed"), yReverse);
    glUniform1f(glGetUniformLocation(programHandleCarOIT, "CarSize"), CarSize[0]);
    glUniform1i( glGetUniformLocation(programHandleCarOIT, "lightMode"), light_mode);
    glUniform1i( glGetUniformLocation(programHandleCarOIT, "skyMode"), skybox_mode);
    glUniform1i( glGetUniformLocation(programHandleCarOIT, "lightNum"), lightNum[0]);
}

static void dynamic_gpu_transfer_oit(car_model_t item, int vaoCar, float alpha,  
float RotateAngle, float carAngle, AVM_CARMODEL *CarmodelContext)
{
    glUseProgram(programHandleCarOIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    if(alpha < 0.99f)
    {
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    //if(preVaoTexture != vaoCar)
    {
        glUniform1i(glGetUniformLocation(programHandleCarOIT, "diffuseMap"), CAR_TEXTURE);
        glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, g_texCar[Car_idx][CarVert[Car_idx][vaoCar].TexId]);
        glUniform3fv(glGetUniformLocation(programHandleCarOIT, "Shift"), 1, CarVert[Car_idx][vaoCar].shift);
        preVaoTexture = vaoCar;
    }

    //if(preitem != item)
    {

        glUniform1f(glGetUniformLocation(programHandleCarOIT, "alpha"), alpha);
        glUniform1i(glGetUniformLocation(programHandleCarOIT, "Self_luminous_status"), 0);
        preitem = item;
        int flag = CarVert[Car_idx][vaoCar].name - 1;
        switch(preitem)
        {
            case MODEL_WHEEL_BL :
            case MODEL_WHEEL_BR :
            case MODEL_WHEEL_FL :
            case MODEL_WHEEL_FR :
            glUniform1i(glGetUniformLocation(programHandleCar[0], "flowing_lamp_uesd"), 0);
            glUniform1i(glGetUniformLocation(programHandleCarOIT, "windows"), 1);
            // MVPRotate
            float LoadformMatrix_move[4][4];
            myIdentity(LoadformMatrix_move);
            myRotate(LoadformMatrix_move, 1.0, 0.0, 0, RotateAngle);
            float tireRudderMatrix[4][4];
            myIdentity(tireRudderMatrix);
            cal_tire_rudder_rotation_matrix(&tireRudderMatrix[0][0], carAngle, flag);
            myMultMatrix(LoadformMatrix_move, LoadformMatrix_move, tireRudderMatrix);
            //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "RUDDER_TIRE"), 1, GL_FALSE, &tireRudderMatrix[0][0]);

            glUniformMatrix4fv(glGetUniformLocation(programHandleCarOIT, "MVPRotate"), 1, GL_FALSE, &LoadformMatrix_move[0][0]);

            break;
            case MODEL_LAMP:
            glUniform1i(glGetUniformLocation(programHandleCarOIT, "Self_luminous_status"), 1);
            glUniform1f( glGetUniformLocation(programHandleCarOIT, "power")   , CarmodelContext->power.lamp);
            glUniform1i( glGetUniformLocation(programHandleCarOIT, "flowing_lamp_uesd")   , 1);
            glUniform1i(glGetUniformLocation(programHandleCarOIT, "windows"), 0);
                        //MVPRotate
            GLint glUniMVPRotate = 0;
            glUniMVPRotate = glGetUniformLocation(programHandleCarOIT, "MVPRotate");
            glUniformMatrix4fv(glUniMVPRotate, 1, GL_FALSE, &originalMatrix[0][0]);
            break;
            case MODEL_GLASS :
            case MODEL_DOOR:
            case MODEL_CAR:
            case MODEL_LICENCE:
            case MODEL_INTERIOR:
            case MODEL_CONCEALER:
            case MODEL_OTHER:
            
            glUniform1i(glGetUniformLocation(programHandleCarOIT, "windows"), 0);
            //MVPRotate
            glUniMVPRotate = 0;
            glUniMVPRotate = glGetUniformLocation(programHandleCarOIT, "MVPRotate");
            glUniformMatrix4fv(glUniMVPRotate, 1, GL_FALSE, &originalMatrix[0][0]);
            break;
        }
    }
    
}

static void load_gpu_oit(camera_para_t para, AVM_CARMODEL *CarmodelContext)
{

    int light_mode =autosys.system_info.light_mode;
    float tmp[9] =
    {
        1.0f / CarSize[0], 0.0f, 0.0f,
        0.0f, 1.0f / CarSize[0], 0.0f,
        0.0f, 0.0f, 1.0f / CarSize[0],
    };

    if (CarmodelContext->axis_density.x == 0)
       CarmodelContext->axis_density.x = 1;
    if (CarmodelContext->axis_density.y == 0)
        CarmodelContext->axis_density.y = 1;
    if (CarmodelContext->axis_density.z == 0)
        CarmodelContext->axis_density.z = 1;

    glUseProgram(programHandleCarOIT);
    glUniform3fv(glGetUniformLocation(programHandleCarOIT, "viewPos"), 1, para.exPara.camera);
    glUniformMatrix3fv(glGetUniformLocation(programHandleCarOIT, "ModelToWorld"), 1, GL_FALSE, tmp);

    int glUniAxis = glGetUniformLocation(programHandleCarOIT, "clipworld");
    float clipword[16];
    float AxisMatrix[16] =
    {
        CarmodelContext->axis_density.x, 0.0f, 0.0f, 0.0f,
        0.0f, CarmodelContext->axis_density.y, 0.0f, 0.0f,
        0.0f, 0.0f, CarmodelContext->axis_density.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    myMultiplyMat4_opengl(clipword, para.Rotate, AxisMatrix);
    // memcpy(clipword,para.Rotate, 16*sizeof(float));
    // clipword[0] *= CarmodelContext->axis_density.x;
    // clipword[5] *= CarmodelContext->axis_density.y;
    // clipword[10] *= CarmodelContext->axis_density.z;
    glUniformMatrix4fv(glUniAxis, 1, GL_FALSE, clipword);

    float* brightness = &autosys.avm_page.page[autosys.avm_page.current_page].set.carmodel.light.para.lightBright;
    float* position   = &autosys.avm_page.page[autosys.avm_page.current_page].set.carmodel.light.para.lightPos;
    float* lightColor = &autosys.avm_page.page[autosys.avm_page.current_page].set.carmodel.light.para.lightColor;
    for(int ix = 0; ix < 2 ; ix++)
    {
        if(ix < lightNum[0])
        {
            Brightness[0][ix] = brightness[ix];
            Brightness[0][ix] = brightness[ix];
            lightPos[0][(ix * 3) + 0] = position[(ix*3) + 0];
            lightPos[0][(ix * 3) + 1] = position[(ix*3) + 1];
            lightPos[0][(ix * 3) + 2] = position[(ix*3) + 2];
            // printf("lightPos[0][%d] = %f\n", (ix * 3) + 0, lightPos[0][(ix * 3) + 0]);
            // printf("lightPos[0][%d] = %f\n", (ix * 3) + 1, lightPos[0][(ix * 3) + 1]);
            // printf("lightPos[0][%d] = %f\n", (ix * 3) + 2, lightPos[0][(ix * 3) + 2]); 
            // printf("Brightness[0][%d] = %f\n", ix, Brightness[0][ix]); 
        }
    }

    if(light_mode == 0)
    {
        float x = lightPos[0][0];
        float y = lightPos[0][1];
        float z = lightPos[0][2];
        float dstX = para.Rotate[0] * x + para.Rotate[1] * y + para.Rotate[2] * z  + para.Rotate[3] * 1.0f;
        float dstY = para.Rotate[4] * x + para.Rotate[5] * y + para.Rotate[6] * z  + para.Rotate[7] * 1.0f;
        float dstZ = para.Rotate[8] * x + para.Rotate[9] * y + para.Rotate[10] * z + para.Rotate[11] * 1.0f;
        lightPos[0][0] = dstX;
        lightPos[0][1] = dstY;
        lightPos[0][2] = dstZ;
    }
    glUniform1fv(glGetUniformLocation(programHandleCarOIT, "bright")  , lightNum[0], Brightness[0]);
    glUniform3fv(glGetUniformLocation(programHandleCarOIT, "lightPos"), lightNum[0], lightPos[0]);
    glUniform3fv(glGetUniformLocation(programHandleCarOIT, "lightColor"), 1, lightColor);

}

void draw_static_car_png(float _alpha, GLuint texture)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glUseProgram(programHandleCar[0]);

    glUniform1i(glGetUniformLocation(programHandleCar[0], "diffuseMap"), CAR_TEXTURE);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "alpha"), _alpha);
    glUniform1i(glGetUniformLocation(programHandleCar[0], "seethroughAlhpaOn"), 0);
    glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, texture);

    float IndeMatrix[16] =
    {
     1, 0, 0, 0,
     0, 1, 0, 0,
     0, 0, 1, 0,
     0, 0, 0, 1
    };

    float Inde[] = {0.0f, 0.0f, 0.0f};

    int glUniAxis = glGetUniformLocation(programHandleCar[0], "Axisdensity");
    float AxisMatrix[] = {
        1.0f , 0.0f  , 0.0f, 0.0f,
        0.0f , 1.0f  , 0.0f, 0.0f,
        0.0f , 0.0f  , 1.0f, 0.0f,
        0.0f , 0.0f  , 0.0f, 1.0f,
    };
    glUniformMatrix4fv(glUniAxis, 1, GL_FALSE, AxisMatrix);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "yRervsed"), 1.0f);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "Shift"), 1, Inde);
    glUniform1i(glGetUniformLocation(programHandleCar[0], "windows"), 5);

    glUniAxis = glGetUniformLocation(programHandleCar[0], "clipworld");
    glUniformMatrix4fv(glUniAxis, 1, GL_FALSE, AxisMatrix);
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "MVPRotate"), 1, GL_FALSE, IndeMatrix);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "viewPos"), 1, Inde);
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "RUDDER_TIRE"), 1, GL_FALSE, IndeMatrix);

    glBindVertexArray(VAOCar2d[1]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void BackupOITToStaticTex(GLuint srcFBO,
                          GLuint accumColorTex_static,
                          GLuint accumAlphaTex_static,
                          int w, int h)
{
    if(w <= 0 || h <= 0)
    {
        AVM_LOGI("[OIT] BackupOITToStaticTex skipped: invalid size %d x %d\n", w, h);
        return;
    }

    GLint prevReadFBO = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
    GLenum status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("[OIT] BackupOITToStaticTex skipped: srcFBO(%u) status=0x%x\n", srcFBO, status);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)prevReadFBO);
        return;
    }

    // Clear stale GL errors to avoid mis-attributing older failures to copy calls below.
    while(glGetError() != GL_NO_ERROR) {}

    // 1) copy COLOR_ATTACHMENT0 -> accumColorTex_static
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindTexture(GL_TEXTURE_2D, accumColorTex_static);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
    {
        AVM_LOGI("[OIT] copy static color failed, glError=0x%x, srcFBO=%u, tex=%u, fallback=ReadPixels\n", err, srcFBO, accumColorTex_static);
        unsigned short *rgba16 = (unsigned short*)malloc((size_t)w * (size_t)h * 4U * sizeof(unsigned short));
        if(rgba16 != NULL)
        {
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glReadPixels(0, 0, w, h, GL_RGBA, GL_HALF_FLOAT, rgba16);
            err = glGetError();
            if(err == GL_NO_ERROR)
            {
                glBindTexture(GL_TEXTURE_2D, accumColorTex_static);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_HALF_FLOAT, rgba16);
                err = glGetError();
                if(err != GL_NO_ERROR)
                {
                    AVM_LOGI("[OIT] fallback static color upload failed, glError=0x%x\n", err);
                }
            }
            else
            {
                AVM_LOGI("[OIT] fallback static color read failed, glError=0x%x\n", err);
            }
            free(rgba16);
        }
        else
        {
            AVM_LOGI("[OIT] fallback static color alloc failed\n");
        }
    }

    // 2) copy COLOR_ATTACHMENT1 -> accumAlphaTex_static
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBindTexture(GL_TEXTURE_2D, accumAlphaTex_static);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
    err = glGetError();
    if(err != GL_NO_ERROR)
    {
        AVM_LOGI("[OIT] copy static alpha failed, glError=0x%x, srcFBO=%u, tex=%u, fallback=ReadPixels\n", err, srcFBO, accumAlphaTex_static);
        unsigned short *r16 = (unsigned short*)malloc((size_t)w * (size_t)h * sizeof(unsigned short));
        if(r16 != NULL)
        {
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadBuffer(GL_COLOR_ATTACHMENT1);
            glReadPixels(0, 0, w, h, GL_RED, GL_HALF_FLOAT, r16);
            err = glGetError();
            if(err == GL_NO_ERROR)
            {
                glBindTexture(GL_TEXTURE_2D, accumAlphaTex_static);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_HALF_FLOAT, r16);
                err = glGetError();
                if(err != GL_NO_ERROR)
                {
                    AVM_LOGI("[OIT] fallback static alpha upload failed, glError=0x%x\n", err);
                }
            }
            else
            {
                AVM_LOGI("[OIT] fallback static alpha read failed, glError=0x%x\n", err);
            }
            free(r16);
        }
        else
        {
            AVM_LOGI("[OIT] fallback static alpha alloc failed\n");
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)prevReadFBO);
}

static float alpha_car_tire_test = 0.0f;
void draw_runtime_car_oit(camera_para_t para, AVM_CARMODEL *CarmodelContext, int* alphamode, int* froce_disable_item)
{
    const int disable_item[20] = {0};
    if(froce_disable_item == NULL)
    {
        froce_disable_item = &disable_item;
    }
    GLint  previousFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_oit->framebuffer);

    GLfloat clearColor[] = { 0.f, 0.f, 0.f, 0.f };
    GLfloat clearAlpha[] = { 0.f };
    glClearBufferfv(GL_COLOR, 0, clearColor); 

    #if PARAM_OPENGL_OIT_COLOR_ALPHA_INDIVIUAL
        glClearBufferfv(GL_COLOR, 1, clearAlpha); 
    #endif

#if PARAM_CARMODEL_OIT_SHARE_DEPTH_BUFFER
    GL_FBO_AttachDepthRB(fbo_oit->framebuffer, autosys.frame_fbo[FBO_FXAA].depthRenderbuffer);
#else
    GL_COPY_FRAMEBUFFER_DEPTH(previousFBO, fbo_oit->framebuffer, g_PANEL_WIDTH, g_PANEL_HEIGHT, g_PANEL_WIDTH, g_PANEL_HEIGHT);
#endif

    //GL_COPY_FRAMEBUFFER_DEPTH(previousFBO, fbo_oit->framebuffer, g_PANEL_WIDTH, g_PANEL_HEIGHT, g_PANEL_WIDTH, g_PANEL_HEIGHT);
    static_gpu_transfer_oit();
    load_gpu_oit(para, CarmodelContext);
    
    float process = autosys.current_info.seethrough_process;
    int show_obj  = 0;
    int *car3d_status = autosys.current_info.carmodel_info.car3d_status;
    for(int ix = 0;ix < CarTotal[Car_idx]; ix++)
    {
        float seethrough_alpha = 0.0f;
        float using_alpha = 0.0f;
        tmp_tireDepth = false;        
        if (is_DrawItemMode(ix, car3d_status, CarmodelContext, CarVert[Car_idx][ix].name) == 1) 
            continue;
        using_alpha = calculate_alpha(ix, CarmodelContext, process, &seethrough_alpha);
        //using_alpha = 0.7f;

        dynamic_gpu_transfer_oit(
            CarVert[Car_idx][ix].name, 
            ix, 
            using_alpha, 
            CarmodelContext->Tire_RotateAngle, 
            CarmodelContext->Car_RotateAngle, 
            CarmodelContext
        );
        if (CarVert[Car_idx][ix].name == MODEL_GLASS)
        {
            if (froce_disable_item[WINDOWS] == 1) continue;
            if (alphamode[ALPHA_WINDOWS] == BACK_CULL_MODE) continue;
            if(using_alpha < 0.99f)
            {
                glDisable(GL_CULL_FACE);
            }
            else
            {
                if(get_carModel_alpha(CarmodelContext) >= 0.99f) continue;
            }

            if (get_carModel_alpha(CarmodelContext) >= 0.99f)
            {
                glEnable(GL_DEPTH_TEST);
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }
            draw_car_body_oit(ix, seethrough_alpha);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_CONCEALER)
        {
            if (froce_disable_item[CAR_BODY] == 1) continue;
            continue;
        }
        else if (CarVert[Car_idx][ix].name >= MODEL_WHEEL_BL && CarVert[Car_idx][ix].name <= MODEL_WHEEL_FR)
        {
            if (froce_disable_item[TIRE] == 1) continue;
            if (alphamode[ALPHA_TIRE] == BACK_CULL_MODE) continue;
            glEnable(GL_DEPTH_TEST);
            if(using_alpha < 0.99f)
            {
                glDisable(GL_CULL_FACE);
                if(alphamode[0] == 0 && get_carModel_alpha(CarmodelContext) < 0.99f)
                {
                    glDisable(GL_DEPTH_TEST);
                }
            }
            else
            {
                if(get_carModel_alpha(CarmodelContext) >= 0.99f) continue;
                glEnable(GL_CULL_FACE);
            }
            
            tmp_tireDepth = false;
            draw_tire_each_oit(ix, CarmodelContext->Tire_RotateAngle, CarmodelContext->Car_RotateAngle, seethrough_alpha);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_LICENCE)
        {
            if (froce_disable_item[CAR_BODY] == 1) continue;
            if (alphamode[ALPHA_CAR_BODY] == BACK_CULL_MODE) continue;
            if(using_alpha < 0.99f)
            {
                glDisable(GL_CULL_FACE);
                draw_car_body_oit(ix, seethrough_alpha);
            }

        }
        else if(CarVert[Car_idx][ix].name == MODEL_INTERIOR)
        {
            if (froce_disable_item[INTERIOR] == 1) continue;
            if (alphamode[ALPHA_INTERIOR] == BACK_CULL_MODE) continue;
            if(using_alpha < 0.99f)
            {
                glEnable(GL_CULL_FACE);
                draw_car_body_oit(ix, seethrough_alpha);
            }
        }
        else if(CarVert[Car_idx][ix].name == MODEL_LAMP)
        {
            if (froce_disable_item[LAMP] == 1) continue;
            if (alphamode[ALPHA_CAR_BODY] == BACK_CULL_MODE) continue;
            if(using_alpha < 0.99f)
            {
                glEnable(GL_CULL_FACE);
                draw_car_body_oit(ix, seethrough_alpha);
            }
        }
        else
        {
            if (froce_disable_item[CAR_BODY] == 1) continue;
            if (alphamode[ALPHA_CAR_BODY] == BACK_CULL_MODE) continue;
            if(using_alpha < 0.99f)
            {
                glEnable(GL_CULL_FACE);
                draw_car_body_oit(ix, seethrough_alpha);
            }
        }
    }
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
}

static oit_main_init = 0;

void write_rgb_float_for_vooya(GLuint fbo, int width, int height, const char* filename)
{
    int pixelCount = width * height;
    GLfloat* rgba = (GLfloat*)malloc(sizeof(GLfloat) * pixelCount * 4);
    if (!rgba) {
        fprintf(stderr, "Failed to allocate memory for RGB.\n");
        return;
    }

    GLint prevFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, rgba);

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        free(rgba);
        return;
    }

    for (int i = 0; i < pixelCount; ++i) {
        fwrite(&rgba[i * 4 + 0], sizeof(GLfloat), 1, fp); // R
        fwrite(&rgba[i * 4 + 1], sizeof(GLfloat), 1, fp); // G
        fwrite(&rgba[i * 4 + 2], sizeof(GLfloat), 1, fp); // B
    }

    fclose(fp);
    free(rgba);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
    //printf("✅ Wrote RGB float to %s\n", filename);
}

void write_r_float_for_vooya(GLuint fbo, int width, int height, const char* filename)
{
    int pixelCount = width * height;
    GLfloat* rdata = (GLfloat*)malloc(sizeof(GLfloat) * pixelCount);
    if (!rdata) {
        fprintf(stderr, "Failed to allocate memory for R channel.\n");
        return;
    }

    GLint prevFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT1); // accumAlphaTex
    glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, rdata);

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        free(rdata);
        return;
    }

    fwrite(rdata, sizeof(GLfloat), pixelCount, fp);
    fclose(fp);
    free(rdata);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
    //printf("✅ Wrote R float to %s\n", filename);
}


void write_r16f_raw(GLuint fbo, int width, int height, const char* filename, int flipY)
{
    int pixelCount = width * height;

    // 1. 綁定 FBO，設定讀取來源
    GLint prevReadFBO = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT1); // 假設 R16F 在 COLOR_ATTACHMENT1

    // 2. PixelStore 保險，避免 stride/padding 問題
    glPixelStorei(GL_PACK_ALIGNMENT,   1);
    glPixelStorei(GL_PACK_ROW_LENGTH,  0);
    glPixelStorei(GL_PACK_SKIP_ROWS,   0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

    // 3. 分配 buffer (每像素 2 bytes, uint16_t)
    uint16_t* rbuf = (uint16_t*)malloc(sizeof(uint16_t) * pixelCount);
    if (!rbuf) {
        fprintf(stderr, "Failed to allocate memory for R16F buffer.\n");
        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO);
        return;
    }

    // 4. 直接讀取 half float
    glReadPixels(0, 0, width, height, GL_RED, GL_HALF_FLOAT, rbuf);

    // 5. 翻轉 Y 軸 (選擇性)
    if (flipY) {
        int y;
        for (y = 0; y < height / 2; ++y) {
            int oppY = height - 1 - y;
            int x;
            for (x = 0; x < width; ++x) {
                int idx1 = y * width + x;
                int idx2 = oppY * width + x;
                uint16_t tmp = rbuf[idx1];
                rbuf[idx1] = rbuf[idx2];
                rbuf[idx2] = tmp;
            }
        }
    }

    // 6. 輸出檔案
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open %s for writing.\n", filename);
        free(rbuf);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO);
        return;
    }
    fwrite(rbuf, sizeof(uint16_t), pixelCount, fp);
    fclose(fp);

    // 7. 釋放資源 & 還原 framebuffer
    free(rbuf);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO);
}
void save_oit_fbo(GLuint oitFbo)
{
    write_rgb_float_for_vooya(oitFbo, PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT, "color_rgb_float.raw");
    //write_r_float_for_vooya(oitFbo,   PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT, "alpha_r_float.raw");
    write_r16f_raw(oitFbo, PARAM_FXAA_PANEL_WIDTH, PARAM_FXAA_PANEL_HEIGHT, "output_r16f.raw", 1);
}

void backup_oit_main(bool backupStaticIMG)
{
    if(backupStaticIMG)
    {
        if(fbo_oit == NULL || fbo_oit->framebuffer == 0)
        {
            AVM_LOGI("[OIT] backup_oit_main skipped: fbo_oit is invalid\n");
            return;
        }
        if(oit_main_init == 0 || g_texStaticCar[0] == 0 || g_texStaticCar[1] == 0)
        {
            init_vert_oit();
            if(oit_main_init == 0)
            {
                oit_main_init = 1;
            }
        }
        AVM_LOGI("Backup Static IMG\n");
        BackupOITToStaticTex(fbo_oit->framebuffer, g_texStaticCar[0], g_texStaticCar[1], fbo_oit->width, fbo_oit->height);
        backupStaticIMG = false;
    }
}
void draw_main_oit(bool UseStaticIMG)
{
    if(oit_main_init == 0)
    {
        init_vert_oit();
        oit_main_init = 1;
    }
    else if(oit_main_init == 20)
    {
        //save_oit_fbo(fbo_oit->framebuffer);
        // AVM_LOGI("Write OIT OKOKO\n");
        // AVM_LOGI("Write OIT OKOKO\n");
        // AVM_LOGI("Write OIT OKOKO\n");
        // AVM_LOGI("Write OIT OKOKO\n");
        oit_main_init++;
    }
    else
    {
        oit_main_init++;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(programHandleOITMain);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo_oit->accumColorTex);
    glUniform1i(glGetUniformLocation(programHandleOITMain, "accumColorTex"), 0);
    
    #if PARAM_OPENGL_OIT_COLOR_ALPHA_INDIVIUAL
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fbo_oit->accumAlphaTex);
    glUniform1i(glGetUniformLocation(programHandleOITMain, "accumAlphaTex"), 1);
    #endif

    
    if(UseStaticIMG)
    {
        //AVM_LOGI("Use Static IMG\n staticID = %d\n", g_texStaticCar[0]);
        glUniform1i(glGetUniformLocation(programHandleOITMain, "UseStaticIMG"), 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, g_texStaticCar[0]);
        glUniform1i(glGetUniformLocation(programHandleOITMain, "accumColorTex_static"), 2);
        
        #if PARAM_OPENGL_OIT_COLOR_ALPHA_INDIVIUAL
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, g_texStaticCar[1]);
        glUniform1i(glGetUniformLocation(programHandleOITMain, "accumAlphaTex_static"), 3);
        #endif
    }
    else
    {
        glUniform1i(glGetUniformLocation(programHandleOITMain, "UseStaticIMG"), 0);
    }
    glBindVertexArray(quadVAO);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glUseProgram(0);
}

void draw_pre_oit_real_car(camera_para_t para, AVM_CARMODEL *CarmodelContext, int* alphamode , int* froce_disable_item)
{
    const int disable_item[20] = {0};
    if(froce_disable_item == NULL)
    {
        froce_disable_item = &disable_item;
    }
    static_gpu_transfer();
    load_gpu(para, CarmodelContext);
    float process = autosys.current_info.seethrough_process;
    int show_obj  = 0;
    int *car3d_status = autosys.current_info.carmodel_info.car3d_status;
    for(int ix = 0;ix < CarTotal[Car_idx]; ix++)
    {
        float seethrough_alpha = 0.0f;
        float using_alpha = 0.0f;
        tmp_tireDepth = true;
        tmp_tireDepth = (CarmodelContext->OffroadMode_displayItem != NULL)?false:true;
        
        if (is_DrawItemMode(ix, car3d_status, CarmodelContext, CarVert[Car_idx][ix].name) == 1) 
            continue;
        using_alpha = calculate_alpha(ix, CarmodelContext, process, &seethrough_alpha);
        //using_alpha = 0.7f;

        dynamic_gpu_transfer(
            CarVert[Car_idx][ix].name, 
            ix, 
            using_alpha, 
            CarmodelContext->Tire_RotateAngle, 
            CarmodelContext->Car_RotateAngle, 
            CarmodelContext
        );
        if(using_alpha < 0.99f)
        {
            continue;
        }

        if (CarVert[Car_idx][ix].name == MODEL_GLASS)
        {
            if (froce_disable_item[WINDOWS] == 1) continue;
            draw_car_body(ix, seethrough_alpha);
        }
        else if (CarVert[Car_idx][ix].name >= MODEL_WHEEL_BL && CarVert[Car_idx][ix].name <= MODEL_WHEEL_FR)
        {
            if (froce_disable_item[TIRE] == 1) continue;
            if(get_carModel_alpha(CarmodelContext) < 0.99f && alphamode[2] == 1)
            {   
                tmp_tireDepth = false;
            }
            glEnable(GL_CULL_FACE);
            draw_tire_each(ix, CarmodelContext->Tire_RotateAngle, CarmodelContext->Car_RotateAngle, seethrough_alpha);
            glDisable(GL_CULL_FACE);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_LICENCE)
        {
            if (froce_disable_item[CAR_BODY] == 1) continue;
            draw_car_body(ix, seethrough_alpha);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_INTERIOR)
        {
            if (froce_disable_item[INTERIOR] == 1) continue;
            draw_car_body(ix, seethrough_alpha);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_LAMP)
        {
            if (froce_disable_item[LAMP] == 1) continue;
            draw_car_body(ix, seethrough_alpha);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            if (froce_disable_item[CAR_BODY] == 1) continue;
            draw_car_body(ix, seethrough_alpha);
            glDisable(GL_CULL_FACE);
        }
        // glDisable(GL_DEPTH_TEST);
        // glDisable(GL_BLEND);
        // glDisable(GL_CULL_FACE);
    }
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}
#pragma endregion }
void draw_runtime_car(camera_para_t para, AVM_CARMODEL *CarmodelContext, int* alphamode, int* froce_disable_item)
{
    const int disable_item[20] = {0};
    if(froce_disable_item == NULL)
    {
        froce_disable_item = &disable_item;
    }
    static_gpu_transfer();
    load_gpu(para, CarmodelContext);
    float process = autosys.current_info.seethrough_process;
    int show_obj  = 0;
    int *car3d_status = autosys.current_info.carmodel_info.car3d_status;
    for(int ix = 0;ix < CarTotal[Car_idx]; ix++)
    {
        float seethrough_alpha = 0.0f;
        float using_alpha = 0.0f;
        tmp_tireDepth = true;
        tmp_tireDepth = (CarmodelContext->OffroadMode_displayItem != NULL)?false:true;
        
        if (is_DrawItemMode(ix, car3d_status, CarmodelContext, CarVert[Car_idx][ix].name) == 1) 
            continue;
        using_alpha = calculate_alpha(ix, CarmodelContext, process, &seethrough_alpha);
        //using_alpha = 0.7f;

        dynamic_gpu_transfer(
            CarVert[Car_idx][ix].name, 
            ix, 
            using_alpha, 
            CarmodelContext->Tire_RotateAngle, 
            CarmodelContext->Car_RotateAngle, 
            CarmodelContext
        );
        if(using_alpha >= 0.99f)
        {
            continue;
        }

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);


        if (CarVert[Car_idx][ix].name == MODEL_GLASS)
        {
            if (alphamode[ALPHA_WINDOWS] == OIT_LIGHTWEIGHT_MODE) continue;
            if (froce_disable_item[WINDOWS] == 1) continue;
            draw_car_body(ix, seethrough_alpha);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_CONCEALER)
        {
            continue;
        }
        else if (CarVert[Car_idx][ix].name >= MODEL_WHEEL_BL && CarVert[Car_idx][ix].name <= MODEL_WHEEL_FR)
        {
            if (alphamode[ALPHA_TIRE] == OIT_LIGHTWEIGHT_MODE) continue;
            if (froce_disable_item[TIRE] == 1) continue;
            draw_tire_each(ix, CarmodelContext->Tire_RotateAngle, CarmodelContext->Car_RotateAngle, seethrough_alpha);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_LICENCE)
        {
            if (alphamode[ALPHA_CAR_BODY] == OIT_LIGHTWEIGHT_MODE) continue;
            if (froce_disable_item[CAR_BODY] == 1) continue;
            draw_car_body(ix, seethrough_alpha);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_INTERIOR)
        {
            if (froce_disable_item[INTERIOR] == 1) continue;
            if (alphamode[ALPHA_INTERIOR] == OIT_LIGHTWEIGHT_MODE) continue;
            draw_car_body(ix, seethrough_alpha);
        }
        else if(CarVert[Car_idx][ix].name == MODEL_LAMP)
        {
            if (froce_disable_item[LAMP] == 1) continue;
            if (alphamode[ALPHA_CAR_BODY] == OIT_LIGHTWEIGHT_MODE) continue;
            draw_car_body(ix, seethrough_alpha);
        }
        else
        {
            if (froce_disable_item[CAR_BODY] == 1) continue;
            if (alphamode[ALPHA_CAR_BODY] == OIT_LIGHTWEIGHT_MODE) continue;
            draw_car_body(ix, seethrough_alpha);
        }
        // glDisable(GL_DEPTH_TEST);
        // glDisable(GL_BLEND);
        // glDisable(GL_CULL_FACE);
    }
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

void draw_car_cover_image(int index)
{
    if(texture_3D_mode == 0) return;
    if(g_existedCover3d[index] == 0) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(programHandleCar[0]);

    glUniform1i(glGetUniformLocation(programHandleCar[0], "diffuseMap"), CAR_TEXTURE);
    glUniform1i(glGetUniformLocation(programHandleCar[0], "seethroughAlhpaOn"), 1.0);
    glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, g_texCover3d[index]);

    float IndeMatrix[16] =
    {
     1, 0, 0, 0,
     0, 1, 0, 0,
     0, 0, 1, 0,
     0, 0, 0, 1
    };

    float Inde[] = {0.0f, 0.0f, 0.0f};
    int glUniAxis = glGetUniformLocation(programHandleCar[0], "Axisdensity");
    float AxisMatrix[] = {
        1.0f , 0.0f  , 0.0f, 0.0f,
        0.0f , 1.0f  , 0.0f, 0.0f,
        0.0f , 0.0f  , 1.0f, 0.0f,
        0.0f , 0.0f  , 0.0f, 1.0f,
    };
    glUniformMatrix4fv(glUniAxis, 1, GL_FALSE, AxisMatrix);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "yRervsed"), yReverse);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "Shift"), 1, Inde);
    glUniform1i(glGetUniformLocation(programHandleCar[0], "windows"), 3);

    // MVP
    //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "MVP"), 1, GL_FALSE, IndeMatrix);

    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Model"), 1, GL_FALSE, IndeMatrix);
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Projection"), 1, GL_FALSE, IndeMatrix);
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "View"), 1, GL_FALSE, IndeMatrix);

    //MVPRotate
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "MVPRotate"), 1, GL_FALSE, IndeMatrix);

    //viewPos
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "viewPos"), 1, Inde);
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Model"), 1, GL_FALSE, IndeMatrix);

    // rudder
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "RUDDER_TIRE"), 1, GL_FALSE, IndeMatrix);

    glBindVertexArray(VAOCar3d[0]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void draw_car_3d_image(int index,int alphaOn)
{
    if(texture_3D_mode == 0) return;
    glDisable(GL_DEPTH_TEST);
    //glEnable(GL_DEPTH_TEST);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    glUseProgram(programHandleCar[0]);

    glUniform1i(glGetUniformLocation(programHandleCar[0], "diffuseMap"), CAR_TEXTURE);
    glUniform1i(glGetUniformLocation(programHandleCar[0], "seethroughAlhpaOn"), 1.0);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "alpha"), 1.0f);
    glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, g_texCar3d[index]);

    float IndeMatrix[16] =
    {
     1, 0, 0, 0,
     0, 1, 0, 0,
     0, 0, 1, 0,
     0, 0, 0, 1
    };

    float Inde[] = {0.0f, 0.0f, 0.0f};

    int glUniAxis = glGetUniformLocation(programHandleCar[0], "Axisdensity");
    float AxisMatrix[] = {
        1.0f , 0.0f  , 0.0f, 0.0f,
        0.0f , 1.0f  , 0.0f, 0.0f,
        0.0f , 0.0f  , 1.0f, 0.0f,
        0.0f , 0.0f  , 0.0f, 1.0f,
    };
    glUniformMatrix4fv(glUniAxis, 1, GL_FALSE, AxisMatrix);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "yRervsed"), yReverse);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "Shift"), 1, Inde);
    glUniform1i(glGetUniformLocation(programHandleCar[0], "windows"), 3);

    // MVP
    //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "MVP"), 1, GL_FALSE, IndeMatrix);

    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Model"), 1, GL_FALSE, IndeMatrix);
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Projection"), 1, GL_FALSE, IndeMatrix);
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "View"), 1, GL_FALSE, IndeMatrix);

    //MVPRotate
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "MVPRotate"), 1, GL_FALSE, IndeMatrix);

    //viewPos
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "viewPos"), 1, Inde);
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Model"), 1, GL_FALSE, IndeMatrix);

    // rudder
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "RUDDER_TIRE"), 1, GL_FALSE, IndeMatrix);

    glBindVertexArray(VAOCar3d[0]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
void draw_car_2d(float _alpha)
{
    glDisable(GL_DEPTH_TEST);
    //glEnable(GL_DEPTH_TEST);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    glUseProgram(programHandleCar[0]);

    glUniform1i(glGetUniformLocation(programHandleCar[0], "diffuseMap"), CAR_TEXTURE);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "alpha"), _alpha);
    glUniform1i(glGetUniformLocation(programHandleCar[0], "seethroughAlhpaOn"), 0);
    glActiveTexture(GL_TEXTURE0 + CAR_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, g_texCar2d);

    float IndeMatrix[16] =
    {
     1, 0, 0, 0,
     0, 1, 0, 0,
     0, 0, 1, 0,
     0, 0, 0, 1
    };

    float Inde[] = {0.0f, 0.0f, 0.0f};

    int glUniAxis = glGetUniformLocation(programHandleCar[0], "Axisdensity");
    float AxisMatrix[] = {
        1.0f , 0.0f  , 0.0f, 0.0f,
        0.0f , 1.0f  , 0.0f, 0.0f,
        0.0f , 0.0f  , 1.0f, 0.0f,
        0.0f , 0.0f  , 0.0f, 1.0f,
    };
    glUniformMatrix4fv(glUniAxis, 1, GL_FALSE, AxisMatrix);
    glUniform1f(glGetUniformLocation(programHandleCar[0], "yRervsed"), yReverse);
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "Shift"), 1, Inde);
    glUniform1i(glGetUniformLocation(programHandleCar[0], "windows"), 2);

    // MVP
    //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "MVP"), 1, GL_FALSE, IndeMatrix);

    //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Model"), 1, GL_FALSE, IndeMatrix);
    //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Projection"), 1, GL_FALSE, autosys_get_zoomin2DCARprojectionmatrix());
    //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "View"), 1, GL_FALSE, IndeMatrix);

    glUniAxis = glGetUniformLocation(programHandleCar[0], "clipworld");
    // float clipword[16];
    // memcpy(clipword,para.Rotate, 16*sizeof(float));
    // clipword[0] *= CarmodelContext->axis_density.x;
    // clipword[5] *= CarmodelContext->axis_density.y;
    // clipword[10] *= CarmodelContext->axis_density.z;
    glUniformMatrix4fv(glUniAxis, 1, GL_FALSE, autosys_get_zoomin2DCARprojectionmatrix());

    //MVPRotate
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "MVPRotate"), 1, GL_FALSE, IndeMatrix);

    //viewPos
    glUniform3fv(glGetUniformLocation(programHandleCar[0], "viewPos"), 1, Inde);
    //glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "Model"), 1, GL_FALSE, IndeMatrix);

    // rudder
    glUniformMatrix4fv(glGetUniformLocation(programHandleCar[0], "RUDDER_TIRE"), 1, GL_FALSE, IndeMatrix);

    glBindVertexArray(VAOCar2d[0]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
