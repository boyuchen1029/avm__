/**
 *******************************************************************************
 * @file : default_vertex.c
 * @describe : Vertex shader code.
 *
 * @author : Woody.
 * @verstion : 0.1.0.
 * @date 20211109 0.1.0 Woody.
 * @date 20211222 0.1.1 Linda.
 *******************************************************************************
*/
//
// Created by Administrator on 2018/6/27.
//

#include "../../../system.h"

static const char DEFAULT_VERTEX_SHADER[] = AVM_VALUE(
        precision lowp float;
        attribute vec3 aPosition;
        attribute vec4 aColor;
        attribute vec2 aCoordinate;
        varying vec4 coordColor;
        varying vec2 aVaryingCoordinate;

        void main()
        {
            gl_Position = vec4(aPosition, 1.0);
            coordColor = aColor;
            aVaryingCoordinate = aCoordinate;
        }
);
static const char vertShader_composite_oit[] = AVM_VALUE(#version 300 es\n

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

void main()
{
    TexCoord = texCoord;
    gl_Position = vec4(position, 0.0, 1.0);
}

);

static const char vertexShader_car_main[] = AVM_VALUE(#version 300 es\n

    in vec3 position;
    in vec2 texCoords;
    in vec3 normal;

    out vec2 TexCoord;
    out vec3 WorldPos;
    out vec3 Normal;
    // varying lowp mat3 TBN;

    uniform mat4 MVPRotate;
    uniform mat4 RUDDER_TIRE;

    uniform mat4 Projection;
    uniform mat4 View;
    uniform mat4 Model;
    uniform mat4 Axisdensity;

    uniform mat4 clipworld;

    uniform lowp mat3 ModelToWorld;

    uniform lowp vec3 Shift;
    uniform float yRervsed;
    uniform float CarSize;
    uniform int flowing_lamp_uesd;
    void main()
    {
        lowp vec4 a_position_tmp;
        lowp vec4 position_cal;
        position_cal =  MVPRotate * (vec4(position,1) - vec4(Shift,0));
        WorldPos = (position_cal + vec4(Shift,0)).xyz;

        a_position_tmp = clipworld * (position_cal + vec4(Shift,0));
        a_position_tmp.g = a_position_tmp.g * yRervsed;
        gl_Position = a_position_tmp;

        TexCoord = texCoords;
        Normal = vec4(ModelToWorld * normal , 1.0).xyz;
        //Normal = (vec4(mat3(transpose(inverse(Model))) * normal / CarSize, 1.0)).xyz;
        // if (gl_VertexID % 3 == 0) {
        //     Normal = (vec4(mat3(transpose(inverse(Model))) * normal / CarSize, 1.0)).xyz;
        // } 
        // // 否則重用前一個計算結果
        // else {
        //     Normal = Normal; // 重用前面的 normal 值
        // }
    }
);

static const char vertexShader_main[]=AVM_VALUE(#version 300 es\n
        in vec3 vPosition;
        in vec2 a_texCoordFront;
        in vec2 a_texCoordBack;
        in vec2 a_texCoordLeft;
        in vec2 a_texCoordRight;
        in vec2 a_texCoordLeftBSD;
        in vec2 a_texCoordRightBSD;
        in vec2 a_tex_alpha;

        uniform mat4 transformatiomMatrix;
        uniform int cameraPosition;

        out vec2 TexCoordFront;
        out vec2 TexCoordBack;
        out vec2 TexCoordLeft;
        out vec2 TexCoordRight;
        out vec2 TexCoordLeftBSD;
        out vec2 TexCoordRightBSD;
        out vec2 TexCoordAlpha;

        void main()
        {
            if(cameraPosition == 0)
            {
                gl_Position =  transformatiomMatrix * vec4(vPosition,1);
            }
            else if(cameraPosition == 1)
            {
                gl_Position =  transformatiomMatrix * vec4(-vPosition.x, vPosition.y, vPosition.z, 1);
            }
            else
            {
                gl_Position =  vec4(vPosition,1);
            }
            TexCoordAlpha = a_tex_alpha;
            TexCoordFront = a_texCoordFront;
            TexCoordBack = a_texCoordBack;
            TexCoordLeft = a_texCoordLeft;
            TexCoordRight = a_texCoordRight;
            TexCoordLeftBSD = a_texCoordLeftBSD;
            TexCoordRightBSD = a_texCoordRightBSD;
        }
);

static const char vertexShader_main_fb[]=AVM_VALUE(#version 300 es\n
        in vec3 vPosition;
        in vec2 a_texCoord;
        in vec2 a_tex_alpha;

        uniform mat4 transformatiomMatrix;
        uniform int cameraPosition;

        out vec2 TexCoord;
        out vec2 TexCoordAlpha;
        uniform float yRervsed;
        uniform int Mirror;
        void main()
        {
            lowp vec4 a_position_tmp;
            if(cameraPosition == 0)
            {
                a_position_tmp =  transformatiomMatrix * vec4(vPosition,1);
                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
            }
            else if(cameraPosition == 1)
            {
                a_position_tmp =  transformatiomMatrix * vec4(-vPosition.x, vPosition.y, vPosition.z, 1);
                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
            }
            else if(cameraPosition == 2)
            {
                a_position_tmp =  transformatiomMatrix * vec4(vPosition,1);
                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
            }

            TexCoordAlpha = a_tex_alpha;
            TexCoord.x = (Mirror == 2)?(1.0f-a_texCoord.x):a_texCoord.x;
            TexCoord.y = a_texCoord.y;
        }
);

static const char vertexShader_main_lr[]=AVM_VALUE(#version 300 es\n
        in vec3 vPosition;
        in vec2 a_texCoord;
        in vec2 a_tex_alpha;

        uniform mat4 transformatiomMatrix;
        uniform int cameraPosition;

        out vec2 TexCoord;
        out vec2 TexCoordAlpha;
        uniform float yRervsed;
        struct scope
        {
            mat4 scaleMatrix;
            mat4 translationMatrix;
        };
        uniform scope ScopeArea;
        void main()
        {
            lowp vec4 a_position_tmp;

            if(cameraPosition == 0)
            {
                a_position_tmp =  transformatiomMatrix * vec4(vPosition,1);

                a_position_tmp.z = a_position_tmp.z + 0.01;

                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
            }
            else if(cameraPosition == 1)
            {
                a_position_tmp =  transformatiomMatrix * vec4(-vPosition.x, vPosition.y, vPosition.z, 1);

                a_position_tmp.z = a_position_tmp.z + 0.01;

                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
            }
            else if(cameraPosition == 2)
            {
                a_position_tmp = transformatiomMatrix * vec4(vPosition, 1);

                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
            }
            else if(cameraPosition == 4)
            {
                vec4 scaledPosition = ScopeArea.scaleMatrix * vec4(vPosition, 1);
                a_position_tmp      = transformatiomMatrix * ScopeArea.translationMatrix * scaledPosition;
                a_position_tmp.g    = a_position_tmp.g * yRervsed;
                gl_Position         =  a_position_tmp;
            }
            TexCoord = a_texCoord;
            TexCoordAlpha = a_tex_alpha;
        }
);


/**
 * 
 *         string vertexShaderSource =
            @"  #version 300 es
                in vec3 Position;
                in vec2 InTexCoord;
                in float InAlpCoord;

                uniform mat4 transformatiomMatrix;
                uniform int cameraPosition;

                out vec2 TexCoord;
                out float TexCoordAlpha;

                void main(void)
                {
                    gl_Position = transformatiomMatrix * vec4(Position, 1.0);
                    TexCoordAlpha = InAlpCoord;
                    TexCoord = InTexCoord;
                }
            ";
 * 
*/

// test_shadow
static const char vertexShader_car_shadow[]=AVM_VALUE(#version 300 es\n
        in vec3 vPosition;
        in vec2 InTexCoord;
        in float alphaValue;
        
        out float TexCoordAlpha;
        out vec2 TexCoord;
        uniform mat4 transformatiomMatrix;
        uniform float yRervsed;
        void main()
        {
            vec4 a_position_tmp;
            a_position_tmp =  transformatiomMatrix * vec4(vPosition, 1.0);
            a_position_tmp.g = a_position_tmp.g * yRervsed;
            
            gl_Position = a_position_tmp;

            // TexCoord = InTexCoord;
            TexCoordAlpha = alphaValue;
            TexCoord = vec2(InTexCoord.x, 1.0 - InTexCoord.y); 
        }
);

static const char vertexShader_rgba[] = AVM_VALUE(
        attribute vec3 position;
        attribute vec2 IntexCoord;
        varying vec2 texCoord;

        uniform float yRervsed;
        uniform mat4 Projection;
        void main(void)
        {
            lowp vec4 a_position_tmp;

            texCoord = IntexCoord;

            a_position_tmp = Projection * vec4(position,1);
            a_position_tmp.g = a_position_tmp.g * yRervsed;
            gl_Position = a_position_tmp;
//            gl_Position = vec4(position,1);
        }
);



static const char vertexShader_line[] = AVM_VALUE(
        precision mediump float;
        struct scope
        {
            mat4 scaleMatrix;
            mat4 translationMatrix;
        };
        
        attribute vec3 position;
        attribute vec2 InTexCoord;
        uniform int switchPosition;
        varying vec3 positionSh;
        varying vec2 texCoord;
        uniform mat4 transformatiomMatrix;
        uniform float yRervsed;
        uniform scope ScopeArea;

        void main(void)
        {
            lowp vec4 a_position_tmp;

            if(switchPosition == 0)
            {
                //vec4 scaledPosition = ScopeArea.scaleMatrix * vec4(position, 1);
                //a_position_tmp = transformatiomMatrix * ScopeArea.translationMatrix * scaledPosition;

                a_position_tmp = transformatiomMatrix * vec4(position, 1);
                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
//                gl_Position = vec4(a_position_tmp.r, -a_position_tmp.g, a_position_tmp.b, a_position_tmp.a);

                positionSh = position;
            }
            else if(switchPosition == 1)
            {
                a_position_tmp =  transformatiomMatrix * vec4(position,1);
                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
//                gl_Position = vec4(a_position_tmp.r, -a_position_tmp.g, a_position_tmp.b, a_position_tmp.a);
            }
            else if(switchPosition == 3)
            {
                a_position_tmp =  transformatiomMatrix * vec4(position.xy, position.z * 10.0,1);
                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
                positionSh = position;
            }
            else
            {
                a_position_tmp =  vec4(position,1);
                a_position_tmp.g = a_position_tmp.g * yRervsed;
                gl_Position = a_position_tmp;
            }
            texCoord = InTexCoord;
        }
);

static const char vertexShader_blur[] =AVM_VALUE(#version 300 es\n
    in vec3 InPosition;
    in vec2 InTexCoord;
    out vec2 TexCoord;
    uniform mat4 transformatiomMatrix;
    void main(void)
    {
        vec4 a_position_tmp;
        a_position_tmp =  transformatiomMatrix * vec4(InPosition,1);
        a_position_tmp.g = a_position_tmp.g * -1.0;
        gl_Position = a_position_tmp;
        TexCoord.y = InTexCoord.y;
        TexCoord.x = InTexCoord.x;
    }
);

static const char vertexShader_mod[] = AVM_VALUE(
        precision mediump float;
   
        attribute vec3 position;
        attribute vec2 texCoord;
        attribute float alpha;

        varying vec3 positionSh;
        varying vec2 TexCoord;
        varying float alphaSh;
        uniform int switchPosition;
        uniform float yRervsed;

        void main(void)
        {
            lowp vec4 a_position_tmp;
            a_position_tmp =  vec4(position,1);
            a_position_tmp.g = a_position_tmp.g * yRervsed;
            gl_Position = a_position_tmp;

            alphaSh  = alpha;
            TexCoord = texCoord;
        }
);

static const char vertexShader_uyvy[] = AVM_VALUE(
        attribute vec3 position;
        attribute vec2 IntexCoord;
        varying vec2 texCoord;

        void main(void)
        {
            texCoord = IntexCoord;
            gl_Position = vec4(position,1);
        }
);

static const char vertexShader_see_through[] = AVM_VALUE(
        precision lowp float;
        attribute vec3 a_Position;
        attribute vec2 a_texCoord;
        varying lowp vec4 v_vertexCoord;
        varying lowp vec2 v_texCoord;
        varying vec3 v_Position;
        uniform float yRervsed;

        uniform mat4 u_transformationMatrix;
        uniform mat4 u_transformationMatrix2;

        void main(void)
        {
            vec4 tt = u_transformationMatrix * vec4(a_Position, 1.0);
            vec4 tt2 = u_transformationMatrix2 * vec4(a_Position, 1.0);
            // tt.g = tt.g * yRervsed;
            // tt2.g = tt2.g * yRervsed;
            gl_Position = tt;
            v_vertexCoord = tt;
            
            v_texCoord = a_texCoord;
            v_Position = a_Position;
        }
);

static const char vertexShader_see_through_2davm[] = AVM_VALUE(
        precision lowp float;
        attribute vec3 a_Position;
        attribute vec2 a_texCoord;
        varying lowp vec2 v_texCoord;

        uniform mat4 u_transformationMatrix;

        void main(void)
        {
            vec4 tt = u_transformationMatrix * vec4(a_Position, 1.0);
            gl_Position = tt;

            // lowp vec4 a_position_tmp;
            // a_position_tmp =  u_transformationMatrix * vec4(a_Position, 1.0);
            // a_position_tmp.g = a_position_tmp.g * -1.0;
            
            // gl_Position = a_position_tmp;

            v_texCoord = a_texCoord;
        }
);
static const char vertexShader_y[] = AVM_VALUE(
        attribute vec3 vPosition;
        attribute vec2 a_TexCoord;

        varying vec2 v_texture;
        uniform float yRervsed;

        void main()
        {
            lowp vec4 a_position_tmp;

//           gl_Position = vec4(vPosition,1.0);
            a_position_tmp = vec4(vPosition,1.0);
            a_position_tmp.g = a_position_tmp.g * yRervsed;
            gl_Position = a_position_tmp;
//            gl_Position = vec4(a_position_tmp.r, -a_position_tmp.g, a_position_tmp.b, a_position_tmp.a);
           v_texture = a_TexCoord;
        }
);

static const char vertexShader_FXAA[] = AVM_VALUE(#version 300 es\n
    in vec3 vPosition;
    in vec2 InTexCoord;

    out vec2 TexCoord;

    void main()
    {
        vec4 poistion_tmp;
        poistion_tmp = vec4(vPosition, 1.0);
        gl_Position = poistion_tmp;
        TexCoord = InTexCoord;
    }


);
