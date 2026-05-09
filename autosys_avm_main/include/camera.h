/**
 *******************************************************************************
 * @file     : camera.h
 * @describe :  Header for camera.c file.
 * 
 * @author : Woody.
 * @verstion : 0.1.0.
 * @date 20211108 0.1.0 Woody.
 * @date 20211220 0.1.1 Linda.
 *******************************************************************************
*/
#ifndef _CAMERA_H_
#define _CAMERA_H_

typedef enum
{
    V4L2_STATUS_OK = 0,
    V4L2_STATUS_OPEN_FAIL,
    V4L2_STATUS_PREPARE_FAIL,
    V4L2_STATUS_START_FAIL,
    V4L2_STATUS_ROUTINE_FAIL,
} v4l2_status_t;
/*
extern void deQdraw_test(void);
extern int Qcamera(void);

extern void get_camera_buf(unsigned char ** cameraFrontBuf,
                           unsigned char ** cameraLeftBuf,
                           unsigned char ** cameraRightBuf,
                           unsigned char ** cameraBackBuf);
*/
extern void get_cam_va_from_8368_p( const unsigned char * cameraBufF,
                                    const unsigned char * cameraBufB,
                                    const unsigned char * cameraBufL,
                                    const unsigned char * cameraBufR);

extern void pass_campa( unsigned int cameraPAF,
                        unsigned int cameraPAB,
                        unsigned int cameraPAL,
                        unsigned int cameraPAR);

#endif //_CAMERA_H_