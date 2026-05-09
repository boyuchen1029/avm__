/**
 *******************************************************************************
 * @file : fp_source.c
 * @describe : .
 *
 * @author : Bennit Yang
 * @verstion : 0.1.0.
 * @date 20201103 0.1.0 Bennit Yang.
 * @date 20201222 0.1.1 Linda.
 *******************************************************************************
*/
//
// Created by AI611 on 2021/11/3.
//
#include "../../system.h"
#include "avm/GLHeader.h"
#include "user_control/ui.h"
#include "avm/fp_source.h"
#include <sys/stat.h>
#include "display_location.h"
#include "avm/stb_image.h"
#include <unistd.h>
#include "../autosys/autosys_IVI.h"
float scope_W, scope_H;
float fbo2d_W, fbo2d_H;

extern int Car_2d_W;
extern int Car_2d_H;

void print_error(FILE * fp, char * dir)
{
    if(fp == 0)
    {
        AVM_LOGI("\033[31m[ReadFile] %s No File.\033[0m\n", dir);
    }else{
        AVM_LOGI("\033[35m[ReadFile]\033[0m %s\n", dir);
    }
}

void print_img_error(unsigned char* data, char * dir)
{
    if(data == NULL)
    {
        AVM_LOGI("\033[31m[ReadIMG] %s No File.\033[0m\n", dir);
    }
    else
    {
        AVM_LOGI("\033[35m[ReadIMG]\033[0m %s\n", dir);
    }
}

FILE * is_file_larger_than_zero(FILE *fp, const char * tarDir)
{
    struct stat statbuf;
    stat(tarDir, &statbuf);
    if (fp != NULL && (statbuf.st_size < sizeof(int) * 1))
    {
        fclose(fp);
        fp = NULL;
    }
    return fp;
}

FILE * fp_source_avm(const char * tarDir, const char * mode)
{
    FILE * fp;
    char dir[1024] = "\0";
    sprintf(dir, "%s%s",EVSTable_path[1] , tarDir);

    fp = fopen(dir, mode);
    if(fp != 0)
    {
        g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, CALIBRATION);
        fp = is_file_larger_than_zero(fp, dir);
        if(fp == 0)
        {
            g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_FLASH);
        }
    }
 
    if(fp == 0)
    {
        sprintf(dir, "%s%s",EVSTable_path[0] , tarDir);
        AVM_LOGI(LOG_RED("[avm file Error]")", So using application [%s] file\n", dir);
        fp = is_file_larger_than_zero(fopen(dir, mode), dir);
    }
    else
    {
        #ifdef __CRC_SYSTEM_WRITE__
        if(BINARY_CRC_CHECK(fp) == 0)
        {
            g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_FLASH);
            fclose(fp);
            sprintf(dir, "%s%s",EVSTable_path[0] , tarDir);
            AVM_LOGI(LOG_RED("[CRC Fail]")", So using application [%s] file\n", dir);
            fp = is_file_larger_than_zero(fopen(dir, mode), dir);
        }
        #endif
    }
    print_error(fp, dir);
    return fp;
}

FILE * fp_source_avm_nonCRC(const char * tarDir, const char * mode, bool isretry)
{
    FILE * fp;
    char dir[1024] = "\0";
    sprintf(dir, "%s%s",EVSTable_path[1] , tarDir);

    fp = fopen(dir, mode);
    if(fp != 0)
    {
        g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, CALIBRATION);
        fp = is_file_larger_than_zero(fp, dir);
        if(fp == 0)
        {
            g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_FLASH);
        }
    }

    if(fp == 0)
    {
        if (isretry == true)
        {
            sprintf(dir, "%s%s", EVSTable_path[0], tarDir);
            AVM_LOGI(LOG_RED("[avm file Error]") ", So using application [%s] file\n", dir);
            fp = is_file_larger_than_zero(fopen(dir, mode), dir);
        }
    }
    print_error(fp, dir);
    return fp;
}

FILE * fp_source_app(const char * tarDir, const char * mode)
{
    FILE * fp;
    char dir[1024] = "\0";
    sprintf(dir, "%s%s", EVSTable_path[0], tarDir);
    fp = fopen(dir, mode);
    print_error(fp, dir);
    return fp;
}

FILE * fp_source(const char * tarDir, const char * mode)
{
    FILE * fp;
    char dir[1024] = "\0";
    sprintf(dir, "%s%s", EVSTable_path[0], tarDir);
    fp = is_file_larger_than_zero(fopen(dir, mode), dir);
    print_error(fp, dir);
    return fp;
}
void fp_close_data_safty(FILE *fp)
{
    if(fp != NULL)
    {
        fclose(fp);
    }
}

bool fp_read_data_safty(void* dst, size_t size, size_t n, FILE *fp)
{
    if (!fp || !dst || size == 0 || n == 0) 
    {
        return false;
    }

    long current_pos = ftell(fp);
    if (current_pos == -1L) 
    {
        return false; 
    }

    if (fseek(fp, 0, SEEK_END) != 0) 
    {
        return false;
    }

    long end_pos = ftell(fp);
    if (end_pos == -1L) 
    {
        return false;
    }


    if (fseek(fp, current_pos, SEEK_SET) != 0) 
    {
        return false;
    }

    long bytes_remaining = end_pos - current_pos;
    size_t total_bytes_to_read = size * n;

    if ((long)total_bytes_to_read > bytes_remaining) 
    {
        return false; 
    }

    size_t actually_read = fread(dst, size, n, fp);
    return (actually_read == n); 
}
void fp_read_image_app(const char* tarDir, unsigned char** data, int* W, int* H, int* ch, int desired_ch, int filp)
{
    //AVM_LOGI("\033[35m[ReadIMG]\033[0m %s\n", tarDir);
    #if 0
    //if(IMGMODE == IMG_CV){
	    *data = (unsigned char *)calloc(*W * *H * *ch * sizeof(unsigned char), sizeof(unsigned char));
        readPNG(tarDir, W, H, ch, *data);
    //}
    //else if(IMGMODE == IMG_STB){
    #else
        char dir[1024] = "\0";
        sprintf(dir, "%s%s", EVSTable_path[0], tarDir);
        stbi_set_flip_vertically_on_load(filp);
        if(access(dir, F_OK) == -1)
        {
            
        }
        else
        {
            *data = stbi_load(dir, W, H, ch, desired_ch);
        }
        stbi_set_flip_vertically_on_load(0);
    //}
    #endif
    AVM_LOGI("Size(%d, %d, %d)\n", *W, *H, *ch);
    print_img_error(*data, dir);
}
void fp_read_image_avm(const char* tarDir, unsigned char** data, int* W, int* H, int* ch, int desired_ch, int filp)
{
    //AVM_LOGI("\033[35m[ReadIMG]\033[0m %s\n", tarDir);
    #if 0
    //if(IMGMODE == IMG_CV){
	    *data = (unsigned char *)calloc(*W * *H * *ch * sizeof(unsigned char), sizeof(unsigned char));
        readPNG(tarDir, W, H, ch, *data);
    //}
    //else if(IMGMODE == IMG_STB){
    #else
        char dir[1024] = "\0";
        sprintf(dir, "%s%s", EVSTable_path[0], tarDir);
        stbi_set_flip_vertically_on_load(filp);
        // if(access(dir, F_OK) == -1)
        // {
        //     sprintf(dir, "%s%s", EVSTable_path[0], tarDir);
        //     AVM_LOGI(LOG_RED("[avm file Error]")", So using application [%s] file\n", dir);
        //     if(access(dir, F_OK) == 0)
        //     {
        //         *data = stbi_load(dir, W, H, ch, desired_ch);
        //     }
        // }
        // else
        {
            g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, NON_CALIBRATION);
            *data = stbi_load(dir, W, H, ch, desired_ch);
            if(*data == NULL) g_calibration_info.status =  UPDATE_CALIB_STATUS(g_calibration_info.status, ADNORMAL_FLASH);
        }
        stbi_set_flip_vertically_on_load(0);
    //}
    #endif
    if(*W == 0 && *H == 0)
    {
        AVM_LOGI(LOG_RED("[ERROR]")"Image is empty in %s\n", tarDir);
    }
    else
    {
        AVM_LOGI("Size(%d, %d, %d)\n", *W, *H, *ch);
    }
    print_img_error(*data, dir);
}

void fp_read_image(const char* tarDir, unsigned char** data, int* W, int* H, int* ch, int filp){
    AVM_LOGI("\033[35m[ReadFile]\033[0m %s\n", tarDir);
    #if 0
    //if(IMGMODE == IMG_CV){
	    *data = (unsigned char *)calloc(*W * *H * *ch * sizeof(unsigned char), sizeof(unsigned char));
        readPNG(tarDir, W, H, ch, *data);
    //}
    //else if(IMGMODE == IMG_STB){
    #else
        stbi_set_flip_vertically_on_load(filp);
        *data = stbi_load(tarDir, W, H, ch, *ch);
        stbi_set_flip_vertically_on_load(0);
    //}
    #endif
    AVM_LOGI("Size(%d, %d, %d)\n", *W, *H, *ch);
}


FILE * fp_source_r(const char * tarDir)
{
    FILE * fp;
    char dir[100] = "\0";
    sprintf(dir, "%s/%s", SOURCE_DIR, tarDir);
    fp = fopen(dir, "r");
    print_error(fp, dir);
    return fp;
}

FILE * fp_source_rb(const char * tarDir)
{
    FILE * fp;
    char dir[100] = "\0";
    sprintf(dir, "%s", tarDir);
    fp = fopen(dir, "rb");
    print_error(fp, dir);
    return fp;
}

FILE * fp_source_w(const char * tarDir)
{
    FILE * fp;
    char dir[100] = "\0";
    sprintf(dir, "%s/%s", SOURCE_DIR, tarDir);
    fp = fopen(dir, "w");
    print_error(fp, dir);
    return fp;
}

FILE * fp_source_wb(const char * tarDir)
{
    FILE * fp;
    char dir[100] = "\0";
    sprintf(dir, "%s/%s", SOURCE_DIR, tarDir);
    fp = fopen(dir, "wb");
    print_error(fp, dir);
    return fp;
}

FILE * fp_source_a(const char * tarDir)
{
    FILE * fp;
    char dir[100] = "\0";
    sprintf(dir, "%s/%s", SOURCE_DIR, tarDir);
    fp = fopen(dir, "a");
    print_error(fp, dir);
    return fp;
}

FILE * fp_source_avm_r(const char * tarDir)
{
    FILE * fp;
    char dir[100] = "\0";
    sprintf(dir, "%s", tarDir);
    fp = fopen(dir, "r");
    print_error(fp, dir);
    return fp;
}

FILE * fp_source_avm_rb(const char * tarDir)
{
    FILE * fp;
    char dir[100] = "\0";
    sprintf(dir, "%s", tarDir);
    fp = fopen(dir, "rb");
    print_error(fp, dir);
    return fp;
}

void save_log_frame(const char * data, char * Writeout)
{
    FILE * fp =fp_source_a(Writeout);
    
    fprintf(fp, "%s",data);
        
    fclose(fp);
}

void fwrite_rgb(unsigned char * img, int w, int h, const char * fileName)
{
    AVM_LOGI("save rgb\n");
    FILE * result;
    char z[100];
    sprintf(z, "%s/%s", SOURCE_DIR, fileName);
    result = fopen(z, "wb");
    fwrite(img, 1, w * h * 3, result);
    fclose(result);
}

void fwrite_rgba(unsigned char * img, int w, int h, const char * fileName)
{
    printf("save rgba\n");
    FILE * result;
    char z[100];
    sprintf(z, "%s/%s", SOURCE_DIR, fileName);
    result = fopen(z, "wb");
    fwrite(img, 1, w * h * 4, result);
    fclose(result);
}

void fwrite_readpixels(int w, int h, const char * fileName) //note: You have to give file storage permission in MTK
{
    unsigned char *buffer;
    buffer = (unsigned char *)calloc(w * h * 4 * sizeof(unsigned char), sizeof(unsigned char));
    glReadPixels( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    //glReadPixels( 0, 0, 768/2, 774, GL_RGBA, GL_UNSIGNED_BYTE, bufferRGBout);

    FILE * resultS;
    AVM_LOGI("saving windows readpixels\n");
    char z[50];
    sprintf(z, "%s/%s", SOURCE_DIR, fileName);
    resultS = fopen(z, "wb");
    fwrite(buffer, 1, w * h * 4, resultS);
    fclose(resultS);
    free(buffer);
}

void save_para_info(camera_para_t * para)
{
    FILE * statusBar = fp_source_w("para.txt");
    
    fprintf(statusBar, "phi %f\n", para->exPara.phi);
    fprintf(statusBar, "theta %f\n", para->exPara.theta);
    fprintf(statusBar, "eyeRadius %f\n", para->exPara.eyeRadius);
    fprintf(statusBar, "tmpCenter[0] %f\n", para->exPara.tmpCenter[0]);
    fprintf(statusBar, "tmpCenter[1] %f\n", para->exPara.tmpCenter[1]);
    fprintf(statusBar, "tmpCenter[2] %f\n", para->exPara.tmpCenter[2]);
    fprintf(statusBar, "tmpCenter[3] %f\n", para->exPara.tmpCenter[3]);
    fprintf(statusBar, "tmpCenter[4] %f\n", para->exPara.tmpCenter[4]);
    fprintf(statusBar, "tmpCenter[5] %f\n", para->exPara.tmpCenter[5]);
    fprintf(statusBar, "tmpCenter[6] %f\n", para->exPara.tmpCenter[6]);
    fprintf(statusBar, "tmpCenter[7] %f\n", para->exPara.tmpCenter[7]);
    fprintf(statusBar, "tmpCenter[8] %f\n", para->exPara.tmpCenter[8]);
    fprintf(statusBar, "tmpCenter[9] %f\n", para->exPara.tmpCenter[9]);
    fprintf(statusBar, "tmpCenter[10] %f\n", para->exPara.tmpCenter[10]);
    fprintf(statusBar, "tmpCenter[11] %f\n", para->exPara.tmpCenter[11]);
    fprintf(statusBar, "tmpCenter[12] %f\n", para->exPara.tmpCenter[12]);
    fprintf(statusBar, "tmpCenter[13] %f\n", para->exPara.tmpCenter[13]);
    fprintf(statusBar, "tmpCenter[14] %f\n", para->exPara.tmpCenter[14]);
    fclose (statusBar);
}

void save_data(int * data)
{
    FILE * statusBar = fp_source_w("data.txt");
    
    fprintf(statusBar, "data %d\n", *data);
    fclose (statusBar);
}

void read_car_size(int *width, int *height)
{
    FILE * carSize = fp_source_r("output_avm_calibration/car_size.txt");
    
    fscanf(carSize, "width: %d\n",width);
    fscanf(carSize, "height: %d\n",height);
    AVM_LOGI("car width %d height %d\n", *width, *height);

    fclose(carSize);
}

void read_2Dlimit(trans_camera_t *avm_2d, int *width, int *height)
{
    int nonused = 0;
    float avmW = 0.0, avmH = 0.0;
    float innerW = 0.0, innerH = 0.0;
    FILE * carSize;

    carSize = fp_source_avm("/EVSTable/avm/avm2d_limit.bin", "rb");
    if (carSize != NULL)
    {
#ifdef __CRC_SYSTEM_WRITE__
        fread(&nonused, 1, 1 * sizeof(int), carSize);
#endif
        fread(&innerW, 1, 1 * sizeof(float), carSize);
        fread(&innerH, 1, 1 * sizeof(float), carSize);
        fread(&avmW, 1, 1 * sizeof(float), carSize);
        fread(&avmH, 1, 1 * sizeof(float), carSize);

        scope_W = avmW * 2.0;
        scope_H = avmH * 2.0;
        *width  = innerW;
        *height = innerH;

        Car_2d_W = (int)(((scope_W + *width) / (scope_H + *height) * 720.0));
        Car_2d_H = 720;

        fbo2d_W = (int)(((scope_W + *width) / (scope_H + *height) * 720.0) / 4.0 + 1.0) * 4.0;
        // fbo2d_W = 488.0;
        fbo2d_H = 720.0;

        fread(&avm_2d[0].front, 1, 1 * sizeof(int), carSize);
        fread(&avm_2d[1].front, 1, 1 * sizeof(int), carSize);
        fread(&avm_2d[0].back, 1, 1 * sizeof(int), carSize);
        fread(&avm_2d[1].back, 1, 1 * sizeof(int), carSize);
        fread(&avm_2d[0].left, 1, 1 * sizeof(int), carSize);
        fread(&avm_2d[1].left, 1, 1 * sizeof(int), carSize);
        fread(&avm_2d[0].right, 1, 1 * sizeof(int), carSize);
        fread(&avm_2d[1].right, 1, 1 * sizeof(int), carSize);

        // printf("inner W %d H %d \n", *width, *height);
        // printf("avm scope W %f H %f\n", avmW, avmH);
        // printf("avm fbo W %f H %f\n", fbo2d_W, fbo2d_H);
        fclose(carSize);
    }
    else
    {
        AVM_LOGI(LOG_RED("[ERROR]")"Read avm_2d_limit file error!!!\n");
		AVM_LOGI("[WARNING] Using default value!!!\n");
        avmW = 100.0;   avmH = 100.0;
        innerW = 100.0; innerH = 100.0;
        scope_W = avmW * 2.0;
        scope_H = avmH * 2.0;
        *width  = innerW;
        *height = innerH;

        Car_2d_W = (int)(((scope_W + *width) / (scope_H + *height) * 720.0));
        Car_2d_H = 720;

        fbo2d_W = (int)(((scope_W + *width) / (scope_H + *height) * 720.0) / 4.0 + 1.0) * 4.0;
        fbo2d_H = 720.0;
    }
    AVM_LOGI("2Davm start Front %d Back %d Left %d Right %d\n", avm_2d[0].front, avm_2d[0].back, avm_2d[0].left, avm_2d[0].right);
    AVM_LOGI("2Davm  end  Front %d Back %d Left %d Right %d\n", avm_2d[1].front, avm_2d[1].back, avm_2d[1].left, avm_2d[1].right);
}

int BINARY_CRC_CHECK(FILE *fp)
{
    if(fp == NULL) return 0;
    //start_key
    int read_start_key = 0;
    int read_end_key = 0;
    fread(&read_start_key, 1, 1*sizeof(int), fp);
    fseek(fp, -1 * (int)sizeof(int), SEEK_END);
    fread(&read_end_key, 1, 1*sizeof(int), fp);
    fseek(fp, 0, SEEK_SET);
    return (read_start_key == binary_crc_start_key)&&(read_end_key == binary_crc_end_key);
}


int FlashReadDataStatus(const char *fileDir)
{
    FILE *fp = fopen(fileDir, "rb");
    AVM_LOGI("\033[31m[ReadFile] %s.\033[0m\n", fileDir);
    if (fp == NULL) return 0;
    if (is_file_larger_than_zero(fp, fileDir) == NULL) return 0;
    int result = BINARY_CRC_CHECK(fp);
    AVM_LOGI("\033[35m[ReadFile] result %d\033[0m\n", result);
    fclose(fp);
    return result;
}


void get_file_path(const char *fileDir, int index, char *output_buffer, size_t size)
{
    snprintf(output_buffer, size, "%s%s", EVSTable_path[index], fileDir);
}