#ifndef _GLFUNUTILS_H_
#define _GLFUNUTILS_H_

#include "avm/GLHeader.h"
#include "../setup/AVM_STRUCT.h"
extern void GL_COPY_FRAMEBUFFER_DEPTH(GLuint srcFbo, GLuint dstFbo,
    int srcOffsetX, int srcOffsetY, GLsizei srcW, GLsizei srcH,
    int dstOffsetX, int dstOffsetY, GLsizei dstW, GLsizei dstH);

extern void GL_FBO_AttachDepthRB(GLuint FBO, GLuint srcDepthObj);
extern void GL_Normal_Pixel2glpt(double x, double y, double* dstx, double* dsty, int w, int h);
extern void GL_Normal_glpt2Pixel(double x, double y, double* dstx, double* dsty, int w, int h);
extern void GL_transform_vec4_by_arr16(VECF4 vec, float* arr16, VECF4* result, _Bool isNormto2D);
#endif //_GLFUNUTILS_H_