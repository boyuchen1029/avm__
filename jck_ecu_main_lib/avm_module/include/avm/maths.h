/*************************************************************************/ /*!
@Title          Matrix maths
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef __MATHS_H__
#define __MATHS_H__

// #ifdef __GNUC__
// #define __internal __attribute__((visibility("hidden")))
// #else
// #define __internal
// #endif
#define SQUARE(x) ((x)*(x))
#define DEG_TO_RAD(theta) ((theta)*0.01745329251994329576923690768489)
#define RAD_TO_DEG(rad) ((rad)*57.29577951308232087679815481410)
#define max(a,b) ((a)>(b))?(a):(b);
#include "../../../setup/AVM_STRUCT.h"
void myFrustum(float pMatrix[4][4], float left, float right, float bottom, float top, float zNear, float zFar);
void myPersp(float pMatrix[4][4], float fovy, float aspect, float zNear, float zFar);
void myTranslate(float pMatrix[4][4], float fX, float fY, float fZ);
void myScale(float pMatrix[4][4], float fX, float fY, float fZ);
void myRotate(float pMatrix[4][4], float fX, float fY, float fZ, float angle);
void myIdentity(float pMatrix[4][4]);
void myMultMatrix(float psRes[4][4], float psSrcA[4][4], float psSrcB[4][4]);
void myInvertTransposeMatrix(float pDstMatrix[3][3], float pSrcMatrix[4][4]);
extern VECF4 VECF4_Transform(VECF4 A, float* B);
extern VECF4 VECF4_Mutiple_4by1(VECF4 A, float* B);
extern float calculateEuclideanDistance(PointF p1, PointF p2);
extern void myCreateRotate4(int axis, float angle, float* out);
extern void myMatrixMultiplyby4(float* out, float* a, float* b);
extern void myMultiplyMat4_opengl(float* result, const float* a, const float* b);
#endif /* __MATHS_H__ */
