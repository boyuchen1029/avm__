#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../../system.h"
#include "user_control/ui.h"
#include "avm/GLHeader.h"

#include "avm_ui_init.h"
#include "program/set_framebuffer.h"
#include "set_memory.h"
#include "program/program_TSAA.h"
#include "gl/glShaderUtils.h"

#include FRAGSHADER
#include VERTSHADER
#include "display_location.h"

//static GLuint programHandleFXAA[1] = {0};
static GLuint FXAA_RAW_TEXTURE = 0;
static int tran_ones = 0;
static int FXAA_frame = 0;
static int FXAA_READY_TO_DRAW_STATUS = 0;

typedef struct
{
    int program;
    int texture;
    int glAttrVertex;
    int glAttrUV;
    int vao[10];
    int vbo[10][5];
}GLContext;

GLContext fxaa;

void init_FXAA_program(void)
{
    AVM_LOGI("-----------------------------\n");
    #if READ_OFFLINE_SHADER
    LoadBinary2Program(&fxaa.program, "shader_program.bin_format_36805_7");
    if (fxaa.program == GL_NONE)
    {
        AVM_LOGI(LOG_RED("[ERROR]") "GL Loading FXAA offline file error!!!\n");
        AVM_LOGI("[WARNING] Using online program!!!\n");
        fxaa.program = create_program(vertexShader_FXAA, fragmentShader_FXAA, 7);
    }
    #else
    fxaa.program = create_program(vertexShader_FXAA, fragmentShader_FXAA,7); // 2D AVM
    write_offline_shader_program(fxaa.program, 7);
    #endif


    AVM_LOGI("AVM program_FXAA[0]: %d \n", fxaa.program);
    fxaa.glAttrVertex = glGetAttribLocation(fxaa.program, "vPosition");
    fxaa.glAttrUV     = glGetAttribLocation(fxaa.program, "InTexCoord");
    AVM_LOGI("-----------------------------\n");

}

void trans_FXAA_texture(GLuint raw_texture)
{
    FXAA_RAW_TEXTURE = raw_texture;
}

int get_FXAA_ready_to_draw()
{
    return FXAA_READY_TO_DRAW_STATUS;
}


int check_FXAA_frame()
{
    FXAA_frame++;
    #if FXAA_LOSS_PARAM_FLAG
    if(FXAA_frame > ( FXAA_LOSS_FRAME))
    {
        FXAA_frame = 0;
        FXAA_READY_TO_DRAW_STATUS = 0;
        return 0;
    }
    else { FXAA_READY_TO_DRAW_STATUS = 1; return 1;}
    #else
    FXAA_READY_TO_DRAW_STATUS = 1; return 1;
    #endif
}


void init_FXAA_opengl()
{
    float v_coord[] = 
    {
        -1.000000f,-1.000000f, 0.100000f,
         1.000000f,-1.000000f, 0.100000f,
         1.000000f, 1.000000f, 0.100000f,
         1.000000f, 1.000000f, 0.100000f,
        -1.000000f, 1.000000f, 0.100000f,
        -1.000000f, -1.000000f, 0.100000f,
    };

    float t_coord[] = 
    {
         0.0000f, 0.0000f, // 左上角
         1.0000f, 0.0000f, // 右上角
         1.0000f, 1.0000f, // 右下角
         1.0000f, 1.0000f, // 右下角
         0.0000f, 1.0000f, // 左下角
         0.0000f, 0.0000f  // 左上角
    };

    glUseProgram(fxaa.program);
    glGenVertexArrays(1, fxaa.vao);

    glGenBuffers(2, fxaa.vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, fxaa.vbo[0][0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), v_coord, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, fxaa.vbo[0][1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), t_coord, GL_DYNAMIC_DRAW);

    glBindVertexArray(fxaa.vao[0]);
    glEnableVertexAttribArray(fxaa.glAttrVertex);
    glBindBuffer(GL_ARRAY_BUFFER, fxaa.vbo[0][0]);
    glVertexAttribPointer(fxaa.glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(fxaa.glAttrUV);
    glBindBuffer(GL_ARRAY_BUFFER, fxaa.vbo[0][1]);
    glVertexAttribPointer(fxaa.glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBindVertexArray(0);

}

void draw_FXAA(float *glViewport, bool isEnableFxaa)
{
    //if(glViewport == NULL) return;
    float panel_width  = (float)g_PANEL_WIDTH  * PARAM_FXAA_LOSS_PANEL_VOLUME;
    float panel_height = (float)g_PANEL_HEIGHT * PARAM_FXAA_LOSS_PANEL_VOLUME;

    float top  = (float)glViewport[0] * PARAM_FXAA_LOSS_PANEL_VOLUME; //0
    float left = (float)glViewport[1] * PARAM_FXAA_LOSS_PANEL_VOLUME; //422
    float w    = (float)glViewport[2] * PARAM_FXAA_LOSS_PANEL_VOLUME; //858
    float h    = (float)glViewport[3] * PARAM_FXAA_LOSS_PANEL_VOLUME; //720

    // float ndc_left = (2.0f * (left / panel_width)) - 1.0f;
    // float ndc_right = (2.0f * ((w) / panel_width)) - 1.0f;
    // float ndc_top = 1.0f - (2.0f * (top / panel_height));
    // float ndc_bottom = 1.0f - (2.0f * ((h) / panel_height));

    float tex_left   =   left / panel_width;
    float tex_right  =  (left + w) / panel_width;
    float tex_top    =  top / panel_height;
    float tex_bottom =  (h + top) / panel_height;

    //float tex_top = 1.0f;
    //float tex_bottom = 0.0f;

    // float v_coord[] = {
    //     ndc_left, ndc_bottom, 0.1f,
    //     ndc_right, ndc_bottom, 0.1f,
    //     ndc_right, ndc_top, 0.1f,
    //     ndc_right, ndc_top, 0.1f,
    //     ndc_left, ndc_top, 0.1f,
    //     ndc_left, ndc_bottom, 0.1f,
    // };  

    // float t_coord[] = {
    //     tex_left, tex_bottom,
    //     tex_right, tex_bottom,
    //     tex_right, tex_top,
    //     tex_right, tex_top,
    //     tex_left, tex_top,
    //     tex_left, tex_bottom,
    // };
    float t_coord[] = 
    {
    tex_left, tex_top,        // 将 tex_bottom 替换为 tex_top
    tex_right, tex_top,       // 将 tex_bottom 替换为 tex_top
    tex_right, tex_bottom,    // 将 tex_top 替换为 tex_bottom
    tex_right, tex_bottom,    // 将 tex_top 替换为 tex_bottom
    tex_left, tex_bottom,     // 将 tex_top 替换为 tex_bottom
    tex_left, tex_top,        // 将 tex_bottom 替换为 tex_top
    };

    glBindVertexArray(fxaa.vao[0]);
    // glBindBuffer(GL_ARRAY_BUFFER, fxaa.vbo[0][0]);
    // glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 3 * sizeof(float), v_coord);
    glBindBuffer(GL_ARRAY_BUFFER, fxaa.vbo[0][1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 2 * sizeof(float), t_coord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    glUseProgram(fxaa.program);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glUniformMatrix4(GL.GetUniformLocation(fxaa.program, "u_transformationMatrix"), 1, false, seeMatrix);

    if(tran_ones == 0)
    {
        float seeMatrix[16] = 
        {
            1.0f, 0.0f,0.0f,0.0f,
            0.0f, 1.0f,0.0f,0.0f,
            0.0f, 0.0f,1.0f,0.0f,
            0.0f, 0.0f,0.0f,1.0f,
        };
        float direction[2] = {0.0f,1.0f};

        glUniform1i(glGetUniformLocation(fxaa.program, "EnableFxaa"), isEnableFxaa);
        glUniform1f(glGetUniformLocation(fxaa.program, "edgeThreshold"), 0.3f);
        glUniform1i(glGetUniformLocation(fxaa.program, "process"), 1);
        glUniform1f(glGetUniformLocation(fxaa.program, "screenwidth"),  (float)g_PANEL_WIDTH);
        glUniform1f(glGetUniformLocation(fxaa.program, "screenheight"), (float)g_PANEL_HEIGHT);
        glUniform2fv(glGetUniformLocation(fxaa.program, "dir"),  1, direction);


        glActiveTexture(GL_TEXTURE0 + FXAA_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, FXAA_RAW_TEXTURE);
        glUniform1i(glGetUniformLocation(fxaa.program, "RAWImage"), FXAA_TEXTURE);
        tran_ones = 0;
    }

    glBindVertexArray(fxaa.vao[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}
