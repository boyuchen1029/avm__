/*************************************************************************/ /*!
@Title          Matrix maths
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include <math.h>

#include "avm/maths.h"

#define GLES_Zero							0.0f
#define GLES_One							1.0f
#define GLES_Pi								((float) 3.14159265358979323846)
#define GLES_DegreesToRadians				(GLES_Pi / (float) 180.0)


VECF4 VECF4_Transform(VECF4 A, float* B)
{
    VECF4 opt;
    opt.X = B[0] * A.X + B[4] * A.Y + B[8] * A.Z + B[12] * A.W;
    opt.Y = B[1] * A.X + B[5] * A.Y + B[9] * A.Z + B[13] * A.W;
    opt.Z = B[2] * A.X + B[6] * A.Y + B[10]* A.Z + B[14] * A.W;
    opt.W = B[3] * A.X + B[7] * A.Y + B[11] * A.Z + B[15] * A.W;
    return opt;
}

VECF4 VECF4_Mutiple_4by1(VECF4 A, float* B)
{
    VECF4 opt;
    opt.X = B[0] * A.X + B[1] * A.Y + B[2] * A.Z + B[3] * A.W;
    //opt.Y = B[1] * A.X + B[5] * A.Y + B[9] * A.Z + B[13] * A.W;
    opt.Z = B[8] * A.X + B[9] * A.Y + B[10]* A.Z + B[11] * A.W;
    //opt.W = ;
    return opt;
}
void translate_xyz(float* result, const float translatex,
		const float translatey, const float translatez) {
	result[12] += result[0] * translatex + result[4] * translatey
			+ result[8] * translatez;
	result[13] += result[1] * translatex + result[5] * translatey
			+ result[9] * translatez;
	result[14] += result[2] * translatex + result[6] * translatey
			+ result[10] * translatez;
	result[15] += result[3] * translatex + result[7] * translatey
			+ result[11] * translatez;
}

float length(const float x, const float y, const float z) {
	return (float) sqrt(x*x + y*y +z*z);
}

float calculateEuclideanDistance(PointF p1, PointF p2)
{
	float dx = p1.X - p2.X;
	float dy = p1.Y - p2.Y;
	return (float)sqrtf(dx * dx + dy * dy);
}

void view_set_lookat(float* result,
		const float eyex, const float eyey, const float eyez,
		const float centerx, const float centery, const float centerz,
		const float upx, const float upy, const float upz) 
{

	float fx = centerx - eyex;
	float fy = centery - eyey;
	float fz = centerz - eyez;

	// normalize f
	float rlf = 1.0f / length(fx, fy, fz);
	fx *= rlf;
	fy *= rlf;
	fz *= rlf;

	// compute s = f x up (x means "cross product")
    float sx = fy * upz - fz * upy;
    float sy = fz * upx - fx * upz;
    float sz = fx * upy - fy * upx;

    // and normalize s
    float rls = 1.0f / length(sx, sy, sz);
    sx *= rls;
    sy *= rls;
    sz *= rls;

    //The up vector must not be parallel to the line of sight from the eye point to the reference point.
    if((0 == sx)&&(0 == sy)&&(0 == sz))
    	return;
    	
    // compute u = s x f
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    result[0] = sx;
    result[1] = ux;
    result[2] = -fx;
    result[3] = 0.0f;

    result[4] = sy;
    result[5] = uy;
    result[6] = -fy;
    result[7] = 0.0f;

    result[8] = sz;
    result[9] = uz;
    result[10] = -fz;
    result[11] = 0.0f;

    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = 0.0f;
    result[15] = 1.0f;

    translate_xyz(result, -eyex, -eyey, -eyez);
}

static float Normalize(float afVout[3], float afVin[3])
{
	float fLen;

	fLen = afVin[0]*afVin[0] + afVin[1]*afVin[1] + afVin[2]*afVin[2];

	if (fLen <= GLES_Zero)
	{
		afVout[0] = GLES_Zero;
		afVout[1] = GLES_Zero;
		afVout[2] = GLES_Zero;
		return fLen;
	}

	if (fLen == 1.0F)
	{
		afVout[0] = afVin[0];
		afVout[1] = afVin[1];
		afVout[2] = afVin[2];
		return fLen;
	}
	else 
	{
		/*
		 * This code could calculates a reciprocal square root accurate to well over
		 * 16 bits using Newton-Raphson approximation.
		 */

		fLen = ((float) 1.0) / (float)sqrt((double)fLen);

		afVout[0] = afVin[0] * fLen;
		afVout[1] = afVin[1] * fLen;
		afVout[2] = afVin[2] * fLen;
		return fLen;
	}
}

void myMultMatrix(float psRes[4][4], float psSrcA[4][4], float psSrcB[4][4])
{
	float fB00, fB01, fB02, fB03;
	float fB10, fB11, fB12, fB13;
	float fB20, fB21, fB22, fB23;
	float fB30, fB31, fB32, fB33;
	int i;

	fB00 = psSrcB[0][0]; fB01 = psSrcB[0][1];
	fB02 = psSrcB[0][2]; fB03 = psSrcB[0][3];
	fB10 = psSrcB[1][0]; fB11 = psSrcB[1][1];
	fB12 = psSrcB[1][2]; fB13 = psSrcB[1][3];
	fB20 = psSrcB[2][0]; fB21 = psSrcB[2][1];
	fB22 = psSrcB[2][2]; fB23 = psSrcB[2][3];
	fB30 = psSrcB[3][0]; fB31 = psSrcB[3][1];
	fB32 = psSrcB[3][2]; fB33 = psSrcB[3][3];

	for (i = 0; i < 4; i++) 
	{
		psRes[i][0] = psSrcA[i][0]*fB00 + psSrcA[i][1]*fB10	+ psSrcA[i][2]*fB20 + psSrcA[i][3]*fB30;
		psRes[i][1] = psSrcA[i][0]*fB01 + psSrcA[i][1]*fB11	+ psSrcA[i][2]*fB21 + psSrcA[i][3]*fB31;
		psRes[i][2] = psSrcA[i][0]*fB02 + psSrcA[i][1]*fB12	+ psSrcA[i][2]*fB22 + psSrcA[i][3]*fB32;
		psRes[i][3] = psSrcA[i][0]*fB03 + psSrcA[i][1]*fB13	+ psSrcA[i][2]*fB23 + psSrcA[i][3]*fB33;
	}
}

void myPersp(float pMatrix[4][4], float fovy, float aspect, float zNear, float zFar)
{

	float sine, cotangent, deltaZ;
	float radians;
	float m[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

	radians = fovy / 2 * GLES_Pi / 180;

	deltaZ = zFar - zNear;
	sine = (float) sin(radians);
	if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
	return;
	}
	cotangent = (float) cos(radians) / sine;

	m[0][0] = cotangent / aspect;
	m[1][1] = cotangent;
	m[2][2] = -(zFar + zNear) / deltaZ;
	m[2][3] = -1;
	m[3][2] = -2 * zNear * zFar / deltaZ;
	m[3][3] = 0;

	myMultMatrix(pMatrix, m, pMatrix);
}

void myFrustum(float pMatrix[4][4], float left, float right, float bottom, float top, float zNear, float zFar)
{

	float deltaX, deltaY, deltaZ;
	float m[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

	deltaX = right - left;
	deltaY = top - bottom;
	deltaZ = zFar - zNear;

	if ((deltaZ == 0) || (deltaX == 0) || (deltaY == 0))
	{
		return;
	}

	m[0][0] = 2 * zNear / deltaX;
	m[1][1] = 2 * zNear / deltaY;
	m[2][0] = (left + right)/ deltaX;
	m[2][1] = (top + bottom)/ deltaY;
	m[2][2] = -(zFar + zNear) / deltaZ;
	m[2][3] = -1;
	m[3][2] = -2 * zNear * zFar / deltaZ;
	m[3][3] = 0;

	myMultMatrix(pMatrix, m, pMatrix);
}

void myScale(float pMatrix[4][4], float fX, float fY, float fZ)
{
	float fM0, fM1, fM2, fM3;

	fM0 = fX * pMatrix[0][0];
	fM1 = fX * pMatrix[0][1];
	fM2 = fX * pMatrix[0][2];
	fM3 = fX * pMatrix[0][3];
	pMatrix[0][0] = fM0;
	pMatrix[0][1] = fM1;
	pMatrix[0][2] = fM2;
	pMatrix[0][3] = fM3;

	fM0 = fY * pMatrix[1][0];
	fM1 = fY * pMatrix[1][1];
	fM2 = fY * pMatrix[1][2];
	fM3 = fY * pMatrix[1][3];
	pMatrix[1][0] = fM0;
	pMatrix[1][1] = fM1;
	pMatrix[1][2] = fM2;
	pMatrix[1][3] = fM3;

	fM0 = fZ * pMatrix[2][0];
	fM1 = fZ * pMatrix[2][1];
	fM2 = fZ * pMatrix[2][2];
	fM3 = fZ * pMatrix[2][3];
	pMatrix[2][0] = fM0;
	pMatrix[2][1] = fM1;
	pMatrix[2][2] = fM2;
	pMatrix[2][3] = fM3;
}

void myTranslate(float pMatrix[4][4], float fX, float fY, float fZ)
{
	float fM30, fM31, fM32, fM33;

	fM30 =	fX * pMatrix[0][0] + fY * pMatrix[1][0] + fZ * 
			pMatrix[2][0] + pMatrix[3][0];

	fM31 =	fX * pMatrix[0][1] + fY * pMatrix[1][1] + fZ * 
			pMatrix[2][1] + pMatrix[3][1];

	fM32 =	fX * pMatrix[0][2] + fY * pMatrix[1][2] + fZ * 
			pMatrix[2][2] + pMatrix[3][2];

	fM33 =	fX * pMatrix[0][3] + fY * pMatrix[1][3] + fZ * 
			pMatrix[2][3] + pMatrix[3][3];

	pMatrix[3][0] = fM30;
	pMatrix[3][1] = fM31;
	pMatrix[3][2] = fM32;
	pMatrix[3][3] = fM33;
}

void myRotate(float pMatrix[4][4], float fX, float fY, float fZ, float fAngle)
{
	float fRadians, fSine, fCosine, fAB, fBC, fCA, fT;
	float afAv[4], afAxis[4];
	float afMatrix[4][4];

	afAv[0] = fX;
	afAv[1] = fY;
	afAv[2] = fZ;
	afAv[3] = 0;

	Normalize(afAxis, afAv);

	fRadians = fAngle * GLES_DegreesToRadians;
	fSine = (float)sin(fRadians);
	fCosine = (float)cos(fRadians);
	
	fAB = afAxis[0] * afAxis[1] * (1 - fCosine);
	fBC = afAxis[1] * afAxis[2] * (1 - fCosine);
	fCA = afAxis[2] * afAxis[0] * (1 - fCosine);

	myIdentity(afMatrix);

	fT = afAxis[0] * afAxis[0];
	afMatrix[0][0] = fT + fCosine * (1 - fT);
	afMatrix[2][1] = fBC - afAxis[0] * fSine;
	afMatrix[1][2] = fBC + afAxis[0] * fSine;

	fT = afAxis[1] * afAxis[1];
	afMatrix[1][1] = fT + fCosine * (1 - fT);
	afMatrix[2][0] = fCA + afAxis[1] * fSine;
	afMatrix[0][2] = fCA - afAxis[1] * fSine;

	fT = afAxis[2] * afAxis[2];
	afMatrix[2][2] = fT + fCosine * (1 - fT);
	afMatrix[1][0] = fAB - afAxis[2] * fSine;
	afMatrix[0][1] = fAB + afAxis[2] * fSine;

	// int ii;
	// for(ii =0; ii < 4; ii ++)
	// {
	// 	printf("afMatrix[%d][0] = %f\n", ii,  afMatrix[ii][0]);
	// 	printf("afMatrix[%d][1] = %f\n", ii,  afMatrix[ii][1]);
	// 	printf("afMatrix[%d][2] = %f\n", ii,  afMatrix[ii][2]);
	// 	printf("afMatrix[%d][3] = %f\n", ii,  afMatrix[ii][3]);
	// }
	myMultMatrix(pMatrix, afMatrix, pMatrix);
}

void myIdentity(float pMatrix[4][4])
{
	pMatrix[0][0] = 1.0f;
	pMatrix[0][1] = 0.0f;
	pMatrix[0][2] = 0.0f;
	pMatrix[0][3] = 0.0f;

	pMatrix[1][0] = 0.0f;
	pMatrix[1][1] = 1.0f;
	pMatrix[1][2] = 0.0f;
	pMatrix[1][3] = 0.0f;

	pMatrix[2][0] = 0.0f;
	pMatrix[2][1] = 0.0f;
	pMatrix[2][2] = 1.0f;
	pMatrix[2][3] = 0.0f;

	pMatrix[3][0] = 0.0f;
	pMatrix[3][1] = 0.0f;
	pMatrix[3][2] = 0.0f;
	pMatrix[3][3] = 1.0f;

}

void myInvertTransposeMatrix(float pDstMatrix[3][3], float pSrcMatrix[4][4])
{
	float fX00, fX01, fX02;
	float fX10, fX11, fX12;
	float fX20, fX21, fX22;
	float fRcp;
	float fX30, fX31, fX32;
	float fY01, fY02, fY03, fY12, fY13, fY23;
	float fZ02, fZ12, fZ22;
	#if 0
		float fZ03, fZ13, fZ23, fZ32, fZ33;
	#endif

	#define fX03 fX01
	#define fX13 fX11
	#define fX23 fX21
	#define fX33 fX31
	#define fZ00 fX02
	#define fZ10 fX12
	#define fZ20 fX22
	#define fZ30 fX32
	#define fZ01 fX03
	#define fZ11 fX13
	#define fZ21 fX23
	#define fZ31 fX33

		/* read 1st two columns of matrix into registers */
	fX00 = pSrcMatrix[0][0];
	fX01 = pSrcMatrix[0][1];
	fX10 = pSrcMatrix[1][0];
	fX11 = pSrcMatrix[1][1];
	fX20 = pSrcMatrix[2][0];
	fX21 = pSrcMatrix[2][1];
	fX30 = pSrcMatrix[3][0];
	fX31 = pSrcMatrix[3][1];

	/* compute all six 2x2 determinants of 1st two columns */
	fY01 = fX00*fX11 - fX10*fX01;
	fY02 = fX00*fX21 - fX20*fX01;
	fY03 = fX00*fX31 - fX30*fX01;
	fY12 = fX10*fX21 - fX20*fX11;
	fY13 = fX10*fX31 - fX30*fX11;
	fY23 = fX20*fX31 - fX30*fX21;

	/* read 2nd two columns of matrix into registers */
	fX02 = pSrcMatrix[0][2];
	fX03 = pSrcMatrix[0][3];
	fX12 = pSrcMatrix[1][2];
	fX13 = pSrcMatrix[1][3];
	fX22 = pSrcMatrix[2][2];
	fX23 = pSrcMatrix[2][3];
	fX32 = pSrcMatrix[3][2];
	fX33 = pSrcMatrix[3][3];

	/* compute all 3x3 cofactors for 2nd two columns */
	#if 0
		fZ33 = fX02*fY12 - fX12*fY02 + fX22*fY01;
		fZ23 = fX12*fY03 - fX32*fY01 - fX02*fY13;
		fZ13 = fX02*fY23 - fX22*fY03 + fX32*fY02;
		fZ03 = fX22*fY13 - fX32*fY12 - fX12*fY23;
		fZ32 = fX13*fY02 - fX23*fY01 - fX03*fY12;
	#endif
	fZ22 = fX03*fY13 - fX13*fY03 + fX33*fY01;
	fZ12 = fX23*fY03 - fX33*fY02 - fX03*fY23;
	fZ02 = fX13*fY23 - fX23*fY13 + fX33*fY12;

	/* compute all six 2x2 determinants of 2nd two columns */
	fY01 = fX02*fX13 - fX12*fX03;
	fY02 = fX02*fX23 - fX22*fX03;
	fY03 = fX02*fX33 - fX32*fX03;
	fY12 = fX12*fX23 - fX22*fX13;
	fY13 = fX12*fX33 - fX32*fX13;
	fY23 = fX22*fX33 - fX32*fX23;

	/* read 1st two columns of matrix into registers */
	fX00 = pSrcMatrix[0][0];
	fX01 = pSrcMatrix[0][1];
	fX10 = pSrcMatrix[1][0];
	fX11 = pSrcMatrix[1][1];
	fX20 = pSrcMatrix[2][0];
	fX21 = pSrcMatrix[2][1];
	fX30 = pSrcMatrix[3][0];
	fX31 = pSrcMatrix[3][1];

	/* compute all 3x3 cofactors for 1st column */
	fZ30 = fX11*fY02 - fX21*fY01 - fX01*fY12;
	fZ20 = fX01*fY13 - fX11*fY03 + fX31*fY01;
	fZ10 = fX21*fY03 - fX31*fY02 - fX01*fY23;
	fZ00 = fX11*fY23 - fX21*fY13 + fX31*fY12;

	/* compute 4fX4 determinant & its reciprocal */
	fRcp = fX30*fZ30 + fX20*fZ20 + fX10*fZ10 + fX00*fZ00;

	if (fRcp == GLES_Zero)
	{
		return;
	}

	fRcp = GLES_One/fRcp;

	/* compute all 3x3 cofactors for 2nd column */
	fZ31 = fX00*fY12 - fX10*fY02 + fX20*fY01;
	fZ21 = fX10*fY03 - fX30*fY01 - fX00*fY13;
	fZ11 = fX00*fY23 - fX20*fY03 + fX30*fY02;
	fZ01 = fX20*fY13 - fX30*fY12 - fX10*fY23;

	/* multiply all 3fX3 cofactors by reciprocal */
	pDstMatrix[0][0] = fZ00*fRcp;
	pDstMatrix[0][1] = fZ01*fRcp;
	pDstMatrix[1][0] = fZ10*fRcp;
	pDstMatrix[0][2] = fZ02*fRcp;
	pDstMatrix[2][0] = fZ20*fRcp;
	#if 0
		pDstMatrix[0][3] = fZ03*fRcp;
		pDstMatrix[3][0] = fZ30*fRcp;
	#endif
		pDstMatrix[1][1] = fZ11*fRcp;
		pDstMatrix[1][2] = fZ12*fRcp;
		pDstMatrix[2][1] = fZ21*fRcp;
	#if 0
		pDstMatrix[1][3] = fZ13*fRcp;
		pDstMatrix[3][1] = fZ31*fRcp;
	#endif
		pDstMatrix[2][2] = fZ22*fRcp;
	#if 0
		pDstMatrix[2][3] = fZ23*fRcp;
		pDstMatrix[3][2] = fZ32*fRcp;
		pDstMatrix[3][3] = fZ33*fRcp;
	#endif
}

void myMultiplyMat4_opengl(float* result, const float* a, const float* b)
{
	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
		{
			result[col * 4 + row] =
				a[0 * 4 + row] * b[col * 4 + 0] +
				a[1 * 4 + row] * b[col * 4 + 1] +
				a[2 * 4 + row] * b[col * 4 + 2] +
				a[3 * 4 + row] * b[col * 4 + 3];
		}
	}
}

void myCreateRotate4(int axis, float angle, float* out)
{
	float angleRad = angle * GLES_DegreesToRadians;
	for (int i = 0; i < 16; ++i)
        out[i] = (i % 5) == 0 ? 1.0f : 0.0f;
	float c = cosf(angleRad), s = sinf(angleRad);
	switch (axis) 
	{
        case 0: // X-axis
            out[5]  =  c; out[6]  =  s;
            out[9]  = -s; out[10] =  c;
            break;
        case 1: // Y-axis
            out[0]  =  c; out[2]  = -s;
            out[8]  =  s; out[10] =  c;
            break;
        case 2: // Z-axis
            out[0]  =  c; out[1]  =  s;
            out[4]  = -s; out[5]  =  c;
            break;
        default:
            break;
    }
}

void myMatrixMultiplyby4(float* out, float* a, float* b) 
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) 
		{
			out[i * 4 + j] = a[i * 4 + 0] * b[0 * 4 + j] + a[i * 4 + 1] * b[1 * 4 + j] + a[i * 4 + 2] * b[2 * 4 + j] + a[i * 4 + 3] * b[3 * 4 + j];
		}
	}
}