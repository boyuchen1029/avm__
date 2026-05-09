/**
 *******************************************************************************
 * @file : program_seeThrough.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20220106 0.1.0 Linda.
 *******************************************************************************
*/

#include <stdio.h>
#include <string.h>

#include <math.h>

#include "../../system.h"
#include "user_control/ui.h"
#include "avm/GLHeader.h"
#include "avm/maths.h"
#include <pthread.h>
#include "canbus.h"
#include "display_location.h"

#include "avm/camera_matrix.h"
#include "avm/fp_source.h"
#include "program/set_framebuffer.h"
#include "program/program_rgba.h"
#include "gl/glShaderUtils.h"
#include "program/program_seeThrough.h"
#include "../../autosys/autosys.h"
#include <unistd.h>//Atlas20250926

#include FRAGSHADER
#include VERTSHADER

static FBO_t fbo_seethrough;
static FBO_t fbo3;

// GL reverse line define
static GLuint progSeeThrough[1] = {0};
static GLuint progSeeThrough_2davm[1] = {0};

static GLuint VAOsee[VAO_NUM];
//static GLuint textureObj_seeThrought[2];

static GLuint vertexbuffer_seethrough[2];
static GLuint texturebuffer_seethroughthrough[2];

static GLint glAttrVertex = 0;
static GLint glAttrUV = 0;

static float u_seeThroughArea[6];

static double normv=130.0;

static int reflashSee = 1;


static pthread_t threadSeethrough;
GLuint seethroughbufferPBO[VAO_NUM];
unsigned char * bufferseethrough;
static int seethroughPBO_w;
static int seethroughPBO_h;
static int indexseethrough    = 0;
static int nextIndexseethrough= 0;

void init_seethrough_program(void)
{
#if (PARAM_AVM_SEETHROUGH == 1)
    AVM_LOGI("-----------------------------\n");
    #if READ_OFFLINE_SHADER
    LoadBinary2Program(&progSeeThrough[0],"shader_program.bin_format_36805_10");
    LoadBinary2Program(&progSeeThrough_2davm[0],"shader_program.bin_format_36805_11");
    if (progSeeThrough[0] == GL_NONE || progSeeThrough_2davm[0] == GL_NONE)
    {
        AVM_LOGI(LOG_RED("[ERROR]") "GL Loading progSeeThrough offline file error!!!\n");
        AVM_LOGI("[WARNING] Using online program!!!\n");
        progSeeThrough[0]       = create_program(vertexShader_see_through, fragmentShader_see_through, 0);
        progSeeThrough_2davm[0] = create_program(vertexShader_see_through_2davm, fragmentShader_see_through_2davm, 11);
    }
    #else
    progSeeThrough[0] = create_program(vertexShader_see_through, fragmentShader_see_through,10);
    progSeeThrough_2davm[0] = create_program(vertexShader_see_through_2davm, fragmentShader_see_through_2davm,11);
    write_offline_shader_program(progSeeThrough[0], 10);
    write_offline_shader_program(progSeeThrough_2davm[0], 11);
    #endif
    AVM_LOGI("AVM program_SeeThrough[0]: %d \n", progSeeThrough[0]);
    AVM_LOGI("AVM program_SeeThrough_2davm[0]: %d \n", progSeeThrough_2davm[0]);
    AVM_LOGI("-----------------------------\n");
#endif

}

void open_gl_init_seethrough(double horiFov, double vertiFov)
{
    #if (PARAM_AVM_SEETHROUGH == 1)
        float v_rightUp[2];     // 1
        float v_leftUp[2];      // 2
        float v_leftDown[2];    // 3
        float v_rightDown[2];   // 4
        v_rightUp[0] = (float)(horiFov / 2.0);
        v_rightUp[1] = (float)(-vertiFov / 2.0);
        v_leftUp[0] = (float)(-horiFov / 2.0);
        v_leftUp[1] = (float)(-vertiFov / 2.0);
        v_leftDown[0] = (float)(-horiFov / 2.0);
        v_leftDown[1] = (float)(vertiFov / 2.0);
        v_rightDown[0] = (float)(horiFov / 2.0);
        v_rightDown[1] = (float)(vertiFov / 2.0);


        float v_coord[6*3]=
                {
                        /* v_rightUp   v_leftUp   v_leftDown */
                        /* v_rightUp   v_leftDown  v_rightDown */

                        v_rightUp[0],   1.00000, v_rightUp[1],
                        v_leftUp[0],    1.00000, v_leftUp[1],
                        v_leftDown[0],  1.00000, v_leftDown[1],
                        v_rightUp[0],   1.00000, v_rightUp[1],
                        v_leftDown[0],  1.00000, v_leftDown[1],
                        v_rightDown[0], 1.00000, v_rightDown[1],
                };
        AVM_LOGI("seethrough vao v_coord\n");
        int ix;
        // for(ix=0;ix<18;ix+=3)
        // {
        //     AVM_LOGI("%f %f %f\n",v_coord[ix],v_coord[ix+1],v_coord[ix+2]);
        // }

        float t_coord[6*2]=
                {
                        1.0000, 1.0000,
                        0.0000, 1.0000,
                        0.0000, 0.0000,
                        1.0000, 1.0000,
                        0.0000, 0.0000,
                        1.0000, 0.0000
                };
        float t_coord1[6*2]=
                {
                        1.0000, 0.0000,
                        0.0000, 0.0000,
                        0.0000, 1.0000,
                        1.0000, 0.0000,
                        0.0000, 1.0000,
                        1.0000, 1.0000
                };

        glUseProgram(progSeeThrough[0]);
        glGenVertexArrays(VAO_NUM, VAOsee);         // 產生VAO物件

        glAttrVertex = glGetAttribLocation(progSeeThrough[0], "a_Position");
        glAttrUV = glGetAttribLocation(progSeeThrough[0], "a_texCoord");

        glGenBuffers(1, &vertexbuffer_seethrough[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_seethrough[0]);
        glBufferData(GL_ARRAY_BUFFER, 6*3*sizeof(float), v_coord, GL_STATIC_DRAW);

        glGenBuffers(1, &texturebuffer_seethroughthrough[0]);
        glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_seethroughthrough[0]);
        glBufferData(GL_ARRAY_BUFFER, 6*2*sizeof(float), t_coord, GL_STATIC_DRAW);

        glGenBuffers(1, &vertexbuffer_seethrough[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_seethrough[1]);
        glBufferData(GL_ARRAY_BUFFER, 6*3*sizeof(float), v_coord, GL_STATIC_DRAW);

        glGenBuffers(1, &texturebuffer_seethroughthrough[1]);
        glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_seethroughthrough[1]);
        glBufferData(GL_ARRAY_BUFFER, 6*2*sizeof(float), t_coord1, GL_STATIC_DRAW);

        glBindVertexArray(VAOsee[0]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_seethrough[0]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_seethroughthrough[0]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glBindVertexArray(0);

        glBindVertexArray(VAOsee[1]);
            glEnableVertexAttribArray(glAttrVertex);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_seethrough[1]);
            glVertexAttribPointer(glAttrVertex, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            glEnableVertexAttribArray(glAttrUV);
            glBindBuffer(GL_ARRAY_BUFFER, texturebuffer_seethroughthrough[1]);
            glVertexAttribPointer(glAttrUV, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glBindVertexArray(0);


        AVM_LOGI("Create seethrough FBO\n");
        create_fbo(&fbo3, AVM_2D_FBO_W, AVM_2D_FBO_H, SEE_THROUGH, GL_LINEAR);
        create_fbo(&fbo_seethrough, AVM_2D_FBO_W, AVM_2D_FBO_H, SEE_THROUGH_RESULT, GL_LINEAR);
        autosys.program_context[SEETHROUGH].textureID[FBO3_TEXTURE] = fbo3.texture;
        autosys.program_context[SEETHROUGH].textureID[FBO_SEETHROUGH_TEXTURE] = fbo_seethrough.texture;
        

        /*    u_seeThroughArea[0] = (float)-160.0 / normv;
    u_seeThroughArea[1] = (float)1.0;
    u_seeThroughArea[2] = (float)-330.0 / normv;
    u_seeThroughArea[3] = (float)160.0 / normv;
    u_seeThroughArea[4] = (float)1.0;
    u_seeThroughArea[5] = (float)330.0 / normv;
    */
        // u_seeThroughArea[0] = (float)-160.0/normv;
        // u_seeThroughArea[1] = (float)1.0;
        // u_seeThroughArea[2] = (float)-280.0/normv;
        // u_seeThroughArea[3] = (float)160.0/normv;
        // u_seeThroughArea[4] = (float)1.0;
        // u_seeThroughArea[5] = (float)280.0/normv;

        float* vertexCoord = autosys.program_context[SEETHROUGH].vertexCoord[0];
        u_seeThroughArea[0] = (float)vertexCoord[0] - 0.5f;
        u_seeThroughArea[1] = (float)1.0;
        u_seeThroughArea[2] = (float)vertexCoord[2] - 0.5f;
        u_seeThroughArea[3] = (float)vertexCoord[15] + 0.5f;
        u_seeThroughArea[4] = (float)1.0;
        u_seeThroughArea[5] = (float)vertexCoord[17] + 0.5f;

        // u_seeThroughArea[0] = (float)-160.0/normv;
        // u_seeThroughArea[1] = (float)1.0;
        // u_seeThroughArea[2] = (float)-330.0/normv;
        // u_seeThroughArea[3] = (float)160.0/normv;
        // u_seeThroughArea[4] = (float)1.0;
        // u_seeThroughArea[5] = (float)330.0/normv;

		glUniform1f(glGetUniformLocation(progSeeThrough[0], "yRervsed"), yReverse);
        
    #endif
}

static void draw_proSee(float *seeArea, float *seeMatrix, float *seeMatrix2, GLuint texture)
{
    glUseProgram(progSeeThrough[0]);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ZERO);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    autosys_system_info *system = &autosys.system_info;
    glUniform1f(glGetUniformLocation(progSeeThrough[0], "sigmoid_alpha"), system->seethrough_sigmoid_alpha);
    glUniform1f(glGetUniformLocation(progSeeThrough[0], "top2d_alpha_center"), system->seethrough_top2d_alpha_center);
    glUniform1f(glGetUniformLocation(progSeeThrough[0], "top2d_alpha_range"), system->seethrough_top2d_alpha_range);
    // glUniform1f(glGetUniformLocation(progSeeThrough[0], "sigmoid_alpha"), 5.0f);
    // glUniform1f(glGetUniformLocation(progSeeThrough[0], "top2d_alpha_center"), 70.0f/130.0f);
    // glUniform1f(glGetUniformLocation(progSeeThrough[0], "top2d_alpha_range"), 40.0f/130.0f);
    glUniform3fv(glGetUniformLocation(progSeeThrough[0], "u_seeThroughArea"), 6, &seeArea[0]);				// 底盤範圍
    glUniformMatrix4fv(glGetUniformLocation(progSeeThrough[0], "u_transformationMatrix"), 1, GL_FALSE, seeMatrix);
    glUniformMatrix4fv(glGetUniformLocation(progSeeThrough[0], "u_transformationMatrix2"), 1, GL_FALSE, seeMatrix2);	// 底盤範圍轉換到正規化座標
    {
        glActiveTexture(GL_TEXTURE0 + SEE_THROUGH_RESULT);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(progSeeThrough[0], "u_2davm"), SEE_THROUGH_RESULT);
    }


    glBindVertexArray(VAOsee[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
}

static void draw_proSee_2davm(float *seeMatrix, GLuint texture, int vao)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(progSeeThrough_2davm[0]);
    // glEnable(GL_BLEND);

    //glUniformMatrix4fv(glGetUniformLocation(progSeeThrough_2davm[0], "u_transformationMatrix2"), 1, GL_FALSE, seeMatrix);
    glUniformMatrix4fv(glGetUniformLocation(progSeeThrough_2davm[0], "u_transformationMatrix"), 1, GL_FALSE, seeMatrix);

    glActiveTexture(GL_TEXTURE0 + SEE_THROUGH);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(progSeeThrough_2davm[0], "u_seeThroughImage"), SEE_THROUGH);


    glBindVertexArray(VAOsee[vao]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
static float pre_distance = 0;
void seeThrough_rotate_callculate(camera_para_t * para, can_bus_info_t *canbus)
{
    #if (PARAM_AVM_SEETHROUGH == 1)
        //printf("steer %f meterage %f\n",canbus->steer, canbus->meterage);
        static double now_distance = 0.0;
        static double travel_distance = 0.0;
        static double before_distance = 0.0;
        static double phi_tire = 0.0;
        static double phi_tire_mid = 0.0;
        static double Sf = 0.0;
        static double Rf = 0.0;
        static double Rr = 0.0;
        static double Rc = 0.0;
        static double alpha = 0.0;
        static double Tc = 0.0;
        static double degree_deg = 0.0;
        static double __degree__ = 0.0;
        static double yaw = DEG_TO_RAD(-90);
        
        static float P_now_center[2] = {0.0, 0.0};
        static float sum_distance = 0.0;

        now_distance = canbus->meterage;
        //printf("distance %f speed %f\n", now_distance, canbus->speed);
        travel_distance = now_distance - before_distance;  
        // travel_distance = 0.1; 
        before_distance = now_distance;
        sum_distance = sum_distance + travel_distance;
        // travel_distance = 0.1; 

        phi_tire = canbus->steer;	
        // printf("phi_tire = %f\n",phi_tire);
        // phi_tire = 20.0;
        Sf = travel_distance*100.0;
        Rf = CAR_WHEELBASE/sin(DEG_TO_RAD(phi_tire));
        Rr = CAR_WHEELBASE/tan(DEG_TO_RAD(phi_tire));
        phi_tire_mid = atan(CAR_WHEELBASE/(2*Rr));
        Rc = sqrt(SQUARE(Rr)+SQUARE(CAR_WHEELBASE/2));
        alpha = Sf/Rf;
        Tc = fabs(2*Rc*sin(alpha/2.0));
        // printf("canbus->gear = %f\n",canbus->gear);

        if(phi_tire == 0.0)
        {
            Tc = Sf;
            alpha = 0.0;

        }
        if(canbus->gear == 'D' || canbus->gear == 'P' || canbus->gear == 'N')
        {
             __degree__ = alpha/2 + phi_tire_mid;
            __degree__ = yaw +  __degree__;
            yaw = yaw + alpha;

        
        }
        else
        {
            __degree__ = alpha/2 - phi_tire_mid;
            __degree__ = yaw + DEG_TO_RAD(180)-__degree__;
            yaw = yaw - alpha;
        }
        P_now_center[0]=P_now_center[0]+Tc*cos(__degree__);
        P_now_center[1]=P_now_center[1]+Tc*sin(__degree__);
        
        para->exPara.translation[0] =P_now_center[0]/normv;
        para->exPara.translation[2] = P_now_center[1]/normv;
        para->exPara.up[0] = cos(yaw);
        para->exPara.up[1] = 0;
        para->exPara.up[2] = sin(yaw);

        pre_distance = sum_distance;
        if(sum_distance> 0.6)
        {
            //sum_distance = sum_distance - 0.80;
            sum_distance = sum_distance - 0.6;
            P_now_center[0] = 0.0;
            P_now_center[1] = 0.0;
            yaw = DEG_TO_RAD(-90);
            reflashSee = 1;
        }

        rotate_callculate_2D(para);
    #endif
}
static int count_read_fbo = 0;
void set_seeThrough_fbo(GLuint texture, float * rotate_2davm, float * rotate_see)
{
    #if (PARAM_AVM_SEETHROUGH == 1)
        static int flag = 0;
        if(reflashSee == 1)
        {
            if(flag == 0)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, fbo_seethrough.framebuffer);
                {
                    glViewport(0, 0, AVM_2D_FBO_W, AVM_2D_FBO_H);
                    draw_proSee_2davm(rotate_2davm, texture, 1);
                }
            }
            else
            {

                glBindFramebuffer(GL_FRAMEBUFFER, fbo3.framebuffer);
                {
                    glViewport(0, 0, AVM_2D_FBO_W, AVM_2D_FBO_H);
                    draw_proSee_2davm(rotate_2davm, texture, 1);
                    // if(count_read_fbo++ > 15 && count_read_fbo < 100)
                    // {
                    //     count_read_fbo = 100000;

                    //         unsigned char *buffer;
                    //         buffer = (unsigned char *)calloc(AVM_2D_FBO_W * AVM_2D_FBO_H * 4 * sizeof(unsigned char), sizeof(unsigned char));
                    //         glBindFramebuffer(GL_FRAMEBUFFER, fbo3.framebuffer);
                    //         glReadPixels( 0, 0, AVM_2D_FBO_W, AVM_2D_FBO_H, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
                    //         //glReadPixels( 0, 0, 768/2, 774, GL_RGBA, GL_UNSIGNED_BYTE, bufferRGBout);

                    //         FILE * resultS;
                    //         AVM_LOGI("saving windows readpixels\n");
                    //         char z[50];
                    //         sprintf(z, "%s/%s", SOURCE_DIR, "fbo3.rgba");
                    //         resultS = fopen(z, "wb");
                    //         fwrite(buffer, 1, AVM_2D_FBO_W * AVM_2D_FBO_H * 4, resultS);
                    //         fclose(resultS);
                    //         free(buffer);

                    // }
                }

                glClear(GL_DEPTH_BUFFER_BIT);
                glBindFramebuffer(GL_FRAMEBUFFER, fbo3.framebuffer);
                {
                    // 產生底盤+rotate
                    draw_proSee(u_seeThroughArea, rotate_see, rotate_2davm, fbo_seethrough.texture);
                }

                glBindFramebuffer(GL_FRAMEBUFFER, fbo_seethrough.framebuffer);
                {
                    glViewport(0, 0, AVM_2D_FBO_W, AVM_2D_FBO_H);
                    draw_proSee_2davm(rotate_2davm, fbo3.texture, 0);
                }


            }
            flag = 1;
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo3.framebuffer);
            glViewport(0, 0, AVM_2D_FBO_W, AVM_2D_FBO_H);
            draw_proSee_2davm(rotate_see, fbo_seethrough.texture, 0);
        }
    #endif
}

void flash_seethrough_texture()
{
    #if PARAM_AVM_SEETHROUGH
    if(reflashSee == 0)
    {
        autosys.program_context[SEETHROUGH].textureID[FBO_SEETHROUGH_TEXTURE] = fbo3.texture;
    }
    else
    {
        autosys.program_context[SEETHROUGH].textureID[FBO_SEETHROUGH_TEXTURE] = fbo_seethrough.texture;
        reflashSee = 0;
    }
    #endif
}

void draw_seethrough(void)
{
    #if (PARAM_AVM_SEETHROUGH == 1)
    draw_texture(autosys.program_context[SEETHROUGH].textureID[FBO_SEETHROUGH_TEXTURE], 1);
        // if(reflashSee == 0)
        // {
        //     draw_texture(autosys.program_context[SEETHROUGH].textureID[FBO_SEETHROUGH_TEXTURE], 1);
        // }
        // else
        // {
        //     draw_texture(autosys.program_context[SEETHROUGH].textureID[FBO_SEETHROUGH_TEXTURE],1);
        //     reflashSee = 0;
        // }
    #endif
}



float calculate_alpha_average(unsigned char *buffer, int width, int height) {
    unsigned long long total_alpha = 0;
    int pixel_count = width * height;

    for (int i = 0; i < pixel_count; i++) {
        total_alpha += buffer[i * 4 + 3];
    }
    float average_alpha = total_alpha / (float)pixel_count;

    return average_alpha;
}

int seethroughStartflag = 0;
float processValue = 0.0f;
int skip_frame_calculate = 3;
int cur_frame_count = 0;
static void * Seethrough_thread(void * mod)
{
	// pthread_detach(pthread_self());
	while(1)
	{
		if(seethroughStartflag && (cur_frame_count > skip_frame_calculate))
		{
			processValue = calculate_alpha_average(bufferseethrough, 64,64);
            processValue /= 255.0f;
            //AVM_LOGI("[%f] Current seethrough area\n",processValue);
			seethroughStartflag = 0;
            // if(processValue >= 0.97f)
            // {
            //     processValue = 1.0f;
            //     break;
            // }
		}
        // else if(processValue > 99.0f)
        // {
        //     break;
        // }
		else
		{
			usleep(30000);
		}	
		usleep(3000);
	}
	printf("seethrough exit!!!!\n");

	glDeleteBuffers(1,seethroughbufferPBO[0]);
	glDeleteBuffers(1,seethroughbufferPBO[1]);
	free(bufferseethrough);
    pthread_exit(0);
	return NULL;
}

void seethrough_process_init(int w, int h)
{
	glGenBuffers(2, seethroughbufferPBO);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, seethroughbufferPBO[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, w * h * 4, 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, seethroughbufferPBO[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, w * h * 4, 0, GL_STREAM_READ);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    seethroughPBO_w = w;
    seethroughPBO_h = h;
    bufferseethrough = (unsigned char *)calloc(w * h * 4 * sizeof(unsigned char), sizeof(unsigned char));

    pthread_create(&threadSeethrough, NULL, &Seethrough_thread, NULL);
}

float get_seethrough_processValue()
{
    return processValue;
}
void calculate_seethrough_prcess_value()
{
    {
        indexseethrough     = (indexseethrough + 1) % 2;
        nextIndexseethrough = (indexseethrough + 1) % 2;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, seethroughbufferPBO[indexseethrough]);
        glReadPixels(0, 0, seethroughPBO_w, seethroughPBO_h, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        //glGetTexImage(GL_TEXTURE_2D, 6, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glBindBuffer(GL_PIXEL_PACK_BUFFER, seethroughbufferPBO[nextIndexseethrough]);
        void *src = (GLfloat *)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, seethroughPBO_w * seethroughPBO_h * 4, GL_MAP_READ_BIT);
        if (src)
        {
            memcpy(bufferseethrough, src, seethroughPBO_w * seethroughPBO_h * 4);
            //mod.input = bufferMod;
            //AVM_LOGI("[%u] Current seethrough area\n", bufferseethrough[3]);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); // release pointer to the mapped buffer
        }
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); // 解綁
    }
    seethroughStartflag = 1;
    cur_frame_count++;
}