/**
 *******************************************************************************
 * @file : camera_matrix.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20201220 0.1.0 Linda.
 *******************************************************************************
*/

// #include <GLES2/gl2.h>
// #include <GLES2/gl2ext.h>
// #include <GLES3/gl3.h>
// #include <GLES3/gl3ext.h>

#include <string.h>
#include <math.h>


#include "../../system.h"
#include "avm/maths.h"

#include "avm/camera_matrix.h"

static camera_windows_para_t WindowsPara;


void h3_matrix_multi(float *result, float *matrix1, float *matrix2) // (0 1 2 3) * (0 4 8 12) 
{
	result[0] = matrix1[0] * matrix2[0] + matrix1[4] * matrix2[1] + matrix1[8] * matrix2[2] + matrix1[12] * matrix2[3];
	result[4] = matrix1[0] * matrix2[4] + matrix1[4] * matrix2[5] + matrix1[8] * matrix2[6] + matrix1[12] * matrix2[7];
	result[8] = matrix1[0] * matrix2[8] + matrix1[4] * matrix2[9] + matrix1[8] * matrix2[10] + matrix1[12] * matrix2[11];
	result[12] = matrix1[0] * matrix2[12] + matrix1[4] * matrix2[13] + matrix1[8] * matrix2[14] + matrix1[12] * matrix2[15];

	result[1] = matrix1[1] * matrix2[0] + matrix1[5] * matrix2[1] + matrix1[9] * matrix2[2] + matrix1[13] * matrix2[3];
	result[5] = matrix1[1] * matrix2[4] + matrix1[5] * matrix2[5] + matrix1[9] * matrix2[6] + matrix1[13] * matrix2[7];
	result[9] = matrix1[1] * matrix2[8] + matrix1[5] * matrix2[9] + matrix1[9] * matrix2[10] + matrix1[13] * matrix2[11];
	result[13] = matrix1[1] * matrix2[12] + matrix1[5] * matrix2[13] + matrix1[9] * matrix2[14] + matrix1[13] * matrix2[15];

	result[2] = matrix1[2] * matrix2[0] + matrix1[6] * matrix2[1] + matrix1[10] * matrix2[2] + matrix1[14] * matrix2[3];
	result[6] = matrix1[2] * matrix2[4] + matrix1[6] * matrix2[5] + matrix1[10] * matrix2[6] + matrix1[14] * matrix2[7];
	result[10] = matrix1[2] * matrix2[8] + matrix1[6] * matrix2[9] + matrix1[10] * matrix2[10] + matrix1[14] * matrix2[11];
	result[14] = matrix1[2] * matrix2[12] + matrix1[6] * matrix2[13] + matrix1[10] * matrix2[14] + matrix1[14] * matrix2[15];

	result[3] = matrix1[3] * matrix2[0] + matrix1[7] * matrix2[1] + matrix1[11] * matrix2[2] + matrix1[15] * matrix2[3];
	result[7] = matrix1[3] * matrix2[4] + matrix1[7] * matrix2[5] + matrix1[11] * matrix2[6] + matrix1[15] * matrix2[7];
	result[11] = matrix1[3] * matrix2[8] + matrix1[7] * matrix2[9] + matrix1[11] * matrix2[10] + matrix1[15] * matrix2[11];
	result[15] = matrix1[3] * matrix2[12] + matrix1[7] * matrix2[13] + matrix1[11] * matrix2[14] + matrix1[15] * matrix2[15];
}

static float h3_dotvector3x3(float *a, float *b)
{
	return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

static void h3_normalizevector3x3(float *result, float *a)
{
	float length = 0;
	length = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	result[0] = a[0] / length;
	result[1] = a[1] / length;
	result[2] = a[2] / length;
}

static void  h3_corssvector3x3(float *result, float *a, float *b) //3x1 = 3x1 cross 3x1
{
	result[0] = a[1] * b[2] - a[2] * b[1];
	result[1] = a[2] * b[0] - a[0] * b[2];
	result[2] = a[0] * b[1] - a[1] * b[0];
}

static void h3_lookat(float *matrix, float *eyePosition3D, float *center3D, float *upVector3D)
{
	float forward[3], side[3], up[3];
	float matrix2[16], resultMatrix[16];
	int i;
	//------------------ -z
	forward[0] = (center3D[0] - eyePosition3D[0])* WindowsPara.CenX;
	forward[1] = (center3D[1] - eyePosition3D[1])* WindowsPara.CenY;
	forward[2] = (center3D[2] - eyePosition3D[2])* WindowsPara.CenZ;
	
	h3_normalizevector3x3(forward, forward);
	//------------------x = -z cross U
	//Side = forward x up
	//corssvector3x3(side, upVector3D, forward);
	h3_corssvector3x3(side, forward, upVector3D);
	h3_normalizevector3x3(side, side);
	//------------------y = x cross -z
	//Recompute up as: up = side x forward
	h3_corssvector3x3(up, side, forward);
	//------------------[x][y][z]

	matrix2[0] = side[0];	 	matrix2[4] = side[1];		matrix2[8] = side[2];		matrix2[12] = -h3_dotvector3x3(side, eyePosition3D);
	matrix2[1] = up[0];		 	matrix2[5] = up[1];			matrix2[9] = up[2];			matrix2[13] = -h3_dotvector3x3(up, eyePosition3D);
	matrix2[2] = -forward[0]; 	matrix2[6] = -forward[1];	matrix2[10] = -forward[2];	matrix2[14] = h3_dotvector3x3(forward, eyePosition3D);
	matrix2[3] = 0.0;		 	matrix2[7] = 0.0;			matrix2[11] = 0.0;			matrix2[15] = 1.0;

	//MultiplyMatrices4by4OpenGL_FLOAT(resultMatrix, matrix, matrix2);
	//glhTranslatef2(resultMatrix,-eyePosition3D[0], -eyePosition3D[1], -eyePosition3D[2]);
	//------------------
	memcpy(matrix, matrix2, 16 * sizeof(float));
}

static void  h3_glhFrustumf2(float *matrix, float left, float right, float bottom, float top, float znear, float zfar) //Perspective Projection Matrix  4x4
{
	float temp, temp2, temp3, temp4;
	temp = 2.0 * znear;
	temp2 = right - left;
	temp3 = top - bottom;
	temp4 = zfar - znear;

	matrix[0] = temp / temp2;	matrix[4] = 0.0;			matrix[8] = (right + left) / temp2;		matrix[12] = 0.0;
	matrix[1] = 0.0;			matrix[5] = temp / temp3;	matrix[9] = (top + bottom) / temp3;		matrix[13] = 0.0;
	matrix[2] = 0.0;			matrix[6] = 0.0;			matrix[10] = (-zfar - znear) / temp4;	matrix[14] = (-temp * zfar) / temp4;
	matrix[3] = 0.0;			matrix[7] = 0.0;			matrix[11] = -1.0;						matrix[15] = 0.0;

	// int i = 0;
	// AVM_LOGI(" %f %f %f %f %f %f\n", left, right, bottom, top, znear,  zfar);
	// AVM_LOGI(" %f %f %f %f\n",temp,temp2,temp3,temp4);
	//for(i = 0 ; i < 15 ; i++)
	//	AVM_LOGI("[%d]  %f\n",i,matrix[i]);

	// AVM_LOGI("\n");
	// for(j = 0 ; j < 4; j ++)
	// {
	//   for(i = 0 ; i < 4; i ++)
	//   {
	//	 int ptr = i +j * 4;
	//	 AVM_LOGI("%f\t",matrix[ptr]);					
	//   }
	//   AVM_LOGI("\n");
	// }
	// AVM_LOGI("\n");
}

static void  h3_glhPerspectivef2_3D(float *matrix, float fovyInDegrees, float aspectRatio,
	float znear, float zfar, float width, float height)
{
	float ymax, xmax;
	float temp, temp2, temp3, temp4;

	float windowXYRatio;
	//ymax = znear * tanf(fovyInDegrees * RADIAN / 2.0);
	//ymin = -ymax;
	//xmin = -ymax * aspectRatio;

	//ymax = znear * tanf(fovyInDegrees / 2);
	//xmax = ymax * aspectRatio;

	// windowUD = -0.002; //-0.002
	// windowLR = 0.184; //0.184

	windowXYRatio = width / height;

	xmax = WindowsPara.windowFov / WindowsPara.scaleLR ;
	// if(LR_flag == 0)
	// 	ymax = windowLR / 1.28;
	// else
	// ymax = windowLR / (1920.0 / 1080.0);
	ymax = WindowsPara.windowFov / windowXYRatio / WindowsPara.scaleUD;
	//AVM_LOGI("windowLR = %f  cen_xy = %f %f %f\n",windowLR,cen_x,cen_y,cen_z);
	//AVM_LOGI("xymax %f %f windowUD = %f\n",xmax,ymax,windowUD);
	// znear = 3;
	// zfar = 10;
	//AVM_LOGI("===========\n");

	h3_glhFrustumf2(matrix, -xmax + WindowsPara.windowLR, xmax + WindowsPara.windowLR, -ymax + WindowsPara.windowUD, ymax + WindowsPara.windowUD, znear, zfar);
}

static void  h3_glhPerspectivef2(float *matrix, camera_para_t * camera)
{
	intrinsic_t inPara = camera->inPara;
	extrinsic_t exPara = camera->exPara;

	float ymax, xmax;
	float temp, temp2, temp3, temp4;

	float left = -(inPara.horiFov)/2;
	float right = (inPara.horiFov)/2;
	float bottom = -(inPara.vertiFov)/2;
	float top = (inPara.vertiFov)/2;

	float znear = inPara.znear;
	float zfar =  inPara.zfar;

	if(camera->mode == 1)
	{
		temp = 2.0 * znear;
		temp2 = right - left;
		temp3 = top - bottom;
		temp4 = zfar - znear;

		matrix[0] = temp / temp2;	matrix[4] = 0.0;			matrix[8] = (right + left) / temp2;		matrix[12] = 0.0;
		matrix[1] = 0.0;			matrix[5] = temp / temp3;	matrix[9] = (top + bottom) / temp3;		matrix[13] = 0.0;
		matrix[2] = 0.0;			matrix[6] = 0.0;			matrix[10] = (-zfar - znear) / temp4;	matrix[14] = (-temp * zfar) / temp4;
		matrix[3] = 0.0;			matrix[7] = 0.0;			matrix[11] = -1.0;						matrix[15] = 0.0;
	}
	else if(camera->mode == 0)
	{
		temp = 2.0;
		temp2 = right - left;
		temp3 = top - bottom;
		temp4 = zfar - znear;

		matrix[0] = temp / temp2;	matrix[4] = 0.0;			matrix[8] = 0.0;			matrix[12] = -(right + left)/temp2;
		matrix[1] = 0.0;			matrix[5] = temp / temp3;	matrix[9] = 0.0;			matrix[13] = -(top + bottom) / temp3;
		matrix[2] = 0.0;			matrix[6] = 0.0;			matrix[10] = -2 / temp4;	matrix[14] = -(zfar + znear) / temp4;
		matrix[3] = 0.0;			matrix[7] = 0.0;			matrix[11] = 0.0;			matrix[15] = 1.0;
	}
}

void rotate_callculate_2D(camera_para_t * para)
{
	float Rotate[16], Rotate2[16], Rotate1[16];

	float phiDeg;
	float thetaDeg;

	float up_temp = 0;

	float  Eye[3] , Center[3],  Up[3] ;

	double eyeX2D;
	double eyeY2D;
	double eyeZ2D;

	if(para->exPara.theta > 360)
	{
		para->exPara.theta = 0;
	}

	phiDeg = DEG_TO_RAD(para->exPara.phi);
	thetaDeg = DEG_TO_RAD(para->exPara.theta);
	para->exPara.camera[0] = para->exPara.eyeRadius * sin(phiDeg) * cos(thetaDeg);
	para->exPara.camera[1] = para->exPara.eyeRadius * cos(phiDeg);
	para->exPara.camera[2] = para->exPara.eyeRadius * sin(phiDeg) * sin(thetaDeg);

	Eye[0] = para->exPara.camera[0] + para->exPara.translation[0];
	Eye[1] = para->exPara.camera[1] + para->exPara.translation[1];
	Eye[2] = para->exPara.camera[2] + para->exPara.translation[2];

	Center[0] = Eye[0];
	Center[1] = 0.0;
	Center[2] = Eye[2];
	
	Up[0] = para->exPara.up[0];
	Up[1] = para->exPara.up[1];
	Up[2] = para->exPara.up[2];

	float transformMatrix[] =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	h3_glhPerspectivef2(para->Projection, para);//投影矩陣 Camera internal parameter. camera to pixel.

	h3_lookat(para->View, Eye, Center, Up); // Camera setting location. world to camera.

	h3_matrix_multi(Rotate, para->Projection, para->View);

	h3_matrix_multi(para->Rotate, para->Model, Rotate); //World location.
	//h3_matrix_multi(para->Rotate, transformMatrix, Rotate); //World location.
}

void rotate_callculate(camera_para_t * para)
{
	float Rotate[16], Rotate2[16], Rotate1[16];

	float phiDeg;
	float thetaDeg;

	float up_temp = 0;

	float  Eye[3] , Center[3],  Up[3] ;

	double eyeX2D;
	double eyeY2D;
	double eyeZ2D;

	if(para->exPara.theta > 360)
	{
		para->exPara.theta = 0;
	}

	phiDeg = DEG_TO_RAD(para->exPara.phi);
	thetaDeg = DEG_TO_RAD(para->exPara.theta);

	para->exPara.camera[0] = para->exPara.eyeRadius * sin(phiDeg) * cos(thetaDeg);
	para->exPara.camera[1] = para->exPara.eyeRadius * cos(phiDeg);
	para->exPara.camera[2] = para->exPara.eyeRadius * sin(phiDeg) * sin(thetaDeg);

	Eye[0] = para->exPara.camera[0] + para->exPara.tmpCenter[0] + para->exPara.tmpCenter[6];
	Eye[1] = para->exPara.camera[1] + para->exPara.tmpCenter[1] + para->exPara.tmpCenter[7];
	Eye[2] = para->exPara.camera[2] + para->exPara.tmpCenter[2] + para->exPara.tmpCenter[8];

	Center[0] = -para->exPara.camera[0] + para->exPara.tmpCenter[0] + para->exPara.tmpCenter[12];
	Center[1] = -para->exPara.camera[1] + para->exPara.tmpCenter[1] + para->exPara.tmpCenter[13]; 
	Center[2] = -para->exPara.camera[2] + para->exPara.tmpCenter[2] + para->exPara.tmpCenter[14];

	if (para->exPara.phi <= 0)
		up_temp = 1;

	if (para->exPara.phi > 0)
		up_temp = -1;

	Up[0] = 0;
	Up[1] = up_temp;
	Up[2] = 0;

	float transformMatrix[] = 
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};//odject to world.

	h3_glhPerspectivef2(para->Projection, para);//投影矩陣 Camera internal parameter. camera to pixel.

	h3_lookat(para->View, Eye, Center, Up); // Camera setting location. world to camera.

	h3_matrix_multi(Rotate, para->Projection, para->View);

	h3_matrix_multi(para->Rotate, para->Model, Rotate); //World location.
}

void init_camera_para(void)
{
	WindowsPara.scaleLR = 1.0;
	WindowsPara.scaleUD = 1.0;
	WindowsPara.windowUD = 0.000;
	WindowsPara.windowLR = 0.000;
	WindowsPara.windowFov = 0.14; //0.184

	WindowsPara.CenX = 1.0;
	WindowsPara.CenY = 1.0;
	WindowsPara.CenZ = 1.0;
}

