
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

#include "../../system.h"
#include "avm/maths.h"
#include "user_control/ui.h"
#include "avm/GLHeader.h"
#include FRAGSHADER
#include VERTSHADER
#include "canbus.h"
#include "avmDefine.h"
#include "avm/fp_source.h"
#include "gl/glShaderUtils.h"
#include "program/program_blur.h"
#include "display_location.h"
#include "../../autosys/autosys.h"
#include "../include/program/program_main.h"

program_context *blur;


void init_blur_program(void)
{
    blur = &autosys.program_context[BLUR];
    blur->programID          = create_program(vertexShader_blur, fragmentShader_blur, 22);
    blur->attrVertexLocation = glGetAttribLocation(blur->programID, "InPosition");
    blur->attrUVLocation     = glGetAttribLocation(blur->programID, "InTexCoord");
    AVM_LOGI("-----------------------------\n");
    AVM_LOGI("AVM program_Blur[0]: %d \n", blur->programID);
    AVM_LOGI("-----------------------------\n");
}

void open_gl_init_blur()
{
    float v_coord[] = 
    {
        -1.000000f, -1.000000f, 0.100000f,
         1.000000f, -1.000000f, 0.100000f,
         1.000000f,  1.000000f, 0.100000f,
         1.000000f,  1.000000f, 0.100000f,
        -1.000000f,  1.000000f, 0.100000f,
        -1.000000f, -1.000000f, 0.100000f,
    };

    static float fragmentCoordView2d[2 * 6] =
    {
        0.00000,
        1.00000,
        1.00000,
        1.00000,
        1.00000,
        0.00000,
        1.00000,
        0.00000,
        0.00000,
        0.00000,
        0.00000,
        1.00000,
    };

    // float t_coord[] = 
    // {
    //      0.0000f, 0.0000f, // 左上角
    //      1.0000f, 0.0000f, // 右上角
    //      1.0000f, 1.0000f, // 右下角
    //      1.0000f, 1.0000f, // 右下角
    //      0.0000f, 1.0000f, // 左下角
    //      0.0000f, 0.0000f  // 左上角
    // };

    float t_coord[] = 
    {
         0.0000f, 1.0000f, // 左上角
         1.0000f, 1.0000f, // 右上角
         1.0000f, 0.0000f, // 右下角
         1.0000f, 0.0000f, // 右下角
         0.0000f, 0.0000f, // 左下角
         0.0000f, 1.0000f  // 左上角
    };

    glUseProgram(blur->programID);
    glGenVertexArrays(1, blur->vao);

    glGenBuffers(2, &blur->vbo[0][0]);
    glBindBuffer(GL_ARRAY_BUFFER, blur->vbo[0][0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), v_coord, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, blur->vbo[0][1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), t_coord, GL_STATIC_DRAW);

    glBindVertexArray(blur->vao[0]);
    glEnableVertexAttribArray(blur->attrVertexLocation);
    glBindBuffer(GL_ARRAY_BUFFER, blur->vbo[0][0]);
    glVertexAttribPointer(blur->attrVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray(blur->attrUVLocation);
    glBindBuffer(GL_ARRAY_BUFFER, blur->vbo[0][1]);
    glVertexAttribPointer(blur->attrUVLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBindVertexArray(0);

}

void generate_blur_fisheye_texture()
{
    #if PARAM_FISHEYE_BLUR_FUNCTION 
    const float GaussianWeight3[2] = { 0.5f, 0.25f  };
    const float GaussianWeight5[3] = { 0.4026f, 0.2442f, 0.0545f };
    const float GaussianWeight9[5] = { 0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f };



	float transfermatrix[16] =
	{
		1.0f,  0.0f, 0.0f,	0.0f,
		0.0f,  1.0f, 0.0f,	0.0f,
		0.0f,  0.0f, 1.0f,	0.0f,
		0.0f,  0.0f, 0.0f,	1.0f,
	};

    PARA_BLUR *blur_info = &autosys.system_info.blur_info;
    int imW = autosys.system_info.IMGSize.X * PARAM_FISHEYE_BLUR_LOSS_PANEL_W_VOLUME;
	int imH = autosys.system_info.IMGSize.Y * PARAM_FISHEYE_BLUR_LOSS_PANEL_H_VOLUME; 
    int program = blur->programID;
    float imgStrep[] = {1.0f / (float)imW, 1.0f / (float)imH}; 
    float LargeMask[4] = {0};
    for(int cam = 0; cam < MAX_CAM_NUM; cam++)
    {
        if(blur_info->minimap_enable == 0)
        {
            blur->textureID[FISHEYE_FRONT_BLUR_TEXTURE + cam] = get_fisheye_raw_textrueID(cam);
            continue;
        }
        float* Mask = &blur_info->blur_mask[cam][0];
        memcpy(LargeMask, Mask, sizeof(float) * 4);
        LargeMask[0] *= 0.8f;
        LargeMask[1] *= 0.8f;
        LargeMask[2] *= 1.2f;
        LargeMask[3] *= 1.2f;
        float* GaussianWeight  = NULL;
        int GaussianWeightSize = 0;
        int *GaussianEnable   = &blur_info->blur_enable;
        switch(blur_info->blur_level[cam])
        {
            case 0:
            GaussianWeight = (float*)GaussianWeight3;
            GaussianWeightSize = 2;
            break;
            case 1:
            GaussianWeight = (float*)GaussianWeight5;
            GaussianWeightSize = 3;
            break;
            case 2:
            GaussianWeight = (float*)GaussianWeight9;
            GaussianWeightSize = 5;
            break;
            default:
            break;
        }
        FBO_t *fbo_blur1 = &autosys.frame_fbo[FBO_BLUR_1 + cam];
        FBO_t *fbo_blur2 = &autosys.frame_fbo[FBO_BLUR_TMP];
        if(GaussianEnable[cam] == 0 || PARAM_FISHEYE_BLUR_GAUSSIAN_FUNCTION == 0)
        {
            //swap
            FBO_t *tmp = fbo_blur1;
            fbo_blur1 = fbo_blur2;
            fbo_blur2 = tmp;
            GaussianWeightSize = 0;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur2->framebuffer);
        {
            glUseProgram(program);
            glViewport(0, 0, imW, imH);
            glUniform1i(glGetUniformLocation(program, "horizontal"), 1);
            glUniform1i(glGetUniformLocation(program, "GAUSSIAN_SAMPLES"), GaussianWeightSize);
            glUniform1fv(glGetUniformLocation(program, "gaussian_weight"), GaussianWeightSize, GaussianWeight);
            glUniform2fv(glGetUniformLocation(program, "tex_offset"), 1, imgStrep);
            glUniform4fv(glGetUniformLocation(program, "MaskArea"), 1, LargeMask);
            glUniform1i(glGetUniformLocation(program, "Texture"), FRONT_CAMERA + cam);
	        glUniformMatrix4fv(glGetUniformLocation(program, "transformatiomMatrix"), 1, GL_FALSE, &transfermatrix[0]); 
            glActiveTexture(GL_TEXTURE0 + FRONT_CAMERA + cam);
            glBindTexture(GL_TEXTURE_2D, get_fisheye_raw_textrueID(cam));

            glBindVertexArray(blur->vao[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }

        if(GaussianEnable[cam] == 0 || PARAM_FISHEYE_BLUR_GAUSSIAN_FUNCTION == 0)
        {
            //swap
            FBO_t *tmp = fbo_blur1;
            fbo_blur1 = fbo_blur2;
            fbo_blur2 = tmp;
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur1->framebuffer);
            glUseProgram(program);
            glViewport(0, 0, imW, imH);
            glUniform1i(glGetUniformLocation(program, "horizontal"), 0);
            glUniform1i(glGetUniformLocation(program, "GAUSSIAN_SAMPLES"), GaussianWeightSize);
            glUniform1fv(glGetUniformLocation(program, "gaussian_weight"), GaussianWeightSize, GaussianWeight);
            glUniform2fv(glGetUniformLocation(program, "tex_offset"), 1, imgStrep);
            glUniformMatrix4fv(glGetUniformLocation(program, "transformatiomMatrix"), 1, GL_FALSE, &transfermatrix[0]); 
            glUniform4fv(glGetUniformLocation(program, "MaskArea"), 1, Mask);
            glUniform1i(glGetUniformLocation(program, "Texture"), 0);
            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, fbo_blur2->texture);

            glBindVertexArray(blur->vao[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }

        glBindTexture(GL_TEXTURE_2D, fbo_blur1->texture);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        blur->textureID[FISHEYE_FRONT_BLUR_TEXTURE + cam] = fbo_blur1->texture;
    }
    #endif
}

