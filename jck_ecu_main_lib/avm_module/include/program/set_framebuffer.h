/**
 *******************************************************************************
 * @file : set_framebuffer.h
 * @describe : Header of set_framebuffer.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211227 0.1.0 Linda.
 *******************************************************************************
*/

#ifndef _SET_FRAMEBUFFER_H_
#define _SET_FRAMEBUFFER_H_

#include "avm/GLHeader.h"


#define MRT_NUM 2

typedef struct
{
    GLuint framebuffer;
    GLuint texture;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
    GLuint stencilRenderbuffer;
    GLenum status;
    GLenum errorDetection;
    GLint params;
    GLint attachmentType;
    GLint attachmentName;
    GLint width;
    GLint height;
    GLint maxRenderbufferSize;
    GLuint textureMRT[MRT_NUM];
    GLuint accumColorTex;
    GLuint accumAlphaTex;
}FBO_t;

extern GLenum setting_fbo(FBO_t *customize, int texWidth, int texHeight, GLenum texture, GLint param);

extern void create_fbo(FBO_t *customize, int texWidth, int texHeight, GLenum texture, GLint param);

extern GLenum setting_multi_fbo(FBO_t *customize, int texWidth, int texHeight, GLenum texture, GLint param);

extern void create_multi_fbo(FBO_t *customize, int texWidth, int texHeight, GLenum texture, GLint param);
extern GLenum setting_fbo_MRT(FBO_t *customize, int texWidth, int texHeight, GLint param);
extern void create_fbo_MRT(FBO_t *customize, int texWidth, int texHeight, GLint param);
extern void create_fbo_OIT(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param, int isShareDepth, GLuint depthid);
extern void create_fbo_BLUR(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param);
extern void destroy_fbo(FBO_t *customize);
#endif //_SET_FRAMEBUFFER_H_