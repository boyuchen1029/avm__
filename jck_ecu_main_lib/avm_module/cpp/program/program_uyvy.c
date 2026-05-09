/**
 *******************************************************************************
 * @file : program_uyvy.c
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
#include "user_control/ui.h"
#include "avm/GLHeader.h"

#include "program/set_framebuffer.h"
#include "program/program_uyvy.h"
#include "gl/glShaderUtils.h"

#include FRAGSHADER
#include VERTSHADER

// GL reverse line define
static GLuint programHandleUYVY[1] = {0};

GLuint VAOavm[VAO_NUM];

GLint glAttrAvmVertex = 0;
GLint glAttrAvmUV = 0;

GLuint vertexbuffer_avm[1]; // bowl Vertex
GLuint texturebuffer_avm[1]; // bowl texture

static FBO_t  fbo_all;

void init_avm_program(void)
{
    #if READ_OFFLINE_SHADER
    #else
    programHandleUYVY[0] = create_program(vertexShader_uyvy, fragmentShader_uyvy,8); //2D AVM
    AVM_LOGI("programHandleUYVY[0]: %d \n", programHandleUYVY[0]);
    #endif

}

void open_gl_init_avm(void)
{
    static float vertexCoordAvm[3*6] =
            {
                    -1.000000,-1.000000,0.000000,
                    1.000000,-1.000000,0.000000,
                    1.000000, 1.000000,0.000000,
                    1.000000, 1.000000,0.000000,
                    -1.000000, 1.000000,0.000000,
                    -1.000000,-1.000000,0.000000
            };

    static float fragmentCoordAvm[2*6] =
            {
                    0.00000, 1.00000,
                    1.00000, 1.00000,
                    1.00000, 0.00000,
                    1.00000, 0.00000,
                    0.00000, 0.00000,
                    0.00000, 1.00000,
            };

    glUseProgram(programHandleUYVY[0]);

    glAttrAvmVertex = glGetAttribLocation(programHandleUYVY[0], "position");
    glAttrAvmUV = glGetAttribLocation(programHandleUYVY[0], "IntexCoord");

    glGenVertexArrays(VAO_NUM, VAOavm);

    glGenBuffers(1, &vertexbuffer_avm[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_avm[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordAvm, GL_STATIC_DRAW);

    glGenBuffers(1, &texturebuffer_avm[0]);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_avm[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordAvm, GL_STATIC_DRAW);

    glBindVertexArray(VAOavm[0]);
    glEnableVertexAttribArray(glAttrAvmVertex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_avm[0]);
    glVertexAttribPointer(
            glAttrAvmVertex,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void *)0           // array buffer offset
    );

    glEnableVertexAttribArray(glAttrAvmUV);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_avm[0]);
    glVertexAttribPointer(glAttrAvmUV, 2,  GL_FLOAT, GL_FALSE, 0, (void *)0 );
    glBindVertexArray(0);
}

int open_gl_init_avm_fbo(void)
{
    create_fbo(&fbo_all,  SCREEN_W, SCREEN_H, AVM_FBO, GL_LINEAR);
    //create_fbo(&fbo_all,  SCREEN_W, SCREEN_H, AVM_FBO, GL_NEAREST);
    return fbo_all.framebuffer;
}

void draw_avm(const GLuint FramebufDefault)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufDefault);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, SCREEN_W, SCREEN_H);

    glUseProgram(programHandleUYVY[0]);

    glUniform1i(glGetUniformLocation(programHandleUYVY[0], "TextureRgba"), AVM_FBO);
    glActiveTexture(GL_TEXTURE0 + AVM_FBO);
    glBindTexture(GL_TEXTURE_2D, fbo_all.texture);

    glBindVertexArray(VAOavm[0]);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}
