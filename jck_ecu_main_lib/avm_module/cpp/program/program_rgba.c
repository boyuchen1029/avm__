/**
 *******************************************************************************
 * @file : program_rgba.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 2021/11/16 0.1.0 Linda.
 * @date 2021/12/08 0.1.1 Bennett.
 *******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../../system.h"
#include "user_control/ui.h"
#include "avm/GLHeader.h"

#include "avm_ui_init.h"
#include "program/set_framebuffer.h"
#include "set_memory.h"
#include "program/program_rgba.h"
#include "gl/glShaderUtils.h"

#include FRAGSHADER
#include VERTSHADER
#include "display_location.h"
#include "../../autosys/autosys.h"
// GL reverse line define
static GLuint programHandle2dAvm[1] = {0};

GLuint VAO2d[VAO_NUM];
GLuint VAORgba[VAO_NUM];

GLint glAttr2dAvmVertex = 0;
GLint glAttr2dAvmUV = 0;
GLint glAttrRgbaVertex = 0;
GLint glAttrRgbaUV = 0;

GLuint vertexbuffer_2davm[1];  // bowl Vertex
GLuint texturebuffer_2davm[1]; // bowl texture
GLuint vertexbuffer_see[1];    // bowl Vertex
GLuint texturebuffer_see[1];   // bowl texture
GLuint vertexbuffer_rgba[7];
GLuint texturebuffer_rgba[7];

GLint glUni_TextureSampler;

static GLuint g_tex_rgb[5];
static GLuint g_tex_car[5];

static unsigned char *outputRgbaMemSta;
static unsigned char *outputRgbaMemDyn;
static unsigned char *outputRgbaMemCar;

static FBO_t fbo_2davm;

// URX status bar
static avm_ui_t *avmUi;
extern float carSizeWScale, carSizeHScale;

void init_2davm_program(void)
{
    AVM_LOGI("-----------------------------\n");
#if READ_OFFLINE_SHADER
    LoadBinary2Program(&programHandle2dAvm[0], "shader_program.bin_format_36805_3");
    if (programHandle2dAvm[0] == GL_NONE)
    {
        AVM_LOGI(LOG_RED("[ERROR]") "GL Loading 2dAvm offline file error!!!\n");
        AVM_LOGI("[WARNING] Using online program!!!\n");
        programHandle2dAvm[0] = create_program(vertexShader_rgba, fragmentShader_rgba, 3);
    }
#else
    programHandle2dAvm[0] = create_program(vertexShader_rgba, fragmentShader_rgba,3); // 2D AVM
    write_offline_shader_program(programHandle2dAvm[0], 3);
    #endif
    

    AVM_LOGI("AVM program_2dAvm[0]: %d \n", programHandle2dAvm[0]);
    AVM_LOGI("-----------------------------\n");
}

void rgba_mem_malloc(void)
{
#if (PLAFORM == IMAX8 || PLAFORM == SPHE8368_P || PLAFORM == NT98690)
    get_rgba_memory(&outputRgbaMemSta, &outputRgbaMemDyn, &outputRgbaMemCar, &avmUi);
#elif (PLAFORM == MTK2712)
    get_rgba_memory(&outputRgbaMemSta, &outputRgbaMemDyn, &outputRgbaMemCar, &avmUi);
#endif

    // fwrite_rgba(outputRgbaMemCar, 100, 200, "outputRgbaMemCar2.rgba");
}

void open_gl_init_avm_2d(void)
{
    glUseProgram(programHandle2dAvm[0]);

    glAttr2dAvmVertex = glGetAttribLocation(programHandle2dAvm[0], "position");
    glAttr2dAvmUV = glGetAttribLocation(programHandle2dAvm[0], "IntexCoord");

    glGenVertexArrays(VAO_NUM, VAO2d);
        static float vertexCoord2dAvm[3*6] =
        {
            -1.000000,-1.000000,0.100000,
             1.000000,-1.000000,0.100000,
             1.000000, 1.000000,0.100000,
             1.000000, 1.000000,0.100000,
             -1.000000,-1.000000,0.100000,//
             -1.000000, 1.000000,0.100000,
    //        -1.000000,-1.000000,0.200000
        };

        static float fragmentCoord2dAvm[2*6] =
        {
            0.00000, 0.00000,
            1.00000, 0.00000,
            1.00000, 1.00000,
            1.00000, 1.00000,
            0.00000, 0.00000,//
            0.00000, 1.00000,
    //        0.00000, 0.00000,
        };

    glGenBuffers(1, &vertexbuffer_2davm[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_2davm[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoord2dAvm, GL_STATIC_DRAW);

    glGenBuffers(1, &texturebuffer_2davm[0]);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_2davm[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoord2dAvm, GL_STATIC_DRAW);

    glBindVertexArray(VAO2d[0]);
    glEnableVertexAttribArray(glAttr2dAvmVertex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_2davm[0]);
    glVertexAttribPointer(glAttr2dAvmVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(glAttr2dAvmUV);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_2davm[0]);
    glVertexAttribPointer(glAttr2dAvmUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBindVertexArray(0);

    //-------------------------------------------------------------------------------------------------seethrought


    float minX_a = (1.0 - carSizeWScale) / 2.0;
    float maxX_a = 1.0 - (1.0 - carSizeWScale) / 2.0;
    float minY_a = (1.0 - carSizeHScale) / 2.0;
    float maxY_a = 1.0 - (1.0 - carSizeHScale) / 2.0;


    float VertZ = 0.1;

    float minVertX = minX_a * 2 - 1;
    float maxVertX = maxX_a * 2 - 1;
    float minVertY = minY_a * 2 - 1;
    float maxVertY = maxY_a * 2 - 1;

    float minFragX = minX_a;
    float maxFragX = maxX_a;
    float minFragY = minY_a;
    float maxFragY = maxY_a;

    float vertexCoordSee[] =
        {
            minVertX,
            minVertY,
            VertZ,
            maxVertX,
            minVertY,
            VertZ,
            maxVertX,
            maxVertY,
            VertZ,
            maxVertX,
            maxVertY,
            VertZ,
            minVertX,
            maxVertY,
            VertZ,
            minVertX,
            minVertY,
            VertZ,
        };

    float fragmentCoordSee[] =
        {
            minFragX,
            minFragY,
            maxFragX,
            minFragY,
            maxFragX,
            maxFragY,
            maxFragX,
            maxFragY,
            minFragX,
            maxFragY,
            minFragX,
            minFragY,
        };

    glGenBuffers(1, &vertexbuffer_see[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_see[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordSee, GL_STATIC_DRAW);

    glGenBuffers(1, &texturebuffer_see[0]);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_see[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordSee, GL_STATIC_DRAW);

    glBindVertexArray(VAO2d[1]);
    glEnableVertexAttribArray(glAttr2dAvmVertex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_see[0]);
    glVertexAttribPointer(glAttr2dAvmVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(glAttr2dAvmUV);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_see[0]);
    glVertexAttribPointer(glAttr2dAvmUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBindVertexArray(0);
}

void open_gl_init_rgba(void)
{
    glUseProgram(programHandle2dAvm[0]);
    glAttrRgbaVertex = glGetAttribLocation(programHandle2dAvm[0], "position");
    glAttrRgbaUV = glGetAttribLocation(programHandle2dAvm[0], "IntexCoord");

    glGenVertexArrays(VAO_NUM, VAORgba);

    //------------------------------------------------------------------------------------------------outputRgbaMemSta vert

    // glGenBuffers(1, &vertexbuffer_rgba[0]);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[0]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 3 * 5 * sizeof(float), NULL, GL_STATIC_DRAW);

    // glGenBuffers(1, &texturebuffer_rgba[0]);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[0]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 2 * 5 * sizeof(float), NULL, GL_STATIC_DRAW);

    // glBindVertexArray(VAORgba[0]);
    // glEnableVertexAttribArray(glAttrRgbaVertex);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[0]);
    // glVertexAttribPointer(glAttrRgbaVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // glEnableVertexAttribArray(glAttrRgbaUV);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[0]);
    // glVertexAttribPointer(glAttrRgbaUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    // glBindVertexArray(0);

    //------------------------------------------------------------------------------------------------outputRgbaMemDyn vert

    // glGenBuffers(1, &vertexbuffer_rgba[1]);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[1]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 3 * 5 * sizeof(float), NULL, GL_STATIC_DRAW);

    // glGenBuffers(1, &texturebuffer_rgba[1]);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[1]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 2 * 5 * sizeof(float), NULL, GL_STATIC_DRAW);

    // glBindVertexArray(VAORgba[1]);
    // glEnableVertexAttribArray(glAttrRgbaVertex);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[1]);
    // glVertexAttribPointer(glAttrRgbaVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // glEnableVertexAttribArray(glAttrRgbaUV);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[1]);
    // glVertexAttribPointer(glAttrRgbaUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    // glBindVertexArray(0);

    //------------------------------------------------------------------------------------------------2d avm camera dot

    glGenBuffers(1, &vertexbuffer_rgba[2]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[2]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * 10 * sizeof(float), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &texturebuffer_rgba[2]);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[2]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * 10 * sizeof(float), NULL, GL_STATIC_DRAW);

    glBindVertexArray(VAORgba[2]);
    glEnableVertexAttribArray(glAttrRgbaVertex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[2]);
    glVertexAttribPointer(glAttrRgbaVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(glAttrRgbaUV);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[2]);
    glVertexAttribPointer(glAttrRgbaUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------------------can rader

    // glGenBuffers(1, &vertexbuffer_rgba[3]);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[3]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 3 * 10 * sizeof(float), NULL, GL_STATIC_DRAW);

    // glGenBuffers(1, &texturebuffer_rgba[3]);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[3]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 2 * 10 * sizeof(float), NULL, GL_STATIC_DRAW);

    // glBindVertexArray(VAORgba[3]);
    // glEnableVertexAttribArray(glAttrRgbaVertex);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[3]);
    // glVertexAttribPointer(glAttrRgbaVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // glEnableVertexAttribArray(glAttrRgbaUV);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[3]);
    // glVertexAttribPointer(glAttrRgbaUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    // glBindVertexArray(0);

    //------------------------------------------------------------------------------------------------DR App format

    // static float vertexRGBA[3 * 6] =
    //     {
    //         -0.700000, 0.460000, 0.000000,
    //         0.700000, 0.460000, 0.000000,
    //         0.700000, -0.460000, 0.000000,
    //         0.700000, -0.460000, 0.000000,
    //         -0.700000, -0.460000, 0.000000,
    //         -0.700000, 0.460000, 0.000000};

    // static float fragmentRGBA[2 * 6] =
    //     {
    //         0.00000,
    //         0.00000,
    //         1.00000,
    //         0.00000,
    //         1.00000,
    //         1.00000,
    //         1.00000,
    //         1.00000,
    //         0.00000,
    //         1.00000,
    //         0.00000,
    //         0.00000,
    //     };

    // glGenBuffers(1, &vertexbuffer_rgba[4]);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[4]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexRGBA, GL_STATIC_DRAW);

    // glGenBuffers(1, &texturebuffer_rgba[4]);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[4]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentRGBA, GL_STATIC_DRAW);

    // glBindVertexArray(VAORgba[4]);
    // glEnableVertexAttribArray(glAttrRgbaVertex);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[4]);
    // glVertexAttribPointer(glAttrRgbaVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // glEnableVertexAttribArray(glAttrRgbaUV);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[4]);
    // glVertexAttribPointer(glAttrRgbaUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    // glBindVertexArray(0);

    //------------------------------------------------------------------------------------------------outputRgbaMemCar vert

    // glGenBuffers(1, &vertexbuffer_rgba[5]);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[5]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 3 * 1 * sizeof(float), NULL, GL_STATIC_DRAW);

    // glGenBuffers(1, &texturebuffer_rgba[5]);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[5]);
    // glBufferData(GL_ARRAY_BUFFER, 6 * 2 * 1 * sizeof(float), NULL, GL_STATIC_DRAW);

    // glBindVertexArray(VAORgba[5]);
    // glEnableVertexAttribArray(glAttrRgbaVertex);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[5]);
    // glVertexAttribPointer(glAttrRgbaVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // glEnableVertexAttribArray(glAttrRgbaUV);
    // glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[5]);
    // glVertexAttribPointer(glAttrRgbaUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    // glBindVertexArray(0);

    //------------------------------------------------------------------------------------------------see through vert

    glGenBuffers(1, &vertexbuffer_rgba[6]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[6]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * 1 * sizeof(float), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &texturebuffer_rgba[6]);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[6]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * 1 * sizeof(float), NULL, GL_STATIC_DRAW);

    glBindVertexArray(VAORgba[6]);
    glEnableVertexAttribArray(glAttrRgbaVertex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[6]);
    glVertexAttribPointer(glAttrRgbaVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(glAttrRgbaUV);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[6]);
    glVertexAttribPointer(glAttrRgbaUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------------------RGBA texture object

    // glGenTextures(1, &g_tex_rgb[0]);
    // glUni_TextureSampler = glGetUniformLocation(programHandle2dAvm[0], "TextureRgba");
    // glUniform1i(glUni_TextureSampler, RGBA_TEXTURE);
    // glActiveTexture(GL_TEXTURE0 + RGBA_TEXTURE);
    // glBindTexture(GL_TEXTURE_2D, g_tex_rgb[0]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, OUTPUT_RGBA_MEM_W, OUTPUT_RGBA_MEM_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, outputRgbaMemSta);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // glGenTextures(1, &g_tex_rgb[1]);
    // glBindTexture(GL_TEXTURE_2D, g_tex_rgb[1]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, OUTPUT_RGBA_MEM_W, OUTPUT_RGBA_MEM_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, outputRgbaMemDyn);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // glGenTextures(1, &g_tex_rgb[2]);
    // glBindTexture(GL_TEXTURE_2D, g_tex_rgb[2]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, OUTPUT_CAR_MEM_W, OUTPUT_CAR_MEM_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, outputRgbaMemCar);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


}

FBO_t *open_gl_init_2davm_fbo(int texWidth, int texHeight)
{
    create_fbo(&fbo_2davm, texWidth, texHeight, AVM_2D_TEXTURE, GL_LINEAR);
    // create_fbo_MRT(&fbo_2davm, texWidth, texHeight, GL_NEAREST);
    return &fbo_2davm;
}

void draw_2DAVM_texture(void)
{
    glUseProgram(programHandle2dAvm[0]);

    glUniform1f(glGetUniformLocation(programHandle2dAvm[0], "yRervsed"), 1.0);
    float cur_projectionMarix[16];
    memcpy(cur_projectionMarix, autosys_get_zoomin2DCARprojectionmatrix(), sizeof(float)*16);
    cur_projectionMarix[13] *= -1.0f;
    glUniformMatrix4fv(glGetUniformLocation(programHandle2dAvm[0], "Projection"), 1, GL_FALSE, cur_projectionMarix);
    glUniform1i(glGetUniformLocation(programHandle2dAvm[0], "TextureRgba"), AVM_2D_TEXTURE);
    glActiveTexture(GL_TEXTURE0 + AVM_2D_TEXTURE);
    // glBindTexture(GL_TEXTURE_2D, fbo_2davm.textureMRT[0]);  // !!!!!!!!!!MRT!!!!!!!!!!
    glBindTexture(GL_TEXTURE_2D, fbo_2davm.texture); // !!!!!!!!!!MRT!!!!!!!!!!

    glBindVertexArray(VAO2d[0]);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}

void draw_texture(GLuint texture, int vao)
{
    glUseProgram(programHandle2dAvm[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1f(glGetUniformLocation(programHandle2dAvm[0], "yRervsed"), -1.0f);

    glUniform1i(glGetUniformLocation(programHandle2dAvm[0], "TextureRgba"), AVM_2D_TEXTURE);
    glActiveTexture(GL_TEXTURE0 + AVM_2D_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAO2d[vao]);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);

    glBindVertexArray(0);
}

void draw_rgba(ui_draw_info_t avmDraw, int stadyn, int subdata, int texsub)
{
    int whitchRectangle = avmDraw.rectangles - 1;
    // AVM_LOGI("whitchRectangle %d\n",whitchRectangle);
    if (whitchRectangle == 0)
    {
        return;
    }
    glUseProgram(programHandle2dAvm[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1f(glGetUniformLocation(programHandle2dAvm[0], "yRervsed"), yReverse);

    glUniform1i(glGetUniformLocation(programHandle2dAvm[0], "TextureRgba"), RGBA_TEXTURE);
    glActiveTexture(GL_TEXTURE0 + RGBA_TEXTURE);

    if (stadyn == 0)
    {
        glBindTexture(GL_TEXTURE_2D, g_tex_rgb[0]);
        glBindVertexArray(VAORgba[0]);
    }
    else if (stadyn == 1)
    {
        glBindTexture(GL_TEXTURE_2D, g_tex_rgb[1]);
        if (texsub)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, OUTPUT_RGBA_MEM_W, OUTPUT_RGBA_MEM_H, GL_RGBA, GL_UNSIGNED_BYTE, outputRgbaMemDyn);
        }
        glBindVertexArray(VAORgba[1]);
    }
    else if (stadyn == 2)
    {
        glBindTexture(GL_TEXTURE_2D, g_tex_rgb[0]);
        glBindVertexArray(VAORgba[2]);
    }
    else if (stadyn == 3)
    {
        glBindTexture(GL_TEXTURE_2D, g_tex_rgb[0]);
        glBindVertexArray(VAORgba[3]);
    }
    else if (stadyn == 6)
    {
        glBindTexture(GL_TEXTURE_2D, g_tex_rgb[0]);
        glBindVertexArray(VAORgba[6]);
    }
    else if (stadyn == 5)
    {
        glBindTexture(GL_TEXTURE_2D, g_tex_rgb[2]);
        glBindVertexArray(VAORgba[5]);
    }

    if (subdata)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_rgba[stadyn]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 3 * whitchRectangle * sizeof(float), avmDraw.vertex);
        glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_rgba[stadyn]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 2 * whitchRectangle * sizeof(float), avmDraw.texCoods);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6 * whitchRectangle);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

#if (PLAFORM == MTK2712)
void draw_tex(unsigned char *arUi)
{
    glUseProgram(programHandle2dAvm[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1i(glGetUniformLocation(programHandle2dAvm[0], "TextureRgba"), RGBA_TEXTURE);
    glActiveTexture(GL_TEXTURE0 + RGBA_TEXTURE);

    glBindTexture(GL_TEXTURE_2D, g_tex_rgb[3]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FORMAT_COM_W, FORMAT_COM_H, GL_RGBA, GL_UNSIGNED_BYTE, arUi);
    glBindVertexArray(VAORgba[4]);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}
#endif
