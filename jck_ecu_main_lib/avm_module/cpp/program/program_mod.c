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
#include "mod/mod_main.h"
#include "avm/fp_source.h"
#include "gl/glShaderUtils.h"
#include "program/program_mod.h"
#include "display_location.h"
#include "../../autosys/autosys.h"

#define SAFE_FREE(p) do { if ((p) != NULL) { free(p); (p) = NULL; } } while(0)

//* GL MOD program buffer Container
// static GLuint MOD_VAO[VAO_NUM]; 
// static GLuint MOD_VBO[10][3];

vector_point3F mod_v_buffer     = {0};
vector_point2F mod_text_buffer  = {0};
vector_point3F mod_alpha_buffer = {0};
vector_point2F l_mod_point= {0};
vector_point2F r_mod_point= {0};
MOD_POLYGON_SUPPLY mod_polygon = {0};

GLContext mod;

int modEachNum[4];
float m_mod_color[4] = {1.0,0.0,0.0,1.0};
static float modPosition[192];

void mod_vector_point2dInit(vector_point2F* src);
void mod_vector_point3dInit(vector_point3F* src);

void init_mod_program(void)
{
    AVM_LOGI("-----------------------------\n");
#if READ_OFFLINE_SHADER
    // LoadBinary2Program(&mod.program, "shader_program.bin_format_36805_0");
    // if (mod.program == GL_NONE)
    // {
    //     AVM_LOGI(LOG_RED("[ERROR]") "GL Loading ReverseLine offline file error!!!\n");
    //     AVM_LOGI("[WARNING] Using online program!!!\n");
    //     mod.program = create_program(vertexShader_line, fragmentShader_line, 0);
    // }
#else
        //write_offline_shader_program(mod.program, 0);
#endif
    mod.program = create_program(vertexShader_mod, fragmentShader_mod, 20);
    AVM_LOGI("AVM program_MOD[0]: %d \n", mod.program);
    AVM_LOGI("-----------------------------\n");

    mod_vector_point3dInit(&mod_v_buffer);
    mod_vector_point2dInit(&mod_text_buffer);
    mod_vector_point3dInit(&mod_alpha_buffer);
    mod_vector_point2dInit(&l_mod_point);
    mod_vector_point2dInit(&r_mod_point);
    //TODO : MOD polygon init
    mod_vector_point2dInit(&mod_polygon.l_mod_point_text);
    mod_vector_point2dInit(&mod_polygon.r_mod_point_text);
}

void open_gl_init_mod_line(void)
{
    glUseProgram(mod.program);
    int glAttrrlineVertexMOD = glGetAttribLocation(mod.program, "position");
    int glAttrrlinetexCoord = glGetAttribLocation(mod.program, "texCoord");
    int glAttrrlineAlpha = glGetAttribLocation(mod.program, "alpha");
    glGenVertexArrays(VAO_NUM, mod.vao);

    glGenBuffers(3, &mod.vbo[0][0]);
    {
        glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][0]);                            
        glBufferData(GL_ARRAY_BUFFER, 200*3*sizeof(float), modPosition, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][1]);                            
        glBufferData(GL_ARRAY_BUFFER, 200*2*sizeof(float), modPosition, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][2]);                            
        glBufferData(GL_ARRAY_BUFFER, 200*1*sizeof(float), modPosition, GL_DYNAMIC_DRAW);

        glBindVertexArray(mod.vao[0]);
        glEnableVertexAttribArray(glAttrrlineVertexMOD);
        glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][0]);
        glVertexAttribPointer(glAttrrlineVertexMOD, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glEnableVertexAttribArray(glAttrrlinetexCoord);
        glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][1]);
        glVertexAttribPointer(glAttrrlinetexCoord, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glEnableVertexAttribArray(glAttrrlineAlpha);
        glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][2]);
        glVertexAttribPointer(glAttrrlineAlpha, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);
    }
    glBindVertexArray(0);
}



static void line_width(float *linePoint,float angle, float *point, int avm2d)
{
    float lineWidth=0.0;
    if(avm2d == 0)
    {
        lineWidth = LINEWIDTH_NOR;
    }
    else if(avm2d == 1)
    {
        lineWidth = LINEWIDTH_NOR_2D;
    }
    else
    {
        lineWidth = LINEWIDTH_NOR*2;
    }

    if(angle < 45.0)
    {
        if(avm2d == 0)
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1] + lineWidth/2.0;
            linePoint[2] = point[2];
            linePoint[3] = point[0];
            linePoint[4] = point[1] - lineWidth/2.0;
            linePoint[5] = point[2];
        }
        else if(avm2d == 1)
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1];
            linePoint[2] = point[2] + lineWidth/2.0;
            linePoint[3] = point[0];
            linePoint[4] = point[1];
            linePoint[5] = point[2] - lineWidth/2.0;
        }   
        else
        {
            linePoint[0] = point[0];
            linePoint[1] = point[1] + lineWidth/2.0;
            linePoint[2] = point[2];
            linePoint[3] = point[0];
            linePoint[4] = point[1] - lineWidth/2.0;
            linePoint[5] = point[2];
        }        
    }
    else
    {
        linePoint[0] = point[0] - lineWidth/2.0;
        linePoint[1] = point[1];
        linePoint[2] = point[2];
        linePoint[3] = point[0] + lineWidth/2.0;
        linePoint[4] = point[1];
        linePoint[5] = point[2];
    }
}
void init_mod_line(float carSizeW, float carSizeH, int w, int h)
{
    float minX_a = -carSizeW;
    float maxX_a = carSizeW;

    float minY_a = -carSizeH;
    float maxY_a = carSizeH;

    float carLF[2] = { minX_a, maxY_a };
    float carRF[2] = { maxX_a, maxY_a };
    float carLB[2] = { minX_a, minY_a };
    float carRB[2] = { maxX_a, minY_a };
    
    float position[60] =
    {
        /* 1st detection area left*/
        -0.99, -0.99, 0.0,
        carLB[0], carLB[1], 0.0,
        carLF[0], carLF[1], 0.0,
        -0.99, 0.99, 0.0,
        -0.99, -0.99, 0.0,

        /* 2nd detection area right*/
        0.99, -0.99, 0.0,
        carRB[0], carRB[1], 0.0,
        carRF[0], carRF[1], 0.0,
        0.99, 0.99, 0.0,
        0.99, -0.99, 0.0,

        /* 3th detection area back*/
        -0.99, -0.99, 0.0,
        carLB[0], carLB[1], 0.0,
        carRB[0], carRB[1], 0.0,
        0.99, -0.99, 0.0,
        -0.99, -0.99, 0.0,

        /* 4th detection area front*/
        0.99, 0.99, 0.0,
        carRF[0], carRF[1], 0.0,
        carLF[0], carLF[1], 0.0,
        -0.99, 0.99, 0.0,
        0.99, 0.99, 0.0,
    };
        
    float triposition[120];
    int ix;
    for(ix =0;ix < 10; ix++)
    {
        int index = ix * 3;
        line_width(&triposition[ix*2*3], 50.0, &position[index], 0);
    }
    for(ix =10;ix < 20; ix++)
    {
        int index = ix * 3;
        line_width(&triposition[ix*2*3], 0.0, &position[index], 0);
    }
    memcpy(modPosition , triposition, sizeof(triposition));
}

#pragma region MOD_AVMattach_version

void mod_vector_2d_init(int max_num, vector_point2F* src)
{
    src->pts = (PointF*)calloc(max_num, sizeof(PointF));
}

void mod_vector_2d_add(float x, float y, vector_point2F* src)
{
    int cur_index = src->count;

    src->pts[cur_index].X   = x;
    src->pts[cur_index++].Y = y;
    src->count = cur_index;
}

void mod_vector_2d_free(vector_point2F* src)
{
    if(src->pts != NULL)
    {
        AVM_LOGI("[DEBUG] Freeing vector_point2F address: %p\n", (void*)src->pts);
        //free(src->pts);
        //src->pts = NULL;
    }
    src->count = 0;
    // src->init = NULL;
    // src->add  = NULL;
    // src->vfree = NULL;
    AVM_LOGI("[DEBUF] finsih OK\n");
}

void mod_vector_2d_addRange(float* buffer , int _count, vector_point2F* src)
{
    memcpy(&src->pts[src->count], buffer,sizeof(float) * _count);
    src->count = src->count + (_count/2);
}

void mod_vector_point2dInit(vector_point2F* src)
{
    src->init = mod_vector_2d_init;
    src->add  = mod_vector_2d_add;
    src->vfree = mod_vector_2d_free;
    src->addRange = mod_vector_2d_addRange;

    src->count = 0;
    src->pts = NULL;
    src->init(50, src);
}

void mod_vector_3d_init(int max_num, vector_point3F* src)
{
    src->pts = (VECF3*)calloc(max_num, sizeof(VECF3));
}

void mod_vector_3d_add(vector_point3F* src)
{

}

void mod_vector_3d_free(vector_point3F* src)
{
    if(src->pts != NULL)
    {
        AVM_LOGI("[DEBUG] Freeing vector_point3F address: %p\n", (void*)src->pts);
        //free(src->pts);
        //src->pts = NULL;
    }
    // src->init = NULL;
    // src->add  = NULL;
    // src->vfree = NULL;
    // src->addRange = NULL;
    src->count = 0;
    AVM_LOGI("[DEBUF] finsih OK\n");
}

void mod_vector_3d_addRange(float* buffer , int _count, vector_point3F* src)
{
    memcpy(&src->pts[src->count], buffer,sizeof(float) * _count);
    src->count = src->count + (_count/3);
}

void mod_vector_point3dInit(vector_point3F* src)
{
    src->init = mod_vector_3d_init;
    src->add  = mod_vector_3d_add;
    src->vfree = mod_vector_3d_free;
    src->addRange = mod_vector_3d_addRange;

    src->count = 0;
    src->pts = NULL;
    src->init(16 * 3 * 2, src);
}
void generate_mod_strip_vert(float *src_pt, float w, int pnum, float *dst_pt)
{
    int index = 0;
    PointF tangent;
    tangent.X = 0.0f;
    tangent.Y = 0.0f;
    for (int ix = 0; ix < (pnum-1); ix++)
    {
        PointF end   = 
        {
            .X = src_pt[((ix+1)*3) + 0], 
            .Y = src_pt[((ix+1)*3) + 1]
        };
        PointF start = 
        {
            .X = src_pt[(ix*3 + 0) + 0],
            .Y = src_pt[(ix*3 + 0) + 1]
        };
        if (ix % 2 == 0)
        {
            PointF diff = 
            {
                .X = end.X - start.X,
                .Y = end.Y - start.Y
            };
            float length = sqrtf(diff.X * diff.X + diff.Y * diff.Y);
            if (length < 1e-6) {
                tangent.X = 0.0f;
                tangent.Y = 0.0f;
            } 
            else
            {
                tangent.X = diff.X / length;
                tangent.Y = diff.Y / length;
            }
            float swapX = tangent.X;
            tangent.X = tangent.Y * -1.0f;
            tangent.Y = swapX;
        }



        float offsetX = tangent.X * w / 2.0f;
        float offsetY = tangent.Y * (w * (float)g_PANEL_WIDTH / (float)g_PANEL_HEIGHT) / 2.0f;
        dst_pt[index++] = src_pt[(ix*3) + 0] - offsetX;
        dst_pt[index++] = src_pt[(ix*3) + 1] - offsetY;
        dst_pt[index++] = src_pt[(ix*3) + 2];
        dst_pt[index++] = src_pt[(ix*3) + 0] + offsetX;
        dst_pt[index++] = src_pt[(ix*3) + 1] + offsetY;
        dst_pt[index++] = src_pt[(ix*3) + 2];
    }    
}

PointF Find_CrossPoint_byDirect(PointF IA, PointF CA, PointF target, int axis)
{
    float m = (CA.Y - IA.Y) / (CA.X - IA.X);
    float b = IA.Y - m * IA.X;
    PointF dst;
    if (axis == 0) // x-axis
    {
        dst.X = (target.Y - b) / m;
        dst.Y = target.Y;
    }
    else // y-axis
    {
        dst.X = target.X;
        dst.Y = m * target.X + b;
    }
    return dst;
}

void PixelPoint2GLPoint(PointF *src, PointF *dst, int imgH, int imgW)
{
	float pixelX = src->X;
	float pixelY = src->Y;

	float norx = pixelX / (float)imgW * 2.0f - 1.0f;
	float nory = 1.0f - pixelY / (float)imgH * 2.0f;

	dst->X = norx;
	dst->Y = nory;
}



void GenerateMODFBVertices(int n, float** modVBuffer, float angle, PointF innerBoxArray[2][2], PointF OutBoxArray[2][2],
vector_point2F* leftModPoints, vector_point2F* rightModPoints, float _width)
{
    int screen_width  = g_PANEL_WIDTH;  //width
    int screen_height = g_PANEL_HEIGHT; //height
    PointF o_l_p1_orig;
    PointF i_l_p1_orig;
    PointF o_l_p1_new;
    PointF o_r_p1_orig;
    PointF i_r_p1_orig;
    PointF o_r_p1_new;

    float srcLF[2], srcRF[2];
    float orig_srcLF[2];
    float orig_srcRF[2];

    float ridus_p1;
    float angle_rad = angle * 3.14159265358979323846f / 180.0f;
    float corrected_angle_rad = (-angle + 180.0f) * 3.14159265358979323846f / 180.0f;
    //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 0\033[0m\n");

    int point_num = 13;
    // left point calculate
    {
        o_l_p1_orig = OutBoxArray[n][0];
        i_l_p1_orig = innerBoxArray[n][0];
        ridus_p1 = calculateEuclideanDistance(o_l_p1_orig, i_l_p1_orig);
        o_l_p1_new.X = (float)(cosf(angle_rad) * ridus_p1) + i_l_p1_orig.X;
        o_l_p1_new.Y = (float)(sinf(angle_rad) * ridus_p1) + i_l_p1_orig.Y;
    }
    // right point calculate
    {
        o_r_p1_orig = OutBoxArray[n][1];
        i_r_p1_orig = innerBoxArray[n][1];
        o_r_p1_new.X = (float)(cosf(corrected_angle_rad) * ridus_p1) + i_r_p1_orig.X;
        o_r_p1_new.Y = (float)(sinf(corrected_angle_rad) * ridus_p1) + i_r_p1_orig.Y;
    }
    PointF gl_i_l_p1_orig;
    PointF gl_car_rside_p;
    PixelPoint2GLPoint(&i_l_p1_orig, &gl_i_l_p1_orig, screen_height, screen_width);
    PixelPoint2GLPoint(&i_r_p1_orig, &gl_car_rside_p, screen_height, screen_width);

    float carLF[2] =  { gl_i_l_p1_orig.X, gl_i_l_p1_orig.Y };
    float carRF[2] =  { gl_car_rside_p.X, gl_car_rside_p.Y };
    //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 1\033[0m\n");
    if (false)//calculateEuclideanDistance(o_l_p1_new, o_l_p1_orig) < 10.0f)
    {
        //o_l_p1_new = o_l_p1_orig;
    }
    else
    {
        //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 2\033[0m\n");
        PointF FinallyPoint_l;
        PointF FinallyPoint_r;
        PointF gl_finally_l;
        PointF gl_finally_r;
        if (((o_l_p1_new.Y < o_l_p1_orig.Y) && n == 0) || ((o_l_p1_new.Y > o_l_p1_orig.Y) && n == 1))
        //((angle > -135.0f && n == 0) || ((angle < 135.0f) && n == 1))
        {
            point_num = 8;
            FinallyPoint_l = Find_CrossPoint_byDirect(i_l_p1_orig, o_l_p1_new, o_l_p1_orig, 0);

            PixelPoint2GLPoint(&FinallyPoint_l, &gl_finally_l, screen_height, screen_width);
            srcLF[0] =  gl_finally_l.X;
            srcLF[1] =  gl_finally_l.Y;
            FinallyPoint_r = Find_CrossPoint_byDirect(i_r_p1_orig, o_r_p1_new, o_r_p1_orig, 0);
            PixelPoint2GLPoint(&FinallyPoint_r, &gl_finally_r, screen_height, screen_width);
            srcRF[0] =  gl_finally_r.X;
            srcRF[1] =  gl_finally_r.Y;
            float position[] = 
            {
                srcLF[0],srcLF[1],0.0f,
                srcRF[0],srcRF[1],0.0f,

                srcRF[0],srcRF[1],0.0f,
                carRF[0],carRF[1],0.0f,

                carRF[0],carRF[1],0.0f,
                carLF[0],carLF[1],0.0f,

                carLF[0],carLF[1],0.0f,
                srcLF[0],srcLF[1],0.0f,

                srcLF[0],srcLF[1],0.0f,
            };
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 3\033[0m\n");
            modEachNum[n] = (point_num) * 2;
            *modVBuffer = (float*)calloc(modEachNum[n] * 3, sizeof(float));
            float linewidth = ((float)_width * 2.0f / 400.0f);
            generate_mod_strip_vert(position, linewidth, point_num + 1,  *modVBuffer);

            PixelPoint2GLPoint(&o_l_p1_orig, &gl_finally_l, screen_height, screen_width);
            PixelPoint2GLPoint(&o_r_p1_orig, &gl_finally_r, screen_height, screen_width);
            orig_srcLF[0] =  gl_finally_l.X;
            orig_srcLF[1] =  gl_finally_l.Y;
            orig_srcRF[0] =  gl_finally_r.X;
            orig_srcRF[1] =  gl_finally_r.Y;
            if (n == 0)
            {
                leftModPoints->add(orig_srcLF[0],orig_srcLF[1], leftModPoints);
                leftModPoints->add(srcLF[0],  srcLF[1], leftModPoints);
                leftModPoints->add(carLF[0],  carLF[1],leftModPoints);

                rightModPoints->add(orig_srcRF[0],orig_srcRF[1],rightModPoints);
                rightModPoints->add(srcRF[0],srcRF[1], rightModPoints);
                rightModPoints->add(carRF[0],carRF[1], rightModPoints);
            }
            else
            {
                leftModPoints->add(carLF[0], carLF[1], leftModPoints);
                leftModPoints->add(srcLF[0], srcLF[1], leftModPoints );
                leftModPoints->add(orig_srcLF[0], orig_srcLF[1], leftModPoints);

                rightModPoints->add(carRF[0], carRF[1], rightModPoints);
                rightModPoints->add(srcRF[0], srcRF[1], rightModPoints);
                rightModPoints->add(orig_srcRF[0], orig_srcRF[1], rightModPoints);
            }
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 4\033[0m\n");
        }
        else
        {
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 5\033[0m\n");
            point_num = 14;
            FinallyPoint_l = Find_CrossPoint_byDirect(i_l_p1_orig, o_l_p1_new, o_l_p1_orig, 1);
            PixelPoint2GLPoint(&FinallyPoint_l, &gl_finally_l, screen_height, screen_width);
            srcLF[0] =  gl_finally_l.X;
            srcLF[1] =  gl_finally_l.Y;
            PixelPoint2GLPoint(&o_l_p1_orig, &gl_finally_l, screen_height, screen_width);
            orig_srcLF[0] =  gl_finally_l.X;
            orig_srcLF[1] =  gl_finally_l.Y;

            FinallyPoint_r = Find_CrossPoint_byDirect(i_r_p1_orig, o_r_p1_new, o_r_p1_orig, 1);
            PixelPoint2GLPoint(&FinallyPoint_r, &gl_finally_r, screen_height, screen_width);
            srcRF[0] =  gl_finally_r.X;
            srcRF[1] =  gl_finally_r.Y;
            PixelPoint2GLPoint(&o_r_p1_orig, &gl_finally_r, screen_height, screen_width);
            orig_srcRF[0] =  gl_finally_r.X;
            orig_srcRF[1] =  gl_finally_r.Y;
            float position[] = 
            {
                srcLF[0],srcLF[1],0.0f,
                orig_srcLF[0],orig_srcLF[1],0.0f,

                orig_srcLF[0],orig_srcLF[1],0.0f,
                orig_srcRF[0],orig_srcRF[1],0.0f,

                orig_srcRF[0],orig_srcRF[1],0.0f,
                srcRF[0],srcRF[1],0.0f,

                srcRF[0],srcRF[1],0.0f,
                carRF[0],carRF[1],0.0f,

                carRF[0],carRF[1],0.0f,
                carLF[0],carLF[1],0.0f,

                carLF[0],carLF[1],0.0f,
                srcLF[0],srcLF[1],0.0f,

                srcLF[0],srcLF[1],0.0f,
                orig_srcLF[0],orig_srcLF[1],0.0f,

                orig_srcLF[0],orig_srcLF[1],0.0f,
            };
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 6\033[0m\n");
            modEachNum[n] = (point_num) * 2;
            *modVBuffer = (float*)calloc(modEachNum[n] * 3, sizeof(float));
            float linewidth = ((float)_width * 2.0f / 400.0f);
            //AVM_LOGI("\033[31m[DEBUF] SRC LF[%f,%f] ORIG LF[%f,%f]\033[0m\n",
            //srcRF[0],srcRF[1],
            //orig_srcRF[0],orig_srcRF[1]);
            generate_mod_strip_vert(position, linewidth, point_num + 1, *modVBuffer);
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 6-2\033[0m\n");
            if (n == 0)
            {
                leftModPoints->add(srcLF[0], srcLF[1], leftModPoints);
                leftModPoints->add(carLF[0], carLF[1], leftModPoints);

                rightModPoints->add(srcRF[0], srcRF[1], rightModPoints);
                rightModPoints->add(carRF[0], carRF[1], rightModPoints);
            }
            else
            {
                leftModPoints->add(carLF[0], carLF[1], leftModPoints);
                leftModPoints->add(srcLF[0], srcLF[1], leftModPoints);

                rightModPoints->add(carRF[0], carRF[1], rightModPoints);
                rightModPoints->add(srcRF[0], srcRF[1], rightModPoints);
            }
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 7\033[0m\n");
        }

    }
    //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 8\033[0m\n");
}

void GenerateSideVertices(int n, float** modVBuffer, vector_point2F* leftModPoints, vector_point2F* rightModPoints, float _width)
{
    vector_point2F* setPoint = (n == 2) ? leftModPoints : rightModPoints;
    int totolPcount = setPoint->count * 2;
    float *position = (float*)calloc(((totolPcount + 2) * 2 + 1) * 3, sizeof(float));
    modEachNum[n] = (totolPcount + 2) * 2;
    *modVBuffer = (float*)calloc(modEachNum[n] * 3, sizeof(float));
    int count = 0;
    int hold_flag = 0;
    for (int i = 0; i < setPoint->count; i++)
    {
        position[count++] = setPoint->pts[i].X;
        position[count++] = setPoint->pts[i].Y;
        position[count++] = 0.0f;

        if (i != 0 && i < setPoint->count) {
            position[count++] = setPoint->pts[i].X;
            position[count++] = setPoint->pts[i].Y;
            position[count++] = 0.0f;
        }
    }
    position[count++] = setPoint->pts[0].X;
    position[count++] = setPoint->pts[0].Y;
    position[count++] = 0.0f;


    position[count++] = setPoint->pts[0].X;
    position[count++] = setPoint->pts[0].Y;
    position[count++] = 0.0f;

    position[count++] = setPoint->pts[1].X;
    position[count++] = setPoint->pts[1].Y;
    position[count++] = 0.0f;

    position[count++] = setPoint->pts[1].X;
    position[count++] = setPoint->pts[1].Y;
    position[count++] = 0.0f;

    float linewidth = (_width * 2.0f / 400.0f);
    int point_num = totolPcount + 2;
    generate_mod_strip_vert(position, linewidth, point_num + 1, *modVBuffer);
    free(position);
}

#pragma region Polygon{
//************************* */
//* Polygon function start
//************************* */
void safy_mod_buffer_free(void **ptr)
{
    if(*ptr != NULL)
    {
        free(*ptr);
        *ptr = NULL;
    }
}
void generate_mod_strip_vert_Polygon(float* src_pt, float w, int pnum, float* dst_pt)
{
    int index = 0;
    for (int ix = 0; ix < (pnum - 1); ix++)
    {
        dst_pt[index++] = src_pt[(ix * 3) + 0];
        dst_pt[index++] = src_pt[(ix * 3) + 1];
        dst_pt[index++] = src_pt[(ix * 3) + 2];
    }
}
void gl_Polygon_2_Triangle(MOD_POLYGON_SUPPLY* polygon,int* nodeNum)
{
    // Supply->vertArray = (float*)calloc(modEachNum[n] * 3, sizeof(float));
    SAFE_FREE(polygon->triangle_vertArray);
    SAFE_FREE(polygon->triangle_textArray);
    SAFE_FREE(polygon->triangle_alphaArray);
    int total_fanNum  = *nodeNum - 2;
    //AVM_LOGI("total_fanNum: %d\n", total_fanNum);
    float *tri_vert  = (float*)calloc(total_fanNum * 3 * 3,sizeof(float));
    float *tri_text  = (float*)calloc(total_fanNum * 3 * 2,sizeof(float));
    float *tri_alpha = (float*)calloc(total_fanNum * 3 * 1,sizeof(float));

    int vertCount  = 0;
    int textCount  = 0;
    int alphaCount = 0;
    for (int ix = 0; ix < total_fanNum; ix++)
    {
        tri_vert[vertCount++]    =  polygon->vertArray[0];
        tri_vert[vertCount++]    =  polygon->vertArray[1];
        tri_vert[vertCount++]    =  polygon->vertArray[2];
        tri_text[textCount++]    =  polygon->textArray[0];
        tri_text[textCount++]    =  polygon->textArray[1];
        tri_alpha[alphaCount++]  =  polygon->alphaArray[0];

        tri_vert[vertCount++]   = polygon->vertArray[(ix+1) * 3 + 0];
        tri_vert[vertCount++]   = polygon->vertArray[(ix+1) * 3 + 1];
        tri_vert[vertCount++]   = polygon->vertArray[(ix+1) * 3 + 2];
        tri_text[textCount++]   = polygon->textArray[(ix+1) * 2 + 0];
        tri_text[textCount++]   = polygon->textArray[(ix+1) * 2 + 1];
        tri_alpha[alphaCount++] = polygon->alphaArray[(ix+1) * 1 + 0];

        tri_vert[vertCount++]   = polygon->vertArray[(ix + 2)* 3 + 0];
        tri_vert[vertCount++]   = polygon->vertArray[(ix + 2)* 3 + 1];
        tri_vert[vertCount++]   = polygon->vertArray[(ix + 2)* 3 + 2];
        tri_text[textCount++]   = polygon->textArray[(ix + 2)* 2 + 0];
        tri_text[textCount++]   = polygon->textArray[(ix + 2)* 2 + 1];
        tri_alpha[alphaCount++] = polygon->alphaArray[(ix + 2) * 1 + 0];

    }

    polygon->triangle_vertArray  = tri_vert;
    polygon->triangle_textArray  = tri_text;
    polygon->triangle_alphaArray = tri_alpha;

    *nodeNum = total_fanNum * 3;
}

void GenerateSideVertices_Polygon(int n, float** modVBuffer, vector_point2F* leftModPoints, vector_point2F* rightModPoints, float _width, MOD_POLYGON_SUPPLY *Supply)
{
    SAFE_FREE(Supply->vertArray);
    SAFE_FREE(Supply->textArray);
    SAFE_FREE(Supply->alphaArray);
    vector_point2F* setPoint     = (n == 2) ? leftModPoints : rightModPoints;
    vector_point2F* settextPoint = (n == 2) ? &(Supply->l_mod_point_text) : &(Supply->r_mod_point_text);
    
    int totolPcount = setPoint->count ;

    float *position = (float*)calloc(setPoint->count * 3, sizeof(float));
    modEachNum[n]      = setPoint->count ;
    Supply->vertArray  = (float*)calloc(modEachNum[n] * 3, sizeof(float));
    Supply->textArray  = (float*)calloc(modEachNum[n] * 2, sizeof(float));
    Supply->alphaArray = (float*)calloc(modEachNum[n] * 1, sizeof(float));
    if (modEachNum[n] == 4)
    {
        Supply->alphaArray[0] = 1.0f;
        Supply->alphaArray[1] = 0.0f;
        Supply->alphaArray[2] = 0.0f;
        Supply->alphaArray[3] = 1.0f;
    }
    else
    {
        Supply->alphaArray[0] = 1.0f;
        Supply->alphaArray[1] = 1.0f;
        Supply->alphaArray[2] = 0.0f;
        Supply->alphaArray[3] = 0.0f;
    }
    int total_count = modEachNum[n];
    for(int i = 0; i < total_count; i++)
    {
        Supply->textArray[2 * i + 0] = settextPoint->pts[i].X;
        Supply->textArray[2 * i + 1] = settextPoint->pts[i].Y;
    }
    int count = 0;
    int hold_flag = 0;
    for (int i = 0; i < setPoint->count; i++)
    {
        position[count++] = setPoint->pts[i].X;
        position[count++] = setPoint->pts[i].Y;
        position[count++] = 0.0f;
    }


    float linewidth = (_width * 2.0f / 400.0f);
    int point_num = totolPcount;
    generate_mod_strip_vert_Polygon(position, linewidth, point_num + 1, Supply->vertArray);
    SAFE_FREE(position);
}


void GenerateMODFBVertices_Polygon(int n, float** modVBuffer, float angle, PointF innerBoxArray[2][2], PointF OutBoxArray[2][2],
vector_point2F* leftModPoints, vector_point2F* rightModPoints, float _width,MOD_POLYGON_SUPPLY *Supply)
{
    int screen_width  = g_PANEL_WIDTH;  //width
    int screen_height = g_PANEL_HEIGHT; //height
    PointF o_l_p1_orig;
    PointF i_l_p1_orig;
    PointF o_l_p1_new;
    PointF o_r_p1_orig;
    PointF i_r_p1_orig;
    PointF o_r_p1_new;

    float srcLF[2], srcRF[2];
    float orig_srcLF[2];
    float orig_srcRF[2];

    float ridus_p1;
    float angle_rad = angle * 3.14159265358979323846f / 180.0f;
    float corrected_angle_rad = (-angle + 180.0f) * 3.14159265358979323846f / 180.0f;
    //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 0\033[0m\n");

    int point_num = 13;
    // left point calculate
    {
        o_l_p1_orig = OutBoxArray[n][0];
        i_l_p1_orig = innerBoxArray[n][0];
        ridus_p1 = calculateEuclideanDistance(o_l_p1_orig, i_l_p1_orig);
        o_l_p1_new.X = (float)(cosf(angle_rad) * ridus_p1) + i_l_p1_orig.X;
        o_l_p1_new.Y = (float)(sinf(angle_rad) * ridus_p1) + i_l_p1_orig.Y;
    }
    // right point calculate
    {
        o_r_p1_orig = OutBoxArray[n][1];
        i_r_p1_orig = innerBoxArray[n][1];
        o_r_p1_new.X = (float)(cosf(corrected_angle_rad) * ridus_p1) + i_r_p1_orig.X;
        o_r_p1_new.Y = (float)(sinf(corrected_angle_rad) * ridus_p1) + i_r_p1_orig.Y;
    }
    PointF gl_i_l_p1_orig;
    PointF gl_car_rside_p;
    PixelPoint2GLPoint(&i_l_p1_orig, &gl_i_l_p1_orig, screen_height, screen_width);
    PixelPoint2GLPoint(&i_r_p1_orig, &gl_car_rside_p, screen_height, screen_width);

    float carLF[2] =  { gl_i_l_p1_orig.X, gl_i_l_p1_orig.Y };
    float carRF[2] =  { gl_car_rside_p.X, gl_car_rside_p.Y };
    //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 1\033[0m\n");
    if (false)//calculateEuclideanDistance(o_l_p1_new, o_l_p1_orig) < 10.0f)
    {
        //o_l_p1_new = o_l_p1_orig;
    }
    else
    {
        //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 2\033[0m\n");
        PointF FinallyPoint_l;
        PointF FinallyPoint_r;
        PointF gl_finally_l;
        PointF gl_finally_r;
        if (((o_l_p1_new.Y <= o_l_p1_orig.Y) && n == 0) || ((o_l_p1_new.Y > o_l_p1_orig.Y) && n == 1))
        //((angle > -135.0f && n == 0) || ((angle < 135.0f) && n == 1))
        {
            point_num = 4;
            FinallyPoint_l = Find_CrossPoint_byDirect(i_l_p1_orig, o_l_p1_new, o_l_p1_orig, 0);

            PixelPoint2GLPoint(&FinallyPoint_l, &gl_finally_l, screen_height, screen_width);
            srcLF[0] =  gl_finally_l.X;
            srcLF[1] =  gl_finally_l.Y;
            FinallyPoint_r = Find_CrossPoint_byDirect(i_r_p1_orig, o_r_p1_new, o_r_p1_orig, 0);
            PixelPoint2GLPoint(&FinallyPoint_r, &gl_finally_r, screen_height, screen_width);
            srcRF[0] =  gl_finally_r.X;
            srcRF[1] =  gl_finally_r.Y;
            float position[] = 
            {
                srcLF[0],srcLF[1],0.0f,
                srcRF[0],srcRF[1],0.0f,
                carRF[0],carRF[1],0.0f,
                carLF[0],carLF[1],0.0f,
            };
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 3\033[0m\n");
            modEachNum[n] = (point_num) ;
            SAFE_FREE(Supply->vertArray);
            SAFE_FREE(Supply->textArray);
            SAFE_FREE(Supply->alphaArray);
            Supply->vertArray = (float*)calloc(modEachNum[n] * 3, sizeof(float));
            Supply->textArray  = (float*)calloc(modEachNum[n] * 2, sizeof(float));
            Supply->alphaArray = (float*)calloc(modEachNum[n] * 1, sizeof(float));
            Supply->textArray[0] = (FinallyPoint_l.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[1] = (FinallyPoint_l.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->textArray[2] = (FinallyPoint_r.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[3] = (FinallyPoint_r.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->textArray[4] = (i_r_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[5] = (i_r_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->textArray[6] = (i_l_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[7] = (i_l_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->alphaArray[0] = 1.0f;
            Supply->alphaArray[1] = 1.0f;
            Supply->alphaArray[2] = 0.0f;
            Supply->alphaArray[3] = 0.0f;

            float linewidth = ((float)_width * 2.0f / 400.0f);
            generate_mod_strip_vert_Polygon(position, linewidth, point_num + 1,  Supply->vertArray);

            PixelPoint2GLPoint(&o_l_p1_orig, &gl_finally_l, screen_height, screen_width);
            PixelPoint2GLPoint(&o_r_p1_orig, &gl_finally_r, screen_height, screen_width);
            orig_srcLF[0] =  gl_finally_l.X;
            orig_srcLF[1] =  gl_finally_l.Y;
            orig_srcRF[0] =  gl_finally_r.X;
            orig_srcRF[1] =  gl_finally_r.Y;
            if (n == 0)
            {
                leftModPoints->add(orig_srcLF[0],orig_srcLF[1], leftModPoints);
                leftModPoints->add(srcLF[0],  srcLF[1], leftModPoints);
                leftModPoints->add(carLF[0],  carLF[1],leftModPoints);

                rightModPoints->add(orig_srcRF[0],orig_srcRF[1],rightModPoints);
                rightModPoints->add(srcRF[0],srcRF[1], rightModPoints);
                rightModPoints->add(carRF[0],carRF[1], rightModPoints);
       
                float px_l = (o_l_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
                float py_l = (o_l_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;
                float px_r = (o_r_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
                float py_r = (o_r_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;
                Supply->l_mod_point_text.add(px_l, py_l, &(Supply->l_mod_point_text));
                Supply->l_mod_point_text.add(Supply->textArray[0], Supply->textArray[1], &(Supply->l_mod_point_text));
                Supply->l_mod_point_text.add(Supply->textArray[6], Supply->textArray[7], &(Supply->l_mod_point_text));

                Supply->r_mod_point_text.add(px_r, py_r, &(Supply->r_mod_point_text));
                Supply->r_mod_point_text.add(Supply->textArray[2], Supply->textArray[3], &(Supply->r_mod_point_text));
                Supply->r_mod_point_text.add(Supply->textArray[4], Supply->textArray[5], &(Supply->r_mod_point_text));
            }
            else
            {
                leftModPoints->add(carLF[0], carLF[1], leftModPoints);
                leftModPoints->add(srcLF[0], srcLF[1], leftModPoints );
                leftModPoints->add(orig_srcLF[0], orig_srcLF[1], leftModPoints);

                rightModPoints->add(carRF[0], carRF[1], rightModPoints);
                rightModPoints->add(srcRF[0], srcRF[1], rightModPoints);
                rightModPoints->add(orig_srcRF[0], orig_srcRF[1], rightModPoints);

                float px_l = (o_l_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
                float py_l = (o_l_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;
                float px_r = (o_r_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
                float py_r = (o_r_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;
                Supply->l_mod_point_text.add(Supply->textArray[6], Supply->textArray[7], &(Supply->l_mod_point_text));
                Supply->l_mod_point_text.add(Supply->textArray[0], Supply->textArray[1], &(Supply->l_mod_point_text));
                Supply->l_mod_point_text.add(px_l, py_l, &(Supply->l_mod_point_text));

                Supply->r_mod_point_text.add(Supply->textArray[4], Supply->textArray[5], &(Supply->r_mod_point_text));
                Supply->r_mod_point_text.add(Supply->textArray[2], Supply->textArray[3], &(Supply->r_mod_point_text));
                Supply->r_mod_point_text.add(px_r, py_r, &(Supply->r_mod_point_text));
            }
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 4\033[0m\n");
        }
        else
        {
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 5\033[0m\n");
            point_num = 6;
            FinallyPoint_l = Find_CrossPoint_byDirect(i_l_p1_orig, o_l_p1_new, o_l_p1_orig, 1);
            PixelPoint2GLPoint(&FinallyPoint_l, &gl_finally_l, screen_height, screen_width);
            srcLF[0] =  gl_finally_l.X;
            srcLF[1] =  gl_finally_l.Y;
            PixelPoint2GLPoint(&o_l_p1_orig, &gl_finally_l, screen_height, screen_width);
            orig_srcLF[0] =  gl_finally_l.X;
            orig_srcLF[1] =  gl_finally_l.Y;

            FinallyPoint_r = Find_CrossPoint_byDirect(i_r_p1_orig, o_r_p1_new, o_r_p1_orig, 1);
            PixelPoint2GLPoint(&FinallyPoint_r, &gl_finally_r, screen_height, screen_width);
            srcRF[0] =  gl_finally_r.X;
            srcRF[1] =  gl_finally_r.Y;
            PixelPoint2GLPoint(&o_r_p1_orig, &gl_finally_r, screen_height, screen_width);
            orig_srcRF[0] =  gl_finally_r.X;
            orig_srcRF[1] =  gl_finally_r.Y;
            float position[] = 
            {
                srcLF[0],srcLF[1],0.0f,
                orig_srcLF[0],orig_srcLF[1],0.0f,
                orig_srcRF[0],orig_srcRF[1],0.0f,
                srcRF[0],srcRF[1],0.0f,
                carRF[0],carRF[1],0.0f,
                carLF[0],carLF[1],0.0f,
            };
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 6\033[0m\n");
            modEachNum[n] = (point_num) ;
            SAFE_FREE(Supply->vertArray);
            SAFE_FREE(Supply->textArray);
            SAFE_FREE(Supply->alphaArray);
            Supply->vertArray = (float*)calloc(modEachNum[n] * 3, sizeof(float));
            Supply->textArray    = (float*)calloc(modEachNum[n] * 2, sizeof(float));
            Supply->textArray[0] = (FinallyPoint_l.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[1] = (FinallyPoint_l.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->textArray[2] = (o_l_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[3] = (o_l_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->textArray[4] = (o_r_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[5] = (o_r_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->textArray[6] = (FinallyPoint_r.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[7] = (FinallyPoint_r.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->textArray[8] = (i_r_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[9] = (i_r_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->textArray[10] = (i_l_p1_orig.X - Supply->OffSet.X) / Supply->UV_IMGWidth;
            Supply->textArray[11] = (i_l_p1_orig.Y - Supply->OffSet.Y) / Supply->UV_IMGHeight;

            Supply->alphaArray = (float*)calloc(modEachNum[n] * 1, sizeof(float));

            Supply->alphaArray[0] = 1.0f;
            Supply->alphaArray[1] = 1.0f;
            Supply->alphaArray[2] = 1.0f;
            Supply->alphaArray[3] = 1.0f;
            Supply->alphaArray[4] = 0.0f;
            Supply->alphaArray[5] = 0.0f;
            float linewidth = ((float)_width * 2.0f / 400.0f);
            //AVM_LOGI("\033[31m[DEBUF] SRC LF[%f,%f] ORIG LF[%f,%f]\033[0m\n",
            //srcRF[0],srcRF[1],
            //orig_srcRF[0],orig_srcRF[1]);
            generate_mod_strip_vert_Polygon(position, linewidth, point_num + 1, Supply->vertArray);
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 6-2\033[0m\n");
            if (n == 0)
            {
                leftModPoints->add(srcLF[0], srcLF[1], leftModPoints);
                leftModPoints->add(carLF[0], carLF[1], leftModPoints);

                rightModPoints->add(srcRF[0], srcRF[1], rightModPoints);
                rightModPoints->add(carRF[0], carRF[1], rightModPoints);
                
                Supply->l_mod_point_text.add(Supply->textArray[0], Supply->textArray[1], &(Supply->l_mod_point_text));
                Supply->l_mod_point_text.add(Supply->textArray[10], Supply->textArray[11], &(Supply->l_mod_point_text));

                Supply->r_mod_point_text.add(Supply->textArray[6], Supply->textArray[7], &(Supply->r_mod_point_text));
                Supply->r_mod_point_text.add(Supply->textArray[8], Supply->textArray[9], &(Supply->r_mod_point_text));
            }
            else
            {
                leftModPoints->add(carLF[0], carLF[1], leftModPoints);
                leftModPoints->add(srcLF[0], srcLF[1], leftModPoints);

                rightModPoints->add(carRF[0], carRF[1], rightModPoints);
                rightModPoints->add(srcRF[0], srcRF[1], rightModPoints);

                Supply->l_mod_point_text.add(Supply->textArray[10], Supply->textArray[11], &(Supply->l_mod_point_text));
                Supply->l_mod_point_text.add(Supply->textArray[0], Supply->textArray[1], &(Supply->l_mod_point_text));

                Supply->r_mod_point_text.add(Supply->textArray[8], Supply->textArray[9], &(Supply->r_mod_point_text));
                Supply->r_mod_point_text.add(Supply->textArray[6], Supply->textArray[7], &(Supply->r_mod_point_text));
            }
            //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 7\033[0m\n");
        }

    }
    //AVM_LOGI("\033[31m[DEBUF] GenerateMODFBVertices 8\033[0m\n");
}

// void mod_polygon_free_init(MOD_POLYGON_SUPPLY* supply)
// {

// }




#pragma endregion}

void upload_newMODline(int *modbuffer)
{
#if true

    float mod_top_angle  = autosys.system_info.BOWL_angle[0];
    float mod_down_angle = autosys.system_info.BOWL_angle[2];

    m_mod_color[0] = (float)modbuffer[0]/255.0;
    m_mod_color[1] = (float)modbuffer[1]/255.0;
    m_mod_color[2] = (float)modbuffer[2]/255.0;
    m_mod_color[3] = (float)modbuffer[3]/255.0;

    float width    = (float)modbuffer[4];

    float userInnerTopp = (float)modbuffer[10];
    float userInnerLeft = (float)modbuffer[9];
    float userInnerWidh = (float)modbuffer[11];
    float userInnerHeih = (float)modbuffer[12];

    float userOutTopp  = (float)modbuffer[6];
    float userOutLeft  = (float)modbuffer[5];
    float userOutWidh  = (float)modbuffer[7];
    float userOutHeih  = (float)modbuffer[8];


    // for(int ix = 0; ix < 9; ix++)
    // {
    //     AVM_LOGI("[value] %d\n", modbuffer[4 + ix]);
    // }

    PointF inner_l_ptf = {userInnerLeft, userInnerTopp};
    PointF inner_r_ptf = {userInnerLeft + userInnerWidh, userInnerTopp};
    PointF inner_l_ptb = {userInnerLeft, userInnerTopp + userInnerHeih};
    PointF inner_r_ptb = {userInnerLeft + userInnerWidh, userInnerTopp + userInnerHeih};

    PointF out_l_ptf   = {userOutLeft, userOutTopp};
    PointF out_r_ptf   = {userOutLeft + userOutWidh, userOutTopp};
    PointF out_l_ptb   = {userOutLeft, userOutTopp + userOutHeih};
    PointF out_r_ptb   = {userOutLeft + userOutWidh, userOutTopp + userOutHeih};

    PointF innerBoxArray[2][2] = {
        {inner_l_ptf, inner_r_ptf},
        {inner_l_ptb, inner_r_ptb}
    };

    PointF outBoxArray[2][2] = {
        {out_l_ptf, out_r_ptf},
        {out_l_ptb, out_r_ptb}
    };

    //! It is tmporary value
    mod_polygon.OffSet.X = userOutLeft;
    mod_polygon.OffSet.Y = userOutTopp;
    mod_polygon.UV_IMGHeight = userOutHeih;
    mod_polygon.UV_IMGWidth  = userOutWidh;
    mod_polygon.UV_Ratio_HbyW = userOutHeih / userOutWidh;
    if(userOutHeih == 0.0f) return;
    mod_polygon.mask.X = 0.1f;
    mod_polygon.mask.Y = 0.9f;
    mod_polygon.mask.Z = 0.1f * userOutWidh / userOutHeih;
    mod_polygon.mask.W = 0.9f + (0.1f - mod_polygon.mask.Z);
    mod_top_angle  = 45.0f;
    mod_down_angle = 45.0f;
    float angle = mod_top_angle - 180.0f;// -135.0f; //-155
    angle = (angle >= -91.0f) ? -90.5f : angle;

    int version = autosys.system_info.mod_version;
    // vector_point3F mod_v_buffer= {0};
    // vector_point2F l_mod_point= {0};
    // vector_point2F r_mod_point= {0};
    //AVM_LOGI("\033[1m\033[33m[DEBUF] ENTER TO THE MOD\033[1m\033[0m\n");

#if 1
    //AVM_LOGI("\033[1m\033[33m[DEBUF] MOD vecter init OK\033[1m\033[0m\n");
    for (int n = 0; n < 4; n++)
    {
        float* triposition = NULL;
        switch (n)
        {
            case 1:
                angle = mod_down_angle - 180.0f;
                angle = (angle >= -91.0f) ? -90.5f : angle;
                angle *= -1.0f;
            case 0:
                //AVM_LOGI("\033[1m\033[33m[DEBUF] front * back %d\033[1m\033[0m\n", n);
                if(version == 0)
                {
                    GenerateMODFBVertices(n, &triposition, angle, innerBoxArray, outBoxArray,  &l_mod_point,  &r_mod_point, width);
                }
                else
                {
                GenerateMODFBVertices_Polygon(n, &triposition, angle, innerBoxArray, outBoxArray,  &l_mod_point,  &r_mod_point, width, &mod_polygon);
                }
                break;
            case 2:
            case 3:
                if(version == 0)
                {
                    GenerateSideVertices(n, &triposition, &l_mod_point, &r_mod_point, width);
                }
                else
                {
                GenerateSideVertices_Polygon(n, &triposition, &l_mod_point, &r_mod_point, width, &mod_polygon);
                }   
                break;
        }

            //mod_polygon.vertArray = triposition;
        if(version == 0)
        {
            if (triposition != NULL)
            {
                mod_v_buffer.addRange(triposition, modEachNum[n]*3, &mod_v_buffer);
                free(triposition);
                triposition = NULL;
            }
        }
        else
        {
        if (mod_polygon.vertArray != NULL && modEachNum[n] != 0)
        {
           
            //mod_polygon.vertArray = triposition;
            gl_Polygon_2_Triangle(&mod_polygon, &modEachNum[n]);
            mod_v_buffer.addRange(mod_polygon.triangle_vertArray, modEachNum[n] *3, &mod_v_buffer);
            mod_text_buffer.addRange(mod_polygon.triangle_textArray, modEachNum[n] * 2, &mod_text_buffer);
            }
        }
            //mod_alpha_buffer.addRange(mod_polygon.triangle_alphaArray, modEachNum[n]/ 3 * 1, &mod_alpha_buffer);
            //safy_mod_buffer_free(triposition);
            //AVM_LOGI("\033[1m\033[33m[DEBUF] merge_num[%d] OK %d\033[1m\033[0m\n", modEachNum[n], n);

    }

    // AVM_LOGI("\033[1m\033[33m[DEBUF] MOD\033[1m\033[0m\n");
    // for(int ix = 0; ix < mod_v_buffer.count; ix++)
    // {
    //     AVM_LOGI("x, y, z [%3.3f, %3.3f, %3.3f]\n", mod_v_buffer.pts[ix].X, mod_v_buffer.pts[ix].Y, mod_v_buffer.pts[ix].Z);
    // }
#endif
// AVM_LOGI("l_mod_point.pts address: %p\n", (void*)l_mod_point.pts);
// AVM_LOGI("r_mod_point.pts address: %p\n", (void*)r_mod_point.pts);
    glBindVertexArray(mod.vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mod_v_buffer.count * 3 * sizeof(float), &mod_v_buffer.pts[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mod_text_buffer.count * 2 * sizeof(float), &mod_text_buffer.pts[0]);
    // glBindBuffer(GL_ARRAY_BUFFER, mod.vbo[0][2]);
    // glBufferSubData(GL_ARRAY_BUFFER, 0, mod_alpha_buffer.count * 1 * sizeof(float), &mod_alpha_buffer.pts[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    r_mod_point.vfree(&r_mod_point);
    l_mod_point.vfree(&l_mod_point);
    mod_v_buffer.vfree(&mod_v_buffer);
    mod_text_buffer.vfree(&mod_text_buffer);
    //mod_alpha_buffer.vfree(&mod_alpha_buffer);

    //TODO : MOD polygon free
    mod_polygon.l_mod_point_text.vfree(&mod_polygon.l_mod_point_text);
    mod_polygon.r_mod_point_text.vfree(&mod_polygon.r_mod_point_text);
#endif

}

#pragma endregion


void draw_mod(void)
{
    int version = autosys.system_info.mod_version;
    #if (MODsta==1)
        int modDetectSW[4];
        get_modDetectSW(modDetectSW);

        glUseProgram(mod.program);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(mod.vao[0]);

        glUniform1i(glGetUniformLocation(mod.program, "switchPosition"), 2);
        glUniform1f(glGetUniformLocation(mod.program, "yRervsed"), yReverse);
        glUniform1f(glGetUniformLocation(mod.program, "UV_Ratio"), mod_polygon.UV_Ratio_HbyW);


        // set_color(GL_COLOR_RED);
        //set_color(GL_COLOR_YELLOW);
        glUniform4fv(glGetUniformLocation(mod.program, "color"), 1, m_mod_color);
        //? Temporary
    // modDetectSW[0] = 1;
    // modDetectSW[1] = 1;
    // modDetectSW[2] = 1;
    // modDetectSW[3] = 1;
        // modDetectSW[0] = 1;
    switch (version)
    {
    case 0:
        glUniform1i(glGetUniformLocation(mod.program, "windows"), 0);
        if (modDetectSW[0] == 1) // left
        {
            glDrawArrays(GL_TRIANGLE_STRIP, modEachNum[0] + modEachNum[1], modEachNum[2]);
        }
        if (modDetectSW[1] == 1) // right
        {
            glDrawArrays(GL_TRIANGLE_STRIP, modEachNum[1] + modEachNum[2] + modEachNum[0], modEachNum[3]);
        }
        if (modDetectSW[3] == 1) // back
        {
            glDrawArrays(GL_TRIANGLE_STRIP, modEachNum[0], modEachNum[1]);
        }
        if (modDetectSW[2] == 1) // front
        {
            glDrawArrays(GL_TRIANGLE_STRIP, 0, modEachNum[0]);
        }
        break;
    case 1:
        glUniform1i(glGetUniformLocation(mod.program, "windows"), 1);
        glUniform4fv(glGetUniformLocation(mod.program, "RectMask"),1, mod_polygon.mask.data);

        if(modDetectSW[0] == 1) //left
        {
            glUniform1i(glGetUniformLocation(mod.program, "type"), 2);
            glDrawArrays(GL_TRIANGLES, modEachNum[0] + modEachNum[1], modEachNum[2]);
            //glDrawArrays(GL_TRIANGLE_STRIP, modEachNum[0] + modEachNum[1], modEachNum[2]);
        }
        if(modDetectSW[1] == 1) //right
        {
            glUniform1i(glGetUniformLocation(mod.program, "type"), 3);
            glDrawArrays(GL_TRIANGLES, modEachNum[1] + modEachNum[2] + modEachNum[0], modEachNum[3]);
            
        }
        if(modDetectSW[3] == 1) //back
        {
            glUniform1i(glGetUniformLocation(mod.program, "type"), 1);
            glDrawArrays(GL_TRIANGLES, modEachNum[0], modEachNum[1]);
            
        }
        if(modDetectSW[2] == 1) //front
        {
            glUniform1i(glGetUniformLocation(mod.program, "type"), 0);
            glDrawArrays(GL_TRIANGLES, 0, modEachNum[0]);
        }
        break;
        }

AutoSys_mod_left_flag  = modDetectSW[0] ;
AutoSys_mod_right_flag = modDetectSW[1] ;
AutoSys_mod_front_flag = modDetectSW[2] ;
AutoSys_mod_back_flag  = modDetectSW[3] ;


        // if(AutoSys_mod_left_flag == 1)
        // {
        //     glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
        // }
        // if(AutoSys_mod_right_flag == 1)
        // {
        //     glDrawArrays(GL_TRIANGLE_STRIP, 10, 10);
        // }
        // if(AutoSys_mod_front_flag == 1)
        // {
        //     glDrawArrays(GL_TRIANGLE_STRIP, 30, 10);
        // }
        // if(AutoSys_mod_back_flag == 1)
        // {
        //     glDrawArrays(GL_TRIANGLE_STRIP, 20, 10);
        // }
        
        glBindVertexArray(0);
        glDisable(GL_BLEND);
        // glDisable(GL_DEPTH_TEST);
        // glDisable(GL_DEPTH_TEST);
    #endif
}