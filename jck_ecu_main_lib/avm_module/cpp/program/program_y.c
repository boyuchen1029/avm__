/**
 *******************************************************************************
 * @file : program_y.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20220418 0.1.1 Linda.
 *******************************************************************************
*/

#include <stdio.h>
#include <string.h>

#include <math.h>

#include "../../system.h"
#include "user_control/ui.h"
#include "avm/GLHeader.h"

#include "program/set_framebuffer.h"
#include "program/program_y.h"
#include "gl/glShaderUtils.h"

#include FRAGSHADER
#include VERTSHADER



// GL reverse line define
static GLuint programHandleY[1] = {0};

static GLuint VAOY[VAO_NUM];

GLint glAttrYVertex = 0;
GLint glAttrYUV = 0;

GLuint vertexbuffer_y[1]; // bowl Vertex
GLuint texturebuffer_y[1]; // bowl texture

// static FBO_t * fbo_all;


void init_rgba2y_program(void)
{
    AVM_LOGI("-----------------------------\n");
    #if READ_OFFLINE_SHADER
    LoadBinary2Program(&programHandleY[0],"shader_program.bin_format_36805_9");
    if(programHandleY[0] == GL_NONE)
	{
		AVM_LOGI(LOG_RED("[ERROR]")"GL Loading programHandleY offline file error!!!\n");
		AVM_LOGI("[WARNING] Using online program!!!\n");
		programHandleY[0]   = create_program(vertexShader_y, fragmentShader_y, 9); 
	}
    #else
    programHandleY[0] = create_program(vertexShader_y, fragmentShader_y,9); //2D AVM
    write_offline_shader_program(programHandleY[0], 9);
    #endif
    

    AVM_LOGI("AVM program_Y[0]: %d \n", programHandleY[0]);
    AVM_LOGI("-----------------------------\n");
}

void open_gl_init_rgba2y(void)
{
    static float vertexCoordAvm[3*6] =
    {
        -1.000000,-1.000000, 0.100000,
         1.000000,-1.000000, 0.100000,
         1.000000, 1.000000, 0.100000,
         1.000000, 1.000000, 0.100000,
        -1.000000, 1.000000, 0.100000,
        -1.000000,-1.000000, 0.100000
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

    glUseProgram(programHandleY[0]);

    glAttrYVertex = glGetAttribLocation(programHandleY[0], "vPosition");
    glAttrYUV = glGetAttribLocation(programHandleY[0], "a_TexCoord");

    glGenVertexArrays(VAO_NUM, VAOY);

    glGenBuffers(1, &vertexbuffer_y[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_y[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexCoordAvm, GL_STATIC_DRAW);

    glGenBuffers(1, &texturebuffer_y[0]);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_y[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), fragmentCoordAvm, GL_STATIC_DRAW);

    glBindVertexArray(VAOY[0]);
    glEnableVertexAttribArray(glAttrYVertex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_y[0]);
    glVertexAttribPointer(
            glAttrYVertex,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void *)0           // array buffer offset
    );

    glEnableVertexAttribArray(glAttrYUV);
    glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_y[0]);
    glVertexAttribPointer(
            glAttrYUV,                  // attribute
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void *)0           // array buffer offset
    );
    glBindVertexArray(0);
}

void draw_rgba2y(GLuint texture,int avm_2d_width)
{
    glUseProgram(programHandleY[0]);

    glUniform1i(glGetUniformLocation(programHandleY[0], "u_video"), AVM_2D_TEXTURE);
    glUniform1f(glGetUniformLocation(programHandleY[0], "yRervsed"), yReverse);
    #if FPS_OPTIMISE_V0
    glUniform1f(glGetUniformLocation(programHandleY[0], "windows"), 1.0f/(float)avm_2d_width);
    #else
    glUniform1f(glGetUniformLocation(programHandleY[0], "windows"), (float)avm_2d_width);
    #endif
    glActiveTexture(GL_TEXTURE0 + AVM_2D_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAOY[0]);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}
