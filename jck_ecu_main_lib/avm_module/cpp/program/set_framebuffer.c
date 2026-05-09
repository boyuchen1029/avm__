/**
 *******************************************************************************
 * @file : set_framebuffer.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 2021/12/27 0.1.0 Linda.
 *******************************************************************************
*/

#include <stdio.h>
#include <string.h>

#include <math.h>

#include "../../system.h"
#include "avm/GLHeader.h"
#include "../setup/AVM_PARAM.h"
#include "program/set_framebuffer.h"

GLenum setting_fbo(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param)
{
    GLint maxRenderbufferSize = 0;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);

    if(texWidth > maxRenderbufferSize || texWidth > maxRenderbufferSize)
    {
        AVM_LOGI("Exceed the maximum size!!");
        AVM_LOGI("(Renderbuffer size: %u)\n", customize->maxRenderbufferSize);
    }
    customize->width = texWidth, customize->height = texHeight;

    glGenFramebuffers(1, &customize->framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glGenTextures(1, &customize->texture);
    glGenRenderbuffers(1, &customize->colorRenderbuffer);
    glGenRenderbuffers(1, &customize->depthRenderbuffer);
    glGenRenderbuffers(1, &customize->stencilRenderbuffer);

    // ===================== texture object ===================== //
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, customize->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, customize->width, customize->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, customize->texture, (GLuint)NULL);

    // ===================== render object ===================== //
    glBindRenderbuffer(GL_RENDERBUFFER, customize->depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, customize->width, customize->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, customize->depthRenderbuffer);

    // 檢查attach 在fbo 的object type

    AVM_LOGI("ID:\n");
    AVM_LOGI("\tFramebuffers: %u\n", customize->framebuffer);
    AVM_LOGI("\ttextures: %u\n", customize->texture);
    AVM_LOGI("\tcolorRenderbuffer: %u\n", customize->colorRenderbuffer);
    AVM_LOGI("\tdepthRenderbuffer: %u\n", customize->depthRenderbuffer);
    AVM_LOGI("\tstencilRenderbuffer: %u\n", customize->stencilRenderbuffer);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Texture: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %u\n", customize->attachmentName);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Depth: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %x\n", customize->attachmentName);

    customize->status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (customize->status == GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("Framebuffer is complete\n");
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        AVM_LOGI("Please check framebuffer: %x\n", customize->status);
        glDeleteFramebuffers(1, &customize->framebuffer);
        glDeleteRenderbuffers(1, &customize->colorRenderbuffer);
        glDeleteRenderbuffers(1, &customize->depthRenderbuffer);
        glDeleteRenderbuffers(1, &customize->stencilRenderbuffer);
        glDeleteTextures(1, &customize->texture);
    }
    return customize->status;
}

void create_fbo(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param)
{
    memset(customize, 0, sizeof(FBO_t));
    customize->errorDetection = -1;
// setting_fbo(customize, texWidth, texHeight, texUnit, param);
    if(setting_fbo(customize, texWidth, texHeight, texUnit, param) != GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    // glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 將狀態改回系統預設
}













GLenum setting_fbo_MRT(FBO_t *customize, int texWidth, int texHeight, GLint param)
{

    GLint maxRenderbufferSize = 0;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);

    if(texWidth > maxRenderbufferSize || texWidth > maxRenderbufferSize)
    {
        AVM_LOGI("Exceed the maximum size!!");
        AVM_LOGI("(Renderbuffer size: %u)\n", customize->maxRenderbufferSize);
    }
    customize->width = texWidth, customize->height = texHeight;

    const GLenum attachments[MRT_NUM] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1
    };
    glGenFramebuffers(1, &customize->framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glGenTextures(MRT_NUM, customize->textureMRT);
    glGenRenderbuffers(1, &customize->colorRenderbuffer);
    glGenRenderbuffers(1, &customize->depthRenderbuffer);
    glGenRenderbuffers(1, &customize->stencilRenderbuffer);

    // ===================== texture object ===================== //
   //glActiveTexture(GL_TEXTURE0 + texUnit);
   for(int i=0;i<MRT_NUM;i++)
   {
        glBindTexture(GL_TEXTURE_2D, customize->textureMRT[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, customize->width, customize->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, customize->textureMRT[i], 0);
   }

    glDrawBuffers(MRT_NUM, attachments);
    // ===================== render object ===================== //
    glBindRenderbuffer(GL_RENDERBUFFER, customize->depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, customize->width, customize->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, customize->depthRenderbuffer);

    // // 檢查attach 在fbo 的object type

    AVM_LOGI("ID:\n");
    AVM_LOGI("\tFramebuffers: %u\n", customize->framebuffer);
    AVM_LOGI("\ttextures: %u\n", customize->texture);
    AVM_LOGI("\tcolorRenderbuffer: %u\n", customize->colorRenderbuffer);
    AVM_LOGI("\tdepthRenderbuffer: %u\n", customize->depthRenderbuffer);
    AVM_LOGI("\tstencilRenderbuffer: %u\n", customize->stencilRenderbuffer);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Texture0: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %u\n", customize->attachmentName);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Texture1: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %u\n", customize->attachmentName);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Depth: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %x\n", customize->attachmentName);

    customize->status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (customize->status == GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("Framebuffer is complete\n");
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        AVM_LOGI("Please check framebuffer: %x\n", customize->status);
        glDeleteFramebuffers(1, &customize->framebuffer);
        glDeleteRenderbuffers(1, &customize->colorRenderbuffer);
        glDeleteRenderbuffers(1, &customize->depthRenderbuffer);
        glDeleteRenderbuffers(1, &customize->stencilRenderbuffer);
        glDeleteTextures(1, &customize->texture);
    }
    return customize->status;
}

void destroy_fbo(FBO_t *customize)
{
    if (customize == NULL) return;

    GLint curFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &curFbo);
    if ((GLuint)curFbo == customize->framebuffer)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (customize->framebuffer) {
        glDeleteFramebuffers(1, &customize->framebuffer);
        customize->framebuffer = 0;
    }

    if (customize->texture) {
        glDeleteTextures(1, &customize->texture);
        customize->texture = 0;
    }

#ifdef MRT_NUM
    for (int i = 0; i < MRT_NUM; i++) {
        if (customize->textureMRT[i]) {
            glDeleteTextures(1, &customize->textureMRT[i]);
            customize->textureMRT[i] = 0;
        }
    }
#endif

    if (customize->colorRenderbuffer) {
        glDeleteRenderbuffers(1, &customize->colorRenderbuffer);
        customize->colorRenderbuffer = 0;
    }
    if (customize->depthRenderbuffer) {
        glDeleteRenderbuffers(1, &customize->depthRenderbuffer);
        customize->depthRenderbuffer = 0;
    }
    if (customize->stencilRenderbuffer) {
        glDeleteRenderbuffers(1, &customize->stencilRenderbuffer);
        customize->stencilRenderbuffer = 0;
    }

    customize->width = 0;
    customize->height = 0;
    customize->status = 0;
    customize->attachmentType = 0;
    customize->attachmentName = 0;
    customize->errorDetection = 0;
}
GLenum setting_fbo_OIT(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param, int isShareDepth, GLuint depthid)
{
    GLint maxRenderbufferSize = 0;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);

    if(texWidth > maxRenderbufferSize || texWidth > maxRenderbufferSize)
    {
        AVM_LOGI("Exceed the maximum size!!");
        AVM_LOGI("(Renderbuffer size: %u)\n", customize->maxRenderbufferSize);
    }
    customize->width = texWidth, customize->height = texHeight;

    glGenFramebuffers(1, &customize->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);
    // ===================== texture object ===================== //
    glGenTextures(1, &customize->accumColorTex);
    glBindTexture(GL_TEXTURE_2D, customize->accumColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, customize->width, customize->height, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, customize->accumColorTex, (GLuint)0);

#if PARAM_OPENGL_OIT_COLOR_ALPHA_INDIVIUAL
    glGenTextures(1, &customize->accumAlphaTex);
    glBindTexture(GL_TEXTURE_2D, customize->accumAlphaTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, customize->width, customize->height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, customize->accumAlphaTex, (GLuint)0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);
#else
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);
#endif

    // ===================== render object ===================== //
    if(isShareDepth == 0)
    {
        glGenRenderbuffers(1, &customize->depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, customize->depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, customize->width, customize->height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, customize->depthRenderbuffer);
    }
    else
    {
        customize->depthRenderbuffer = depthid;
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, customize->depthRenderbuffer);
    }

    // 檢查attach 在fbo 的object type

    AVM_LOGI("ID:\n");
    AVM_LOGI("\tFramebuffers: %u\n", customize->framebuffer);
    AVM_LOGI("\taccumColortextures: %u\n", customize->accumColorTex);
    AVM_LOGI("\taccumAlphatextures: %u\n", customize->accumAlphaTex);
    AVM_LOGI("\tdepthRenderbuffer: %u\n", customize->depthRenderbuffer);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Texture: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %u\n", customize->attachmentName);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Depth: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %x\n", customize->attachmentName);

    customize->status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return customize->status;
}

void create_fbo_OIT(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param, int isShareDepth, GLuint depthid)
{
    memset(customize, 0, sizeof(FBO_t));
    customize->errorDetection = -1;
// setting_fbo(customize, texWidth, texHeight, texUnit, param);
    if(setting_fbo_OIT(customize, texWidth, texHeight, texUnit, param, isShareDepth, depthid) != GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    // glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 將狀態改回系統預設
}

void create_fbo_MRT(FBO_t *customize, int texWidth, int texHeight, GLint param)
{
    memset(customize, 0, sizeof(FBO_t));
    customize->errorDetection = -1;

    if(setting_fbo_MRT(customize, texWidth, texHeight, param) != GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    // glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 將狀態改回系統預設
}

GLenum setting_multi_fbo(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param)
{
    int multiSampleCount = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &multiSampleCount);
    AVM_LOGI("MSAA sample count = %d\n", multiSampleCount);

    GLint maxRenderbufferSize = 0;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);

    if(texWidth > maxRenderbufferSize || texWidth > maxRenderbufferSize)
    {
        AVM_LOGI("Exceed the maximum size!!");
        AVM_LOGI("(Renderbuffer size: %u)\n", customize->maxRenderbufferSize);
    }
    customize->width = texWidth, customize->height = texHeight;

    glGenFramebuffers(1, &customize->framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glGenTextures(1, &customize->texture);
    glGenRenderbuffers(1, &customize->colorRenderbuffer);
    glGenRenderbuffers(1, &customize->depthRenderbuffer);
    glGenRenderbuffers(1, &customize->stencilRenderbuffer);

    // ===================== texture object ===================== //
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, customize->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, customize->width, customize->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, customize->texture, (GLuint)NULL);

    // ===================== render object ===================== //
    glBindRenderbuffer(GL_RENDERBUFFER, customize->depthRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT24, customize->width, customize->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, customize->colorRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGBA8, customize->width, customize->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, customize->depthRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, customize->colorRenderbuffer);

    // // 檢查attach 在fbo 的object type

    AVM_LOGI("ID:\n");
    AVM_LOGI("\tFramebuffers: %u\n", customize->framebuffer);
    AVM_LOGI("\ttextures: %u\n", customize->texture);
    AVM_LOGI("\tcolorRenderbuffer: %u\n", customize->colorRenderbuffer);
    AVM_LOGI("\tdepthRenderbuffer: %u\n", customize->depthRenderbuffer);
    AVM_LOGI("\tstencilRenderbuffer: %u\n", customize->stencilRenderbuffer);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Texture: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %u\n", customize->attachmentName);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Depth: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %x\n", customize->attachmentName);

    customize->status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (customize->status == GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("Framebuffer is complete\n");
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        AVM_LOGI("Please check framebuffer: %x\n", customize->status);
        glDeleteFramebuffers(1, &customize->framebuffer);
        glDeleteRenderbuffers(1, &customize->colorRenderbuffer);
        glDeleteRenderbuffers(1, &customize->depthRenderbuffer);
        glDeleteRenderbuffers(1, &customize->stencilRenderbuffer);
        glDeleteTextures(1, &customize->texture);
    }
    return customize->status;
}

void create_multi_fbo(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param)
{
    memset(customize, 0, sizeof(FBO_t));
    customize->errorDetection = -1;

    if(setting_multi_fbo(customize, texWidth, texHeight, texUnit, param) != GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    // glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 將狀態改回系統預設
}




GLenum setting_fbo_BLUR(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param)
{
    #if 0
    GLint maxRenderbufferSize = 0;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);

    if(texWidth > maxRenderbufferSize || texWidth > maxRenderbufferSize)
    {
        AVM_LOGI("Exceed the maximum size!!");
        AVM_LOGI("(Renderbuffer size: %u)\n", customize->maxRenderbufferSize);
    }
    customize->width = texWidth, customize->height = texHeight;

    glGenFramebuffers(1, &customize->framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glGenTextures(1, &customize->texture);
    glGenRenderbuffers(1, &customize->colorRenderbuffer);
    glGenRenderbuffers(1, &customize->depthRenderbuffer);
    glGenRenderbuffers(1, &customize->stencilRenderbuffer);

    // ===================== texture object ===================== //
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, customize->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, customize->width, customize->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, customize->texture, (GLuint)NULL);

    // ===================== render object ===================== //
    glBindRenderbuffer(GL_RENDERBUFFER, customize->depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, customize->width, customize->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, customize->depthRenderbuffer);

    // 檢查attach 在fbo 的object type

    AVM_LOGI("ID:\n");
    AVM_LOGI("\tFramebuffers: %u\n", customize->framebuffer);
    AVM_LOGI("\ttextures: %u\n", customize->texture);
    AVM_LOGI("\tcolorRenderbuffer: %u\n", customize->colorRenderbuffer);
    AVM_LOGI("\tdepthRenderbuffer: %u\n", customize->depthRenderbuffer);
    AVM_LOGI("\tstencilRenderbuffer: %u\n", customize->stencilRenderbuffer);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Texture: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %u\n", customize->attachmentName);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Depth: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %x\n", customize->attachmentName);

    customize->status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (customize->status == GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("Framebuffer is complete\n");
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        AVM_LOGI("Please check framebuffer: %x\n", customize->status);
        glDeleteFramebuffers(1, &customize->framebuffer);
        glDeleteRenderbuffers(1, &customize->colorRenderbuffer);
        glDeleteRenderbuffers(1, &customize->depthRenderbuffer);
        glDeleteRenderbuffers(1, &customize->stencilRenderbuffer);
        glDeleteTextures(1, &customize->texture);
    }
    return customize->status;
    #endif

    #if 1
    GLint maxRenderbufferSize = 0;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);

    if(texWidth > maxRenderbufferSize || texWidth > maxRenderbufferSize)
    {
        AVM_LOGI("Exceed the maximum size!!");
        AVM_LOGI("(Renderbuffer size: %u)\n", customize->maxRenderbufferSize);
    }
    customize->width = texWidth, customize->height = texHeight;

    glGenFramebuffers(1, &customize->framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glGenTextures(1, &customize->texture);
    glGenRenderbuffers(1, &customize->colorRenderbuffer);
    
    // ===================== texture object ===================== //
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, customize->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, customize->width, customize->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    #if 1
    // GLfloat aniso = 0.0f;
    // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  3);
    glGenerateMipmap(GL_TEXTURE_2D);
    #endif
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, customize->texture, (GLuint)NULL);

    // glDrawBuffer(GL_COLOR_ATTACHMENT0);
    // glReadBuffer(GL_COLOR_ATTACHMENT0);
    // 檢查attach 在fbo 的object type

    AVM_LOGI("ID:\n");
    AVM_LOGI("\tFramebuffers: %u\n", customize->framebuffer);
    AVM_LOGI("\ttextures: %u\n", customize->texture);
    AVM_LOGI("\tcolorRenderbuffer: %u\n", customize->colorRenderbuffer);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &customize->attachmentType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &customize->attachmentName);
    AVM_LOGI("FBO-----Texture: \n");
    AVM_LOGI("\tobject type: %x\n", customize->attachmentType);
    AVM_LOGI("\tobject name: %u\n", customize->attachmentName);

    customize->status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (customize->status == GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("Framebuffer is complete\n");
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        AVM_LOGI("Please check framebuffer: %x\n", customize->status);
        glDeleteFramebuffers(1, &customize->framebuffer);
        glDeleteRenderbuffers(1, &customize->colorRenderbuffer);
        glDeleteTextures(1, &customize->texture);
    }
    return customize->status;
    #endif
}

void create_fbo_BLUR(FBO_t *customize, int texWidth, int texHeight, GLenum texUnit, GLint param)
{
    memset(customize, 0, sizeof(FBO_t));
    customize->errorDetection = -1;
// setting_fbo(customize, texWidth, texHeight, texUnit, param);
    if(setting_fbo_BLUR(customize, texWidth, texHeight, texUnit, param) != GL_FRAMEBUFFER_COMPLETE)
    {
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
        AVM_LOGI("setting_fbo fail\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, customize->framebuffer);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    // glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 將狀態改回系統預設
}
