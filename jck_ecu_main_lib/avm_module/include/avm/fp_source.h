/**
 *******************************************************************************
 * @file     : fp_source.h
 * @describe : Header for fp_source.c file
 * 
 * @author : Bennit Yang.
 * @verstion : 0.1.0.
 * @date 20211115 0.1.0 Bennit Yang.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/
//
// Created by AI611 on 2021/11/15.
//

#ifndef _FP_SOURCE_H_
#define _FP_SOURCE_H_

#include "../../system.h"
#include "../../include/avm/camera_matrix.h"
#include "../../include/user_control/ui.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

extern FILE * fp_source_avm(const char * tarDir, const char * mode);
extern FILE * fp_source_app(const char * tarDir, const char * mode);
extern bool   fp_read_data_safty(void* dst, size_t size, size_t n, FILE *fp);
extern void   fp_close_data_safty(FILE *fp);
extern FILE * fp_source_avm_nonCRC(const char * tarDir, const char * mode, bool isretry);
extern void   fp_read_image_avm(const char* tarDir, unsigned char** data, int* W, int* H, int* ch, int desired_ch, int filp);
extern void   fp_read_image_app(const char* tarDir, unsigned char** data, int* W, int* H, int* ch, int desired_ch, int filp);

/**
 * Function :
 *  Open the file as read-only.
 * 
 * Parameter :
 *  intput :
 *      char * tarDir : Filename to open.
 * 
 * Return :  
 *      A FILE pointer, if open fail return NULL.
*/
extern FILE * fp_source_r(const char * tarDir);

/**
 * Function :
 *  Open binary file as read-only.
 * 
 * Parameter :
 *  intput :
 *      char * tarDir : Filename to open.
 * 
 * Return :  
 *      A FILE pointer, if open fail return NULL.
*/
extern FILE * fp_source_rb(const char * tarDir);

/**
 * Function :
 *  Create an empty file.
 * 
 * Parameter :
 *  intput :
 *      char * tarDir : Filename to open.
 * 
 * Return :  
 *      A FILE pointer, if open fail return NULL.
*/
extern FILE * fp_source_w(const char * tarDir);

/**
 * Function :
 *  Create an empty binary file.
 * 
 * Parameter :
 *  intput :
 *      char * tarDir : Filename to open.
 * 
 * Return :  
 *      A FILE pointer, if open fail return NULL.
*/
extern FILE * fp_source_wb(const char * tarDir);

/**
 * Function :
 *  Create an empty binary file.
 * 
 * Parameter :
 *  intput :
 *      char * tarDir : Filename to open.
 * 
 * Return :  
 *      A FILE pointer, if open fail return NULL.
*/
extern FILE * fp_source_a(const char * tarDir);

extern FILE * fp_source_avm_r(const char * tarDir);

extern FILE * fp_source_avm_rb(const char * tarDir);

extern FILE * fp_source(const char * tarDir, const char * mode);//Atlas20250926

extern void save_log_frame(const char * data, char * Writeout);

extern void fwrite_rgb(unsigned char * img, int w, int h, const char * fileName);

extern void fwrite_rgba(unsigned char * img, int w, int h, const char * fileName);

extern void fwrite_readpixels(int w, int h, const char * fileName);

extern void save_para_info(camera_para_t * para);

extern void save_data(int * data);

extern void read_car_size(int *width, int *height);

extern void read_2Dlimit(trans_camera_t *avm_2d, int *width, int *height);

extern int BINARY_CRC_CHECK(FILE *fp);

extern int FlashReadDataStatus(const char *fileDir);

extern void get_file_path(const char *fileDir, int index, char *output_buffer, size_t size);
#endif //_FP_SOURCE_H_