/**
 *******************************************************************************
 * @file : camera_matrix.h
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20201220 0.1.0 Linda.
 *******************************************************************************
*/

#ifndef _CAMERA_MATRIX_H_
#define _CAMERA_MATRIX_H_


typedef struct
{
	float width;
	float height;
	float znear;
	float zfar;
	double horiFov;  // horizontal field of view
	double vertiFov; // vertical field of view

}intrinsic_t;

typedef struct
{
	float phi;
	float theta;

	double eyeRadius;
	double tmpCenter[15];
	double translation[3];

	float camera[3];
	float eye[3];
	float center[3];
	float up[3];

}extrinsic_t;

typedef struct
{
	intrinsic_t inPara;
	extrinsic_t exPara;
	char mode;
	float Rotate[16];
	float Model[16];
	float View[16];
	float Projection[16];
	float ModelToWorld[9];
}camera_para_t;


typedef struct
{
	float windowLR;
	float windowUD;
	float windowFov;

	float scaleLR;
	float scaleUD;

	float CenX;
	float CenY;
	float CenZ; 
	
}camera_windows_para_t;

extern void h3_matrix_multi(float* result, float* matrix1, float* matrix2);

extern void rotate_callculate_2D(camera_para_t * para);

extern void rotate_callculate(camera_para_t * para);

extern void init_camera_para(void);

#endif //_CAMERA_MATRIX_H_