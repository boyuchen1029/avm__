/**
 *******************************************************************************
 * @file : default_frag.c
 * @describe : Fragment shader code.
 *
 * @author : Woody.
 * @verstion : 0.1.0.
 * @date 20211109 0.1.0 Woody.
 * @date 20211222 0.1.1 Linda.
 *******************************************************************************
*/

#include "../../../system.h"
#include "../../autosys/autosys.h"
//
// Created by Administrator on 2018/6/27.
//
static const char DEFAULT_FRAG_SHADER[] = AVM_VALUE(
        precision lowp float;
        uniform sampler2D sampler;
        varying vec2 aVaryingCoordinate;
        varying vec4 coordColor;

        void main(){
            gl_FragColor = texture2D(sampler, aVaryingCoordinate.xy);
        }
);
#pragma region CarModel

#if PARAM_OPENGL_OIT_COLOR_ALPHA_INDIVIUAL
static const char fragmentShader_composite_oit[] = AVM_VALUE(#version 300 es\n
    
    precision mediump  float;

    uniform sampler2D accumColorTex;
    uniform sampler2D accumAlphaTex;
    uniform sampler2D accumColorTex_static;
    uniform sampler2D accumAlphaTex_static;
    uniform int UseStaticIMG;
    in vec2 TexCoord;
    out vec4 finalColor;
    void main()
    {
        vec3  color      = texture(accumColorTex, TexCoord).rgb;
        //vec3  alpha      = texture(accumColorTex, TexCoord).r;
        float background = texture(accumAlphaTex, TexCoord).r;

        if(UseStaticIMG == 1)
        {
            color      += texture(accumColorTex_static, TexCoord).rgb;
            background += texture(accumAlphaTex_static, TexCoord).r;
        }

        background = max(background, 1e-5);
        if(background <= 1e-5) 
        {
            finalColor = vec4(0.0,0.0,0.0,0.0);
        }
        else
        {
            vec3 finalRGB = color / background * 2.0;
            finalColor = vec4(finalRGB, background);
        }
    }
);

static const char fragmentShader_car_main_oit[] = AVM_VALUE(#version 300 es\n
    
    precision mediump  float;

    layout(location = 0) out vec4 accumColor;
    layout(location = 1) out float accumAlpha;

    in mediump vec2 TexCoord;
    in mediump vec3 Normal;
    in mediump vec3 WorldPos;

    uniform vec3 viewPos;
    uniform int seethroughAlhpaOn;
    uniform int Self_luminous_status;

    uniform sampler2D diffuseMap;

    uniform vec3 lightPos[10];
    uniform vec3 lightColor;
    uniform float bright[10];

    uniform float alpha;
    uniform int windows;
    uniform int lightNum;
    uniform float power;

    uniform int lightMode;
    uniform int skyMode;

    uniform samplerCube skybox;

    float yrgb(vec3 rgb)
    {
        float y;
        y = rgb.r * 0.299+rgb.g*-0.169+rgb.b*0.5;
        return y;
    }

    void main(void)
    {
        if(windows == 0)
        {
            vec4 diffuse      = texture(diffuseMap, TexCoord);
            vec3 specular     = diffuse.bgr;
            vec3 ambient      = vec3(0.0);
            vec3 lighting     = vec3(0.0);
            vec3 viewDir      = normalize(viewPos - WorldPos);
            vec3 lightAmbient = 0.1 * diffuse.bgr;
            float glossiness  = 1.0-yrgb(diffuse.bgr);

            int i = 0;
            if(lightMode == 0)
            {
                vec3 lightDir   = normalize(lightPos[0]);
                vec3 halfwayDir = normalize(lightDir + viewDir);

                vec3 lightDiffuse  = 0.5 * diffuse.bgr * max(dot(Normal, lightDir), 0.0);
                vec3 lightSpecular = bright[0] * specular * pow(max(dot(Normal, halfwayDir), 0.0), glossiness * 8.0) * lightColor.bgr;


                //vec3 tmplighting = lightAmbient + lightDiffuse + lightSpecular;
				//lighting += mix(tmplighting, mix( tmplighting, diffuse.bgr, power),float(Self_luminous_status));

                if(Self_luminous_status == 1)
                {
                    lighting += power * diffuse.bgr + (1.0 - power) * (lightAmbient + lightDiffuse + lightSpecular);
                }
                else
                {
                    lighting += (lightAmbient + lightDiffuse + lightSpecular);
                }
            }
            else
            {
                for (i = 0; i < 10; i++)
                {
                   if(i<lightNum)
                   {
                        vec3 lightVector = lightPos[i] - WorldPos;
                        vec3 lightDir   = normalize(lightVector);
                        vec3 halfwayDir = normalize(lightDir + viewDir);

                        float distance = length(lightVector);

                        float attenuation = 1.0 /   (1.0 + 
                                                    0.0014 * distance + 
                                                    0.000007 * distance * distance);

                        vec3 lightDiffuse = 0.5 * diffuse.bgr * max(dot(Normal, lightDir), 0.0);
                        vec3 lightSpecular = bright[i] * specular * pow(max(dot(Normal, halfwayDir), 0.0), glossiness * 32.0) * lightColor.bgr;

                        if(Self_luminous_status == 1)
                        {
                            lighting += power * diffuse.bgr + (1.0 - power) * attenuation * (lightAmbient + lightDiffuse + lightSpecular);
                        }
                        else
                        {
                            lighting += attenuation * (lightAmbient + lightDiffuse + lightSpecular);
                        }
                   }
                }
            }

            vec3 reflectedColor = vec3(0,0,0);
            if(skyMode == 1)
            {
                reflectedColor = texture(skybox, reflect(normalize(WorldPos), Normal)).rgb;
            }
            else
            {
                reflectedColor = lighting;
            }

            vec3 finalColor = vec3(0,0,0);
            if(Self_luminous_status == 1)
            {
                finalColor = lighting;
            }
            else
            {
                finalColor = mix(reflectedColor, lighting, glossiness);
            }    
    
            vec3 weightedColor = (finalColor.bgr * alpha );
            accumColor = vec4(weightedColor, alpha); 
            accumAlpha    = alpha ;
        }
        //else if(windows == 2)
        //{
            //accumColor = vec4(color.rgb * alpha, 1.0f); 
            //accumColor = vec4(sin(float(windows)) * cos(float(windows)), 0.0, 0.0, 1.0);
            //accumColor = vec4(1.0,1.0,1.0, 1.0); 
        //}
        else
        {
            vec3 diffuse = texture(diffuseMap, TexCoord).rgb;
            vec3 safeColor = mix(vec3(0.01), diffuse.bgr, alpha);
            vec3 weightedColor = safeColor * alpha ;
            accumColor = vec4(weightedColor, 1.0); 
            accumAlpha = alpha;
        }

    }
);

#else
static const char fragmentShader_car_main_oit[] = AVM_VALUE(#version 300 es\n
    
    precision mediump  float;

    layout(location = 0) out vec4 accumColor;

    in lowp vec2 TexCoord;
    in lowp vec3 WorldPos;
    in lowp vec3 Normal;

    uniform vec3 viewPos;
    uniform int seethroughAlhpaOn;
    uniform int Self_luminous_status;

    uniform sampler2D diffuseMap;

    uniform vec3 lightPos[10];
    uniform vec3 lightColor;
    uniform float bright[10];

    uniform float alpha;
    uniform int windows;
    uniform int lightNum;
    uniform float power;

    uniform int lightMode;
    uniform int skyMode;

    uniform samplerCube skybox;


    float yrgb(vec3 rgb)
    {
        float y;
        y = rgb.r * 0.299+rgb.g*-0.169+rgb.b*0.5;
        return y;
    }


    void main(void)
    {
        if(windows == 0)
        {
            vec4 diffuse      = texture(diffuseMap, TexCoord);
            vec3 specular     = diffuse.bgr;
            vec3 ambient      = vec3(0.0);
            vec3 lighting     = vec3(0.0);
            vec3 viewDir      = normalize(viewPos - WorldPos);
            vec3 lightAmbient = 0.1 * diffuse.bgr;
            float glossiness  = 1.0-yrgb(diffuse.bgr);

            int i = 0;
            if(lightMode == 0)
            {
                vec3 lightDir   = normalize(lightPos[0]);
                vec3 halfwayDir = normalize(lightDir + viewDir);

                vec3 lightDiffuse  = 0.5 * diffuse.bgr * max(dot(Normal, lightDir), 0.0);
                vec3 lightSpecular = bright[0] * specular * pow(max(dot(Normal, halfwayDir), 0.0), glossiness * 8.0) * lightColor.bgr;


                //vec3 tmplighting = lightAmbient + lightDiffuse + lightSpecular;
				//lighting += mix(tmplighting, mix( tmplighting, diffuse.bgr, power),float(Self_luminous_status));

                if(Self_luminous_status == 1)
                {
                    lighting += power * diffuse.bgr + (1.0 - power) * (lightAmbient + lightDiffuse + lightSpecular);
                }
                else
                {
                    lighting += (lightAmbient + lightDiffuse + lightSpecular);
                }
            }
            else
            {
                for (i = 0; i < 10; i++)
                {
                   if(i<lightNum)
                   {
                        vec3 lightVector = lightPos[i] - WorldPos;
                        vec3 lightDir   = normalize(lightVector);
                        vec3 halfwayDir = normalize(lightDir + viewDir);

                        float distance = length(lightVector);

                        float attenuation = 1.0 /   (1.0 + 
                                                    0.0014 * distance + 
                                                    0.000007 * distance * distance);

                        vec3 lightDiffuse = 0.5 * diffuse.bgr * max(dot(Normal, lightDir), 0.0);
                        vec3 lightSpecular = bright[i] * specular * pow(max(dot(Normal, halfwayDir), 0.0), glossiness * 32.0) * lightColor.bgr;

                        if(Self_luminous_status == 1)
                        {
                            lighting += power * diffuse.bgr + (1.0 - power) * attenuation * (lightAmbient + lightDiffuse + lightSpecular);
                        }
                        else
                        {
                            lighting += attenuation * (lightAmbient + lightDiffuse + lightSpecular);
                        }
                   }
                }
            }


            //vec3 reflectedColor = texture(skybox, vec3(1,1,0)).bgr;
            //vec3 reflectedColor = texture(skybox, reflect(normalize(WorldPos - viewPos), Normal)).bgr;
            vec3 reflectedColor;
            if(skyMode == 1)
            {
                reflectedColor = texture(skybox, reflect(normalize(WorldPos), Normal)).rgb;
            }
            else
            {
                reflectedColor = lighting;
            }

            vec3 finalColor;//     = mix(reflectedColor, lighting, glossiness);
            if(Self_luminous_status == 1)
            {
                finalColor = lighting;
            }
            else
            {
                finalColor = mix(reflectedColor, lighting, glossiness);
            }  
            //fragColor = vec4(finalColor.bgr, alpha);
            //vec3 safeColor = mix(vec3(0.05), finalColor.bgr, alpha);
            vec3 weightedColor = finalColor.bgr * alpha ;
            accumColor = vec4(weightedColor, alpha); 
        }
        else if(windows == 1)
        {
            vec3 carRGB = texture(diffuseMap, TexCoord).rgb;
            //vec3 safeColor = mix(vec3(0.05), carRGB, alpha);
            vec3 weightedColor = carRGB * alpha ;
            accumColor = vec4(weightedColor, alpha); 
        }

    }
);

static const char fragmentShader_composite_oit[] = AVM_VALUE(#version 300 es\n
    
    precision mediump  float;

    uniform sampler2D accumColorTex;
    uniform sampler2D accumAlphaTex;

    in vec2 TexCoord;
    out vec4 finalColor;
    void main()
    {
        vec3  color      = texture(accumColorTex, TexCoord).rgb;
        float background = texture(accumColorTex, TexCoord).a;
        background = max(background, 1e-5);
        if(background <= 1e-5) 
        {
            finalColor = vec4(0.0,0.0,0.0,0.0);
        }
        else
        {
            vec3 finalRGB = color / background;
            finalColor = vec4(finalRGB, background);
        }
    }
);
#endif

#if 0
static const char fragmentShader_composite_oit[] = AVM_VALUE(#version 300 es\n
    
    precision mediump  float;

    uniform sampler2D accumColorTex;
    uniform sampler2D accumAlphaTex;

    in vec2 TexCoord;
    out vec4 finalColor;

    #if 0 
    uniform float obj_alpha;
    void main()
    {
        vec3  color      = texture(accumColorTex, TexCoord).rgb;
        float background = texture(accumColorTex, TexCoord).a;
        float alpha      = texture(accumAlphaTex, TexCoord).r;
        //float alpha      = 0.5;
    
        if (length(color) < 1e-5)
        {
            finalColor = vec4(0.0,0.0,0.0,0.0);
        }
        else
        {
            alpha = max(alpha, 1e-5);
            vec3 finalRGB = color / alpha;
            if (alpha <= 1e-5)
            {
                finalColor = vec4(0.0, 0.0, 0.0, 0.0);
                // finalColor = vec4(finalRGB, background);
            }
            else
            {
                float brightness = dot(finalRGB, vec3(0.2126, 0.7152, 0.0722));
                if (brightness < 0.05)
                {
                    finalRGB = vec3(0.05);
                }
                finalColor = vec4(finalRGB, alpha);
            }
        }
    }
    #else 
    void main()
    {
        vec3  color      = texture(accumColorTex, TexCoord).rgb;
        float background = texture(accumColorTex, TexCoord).a;
        background = max(background, 1e-5);
        if(background <= 1e-5) 
        {
            finalColor = vec4(0.0,0.0,0.0,0.0);
        }
        else
        {
            vec3 finalRGB = color / background;
            finalColor = vec4(finalRGB, background);
        }
    }
    #endif
);

static const char fragmentShader_car_main_oit[] = AVM_VALUE(#version 300 es\n
    
    precision mediump  float;

    layout(location = 0) out vec4 accumColor;
    //layout(location = 1) out float accumAlpha;

    in lowp vec2 TexCoord;
    in lowp vec3 WorldPos;
    in lowp vec3 Normal;

    uniform vec3 viewPos;
    uniform int seethroughAlhpaOn;
    uniform int Self_luminous_status;

    uniform sampler2D diffuseMap;

    uniform vec3 lightPos[10];
    uniform vec3 lightColor;
    uniform float bright[10];

    uniform float alpha;
    uniform int windows;
    uniform int lightNum;
    uniform float power;

    uniform int lightMode;
    uniform int skyMode;

    uniform samplerCube skybox;

    void main(void)
    {
        if(windows == 1)
        {
            #if 0
            vec3 carRGB = texture(diffuseMap, TexCoord).rgb;
            vec3 safeColor = mix(vec3(0.05), carRGB, alpha);
            //vec3 weightedColor = safeColor * alpha ;
            vec3 weightedColor = safeColor * alpha ;
            accumColor = vec4(weightedColor, 1.0); 
            accumAlpha = alpha;
            #else
            vec3 carRGB = texture(diffuseMap, TexCoord).rgb;
            vec3 safeColor = mix(vec3(0.05), carRGB, alpha);
            //vec3 weightedColor = safeColor * alpha ;
            vec3 weightedColor = safeColor * alpha ;
            accumColor = vec4(weightedColor, alpha); 
            #endif
        }

    }
);

#endif


static const char fragmentShader_car_main[] = AVM_VALUE(#version 300 es\n
    precision lowp float;
    out vec4 fragColor;
    in lowp vec2 TexCoord;
    in lowp vec3 WorldPos;
    in lowp vec3 Normal;

    uniform vec3 viewPos;
    uniform sampler2D diffuseMap;
    uniform sampler2D specularMap;
    uniform sampler2D glossisessMap;

    uniform int seethroughAlhpaOn;
    uniform int Self_luminous_status;

    uniform vec3 lightPos[10];
    uniform vec3 lightColor;
    uniform float bright[10];

    uniform float alpha;
    uniform int windows;
    uniform int lightNum;
    uniform float power;

    uniform int lightMode;
    uniform int skyMode;

    uniform samplerCube skybox;

    float yrgb(vec3 rgb)
    {
        float y;
        y = rgb.r * 0.299+rgb.g*-0.169+rgb.b*0.5;
        return y;
    }

    void main(void)
    {
        if(windows == 0)
        {
            vec4 diffuse      = texture(diffuseMap, TexCoord);
            vec3 specular     = diffuse.bgr;
            vec3 ambient      = vec3(0.0);
            vec3 lighting     = vec3(0.0);
            vec3 viewDir      = normalize(viewPos - WorldPos);
            vec3 lightAmbient = 0.1 * diffuse.bgr;
            float glossiness  = 1.0-yrgb(diffuse.bgr);

            int i = 0;
            if(lightMode == 0)
            {
                vec3 lightDir   = normalize(lightPos[0]);
                vec3 halfwayDir = normalize(lightDir + viewDir);

                vec3 lightDiffuse  = 0.5 * diffuse.bgr * max(dot(Normal, lightDir), 0.0);
                vec3 lightSpecular = bright[0] * specular * pow(max(dot(Normal, halfwayDir), 0.0), glossiness * 8.0) * lightColor.bgr;


                //vec3 tmplighting = lightAmbient + lightDiffuse + lightSpecular;
				//lighting += mix(tmplighting, mix( tmplighting, diffuse.bgr, power),float(Self_luminous_status));

                if(Self_luminous_status == 1)
                {
                    lighting += power * diffuse.bgr + (1.0 - power) * (lightAmbient + lightDiffuse + lightSpecular);
                }
                else
                {
                    lighting += (lightAmbient + lightDiffuse + lightSpecular);
                }
            }
            else
            {
                for (i = 0; i < 10; i++)
                {
                   if(i<lightNum)
                   {
                        vec3 lightVector = lightPos[i] - WorldPos;
                        vec3 lightDir   = normalize(lightVector);
                        vec3 halfwayDir = normalize(lightDir + viewDir);

                        float distance = length(lightVector);

                        float attenuation = 1.0 /   (1.0 + 
                                                    0.0014 * distance + 
                                                    0.000007 * distance * distance);

                        vec3 lightDiffuse = 0.5 * diffuse.bgr * max(dot(Normal, lightDir), 0.0);
                        vec3 lightSpecular = bright[i] * specular * pow(max(dot(Normal, halfwayDir), 0.0), glossiness * 32.0) * lightColor.bgr;

                        if(Self_luminous_status == 1)
                        {
                            lighting += power * diffuse.bgr + (1.0 - power) * attenuation * (lightAmbient + lightDiffuse + lightSpecular);
                        }
                        else
                        {
                            lighting += attenuation * (lightAmbient + lightDiffuse + lightSpecular);
                        }
                   }
                }
            }


            //vec3 reflectedColor = texture(skybox, vec3(1,1,0)).bgr;
            //vec3 reflectedColor = texture(skybox, reflect(normalize(WorldPos - viewPos), Normal)).bgr;
            vec3 reflectedColor;
            if(skyMode == 1)
            {
                reflectedColor = texture(skybox, reflect(normalize(WorldPos), Normal)).rgb;
            }
            else
            {
                reflectedColor = lighting;
            }

            vec3 finalColor;//     = mix(reflectedColor, lighting, glossiness);
            if(Self_luminous_status == 1)
            {
                finalColor = lighting;
            }
            else
            {
                finalColor = mix(reflectedColor, lighting, glossiness);
            }  
            //fragColor = vec4(finalColor.bgr, alpha);
            fragColor = vec4(finalColor.bgr * alpha, alpha);
        }
        else if(windows == 1)
        {
            vec3 carRGB = texture(diffuseMap, TexCoord).rgb;
            //fragColor = vec4(carRGB, alpha);
            fragColor = vec4(carRGB * alpha, alpha);
        }
        else if(windows == 2)
        {
            vec4 carRGB = texture(diffuseMap, TexCoord);
            if(seethroughAlhpaOn == 1)
            {
                carRGB.a *= 1.3;
                fragColor = vec4(carRGB);
            }
            else
            {
                if(carRGB.a > 0.0) carRGB.a = 1.0;
                float temp_a = alpha;
                carRGB.a *= temp_a;
                fragColor = carRGB;
            }
        }
        else if(windows == 3)
        {
            vec4 carRGB = texture(diffuseMap, TexCoord);
            fragColor = carRGB;
        }
        else if(windows == 5)
        {
            vec4 carRGB = texture(diffuseMap, TexCoord);
            float maxRGB = max(max(carRGB.r, carRGB.g), carRGB.b);
            float carMask = (carRGB.a > 0.001) ? 1.0 : ((maxRGB > 0.02) ? 1.0 : 0.0);
            if(carMask > 0.5 && maxRGB <= 0.01)
            {
                carRGB.r = 0.01;
                carRGB.g = 0.01;
                carRGB.b = 0.01;
            }
            fragColor = vec4(carRGB.rgb, carMask);
        }
    }
);
#pragma endregion
#if(GLAPI == VIV)

static const char fragmentShader_main[] = AVM_VALUE(
        precision lowp float;
        varying lowp vec2 TexCoordFront;
        varying lowp vec2 TexCoordBack;
        varying lowp vec2 TexCoordLeft;
        varying lowp vec2 TexCoordRight;
        varying lowp vec2 TexCoordAlpha;

        uniform sampler2D TextureFront;
        uniform sampler2D TextureBack;
        uniform sampler2D TextureLeft;
        uniform sampler2D TextureRight;

        uniform sampler2D SamplerU;
        uniform sampler2D SamplerV;
        uniform sampler2D SamplerUV;
        uniform sampler2D SamplerUV2;
        uniform sampler2D SamplerUV3;
        uniform sampler2D SamplerUV4;

        uniform sampler2D TextureAlpha;
        uniform sampler2D TextureColor;
        uniform sampler2D TextureColor2;

        uniform lowp vec3 rgbtotal_diff13;
        uniform lowp vec3 rgbtotal_diff24;
        uniform lowp vec3 rgbtotal_diff1_13;
        uniform lowp vec3 rgbtotal_diff2_24;
        uniform int windows;
        float y(vec3 rgb)
        {
            float yyyyy;
            yyyyy = rgb.r * 0.299+rgb.g*-0.169+rgb.b*0.5;
            return vec4(yyyyy, yyyyy, yyyyy,0.0);
        }
        void main(void)
        {
            float r, g, b,y,u,v;
            float r1, g1, b1,r2, g2, b2,r3, g3, b3,r4, g4, b4;
            float r_diff, g_diff, b_diff;
            float ratio = 0.0;

            lowp vec4 yuv;
            lowp vec4 rgbFront;
            lowp vec4 rgbBack;
            lowp vec4 rgbLeft;
            lowp vec4 rgbRight;
                
            if(windows == 0)
            {

                rgbFront = texture2D(TextureFront, TexCoordFront);
                rgbBack = texture2D(TextureBack, TexCoordBack);
                rgbLeft = texture2D(TextureLeft, TexCoordLeft);
                rgbRight = texture2D(TextureRight, TexCoordRight);
                
                ratio = texture2D(TextureAlpha, TexCoordAlpha).r;
                
                float color_r,color_g,color_b;
                float ratio_color;
                float ratio_color2;
                ratio_color = texture2D(TextureColor, TexCoordAlpha).r;
                ratio_color2 = texture2D(TextureColor2, TexCoordAlpha).r;
                float tmpr1,tmpg1,tmpb1;
                float part01 = 111.0/255.0;
                float part02 = 131.0/255.0;
                float part03 = 151.0/255.0;
                float part04 = 171.0/255.0;


                if(ratio_color <=part01)
                {

                    rgbLeft = rgbLeft - vec4(yyyyy, yyyyy, yyyyy,0.0);

                    rgbFront = vec4(0.0, 0.0, 0.0, 1.0);
                    rgbBack = vec4(0.0, 0.0, 0.0, 1.0);
                    rgbRight = vec4(0.0, 0.0, 0.0, 1.0);
                }
                else if(ratio_color <=part02)
                {
                    rgbLeft = rgbLeft - vec4(rgbtotal_diff13, 0.0);
                    
                    if(TexCoordAlpha.y < 0.5)
                    {
                        rgbFront = rgbFront + vec4(rgbtotal_diff1_13, 0.0);
                        rgbBack = vec4(0.0, 0.0, 0.0, 1.0);
                        rgbRight = vec4(0.0, 0.0, 0.0, 1.0);
                    }
                    else
                    {
                        rgbBack = rgbBack - vec4(rgbtotal_diff1_13, 0.0);
                        rgbFront = vec4(0.0, 0.0, 0.0, 1.0);
                        rgbRight = vec4(0.0, 0.0, 0.0, 1.0);
                    }
                }
                else if(ratio_color <=part03)
                {
                    r_diff = rgbtotal_diff1_13.r * (1.0 - ratio_color2) + rgbtotal_diff2_24.r * ratio_color2;
                    g_diff = rgbtotal_diff1_13.g * (1.0 - ratio_color2) + rgbtotal_diff2_24.g * ratio_color2;
                    b_diff = rgbtotal_diff1_13.b * (1.0 - ratio_color2) + rgbtotal_diff2_24.b * ratio_color2;
                    
                    //r_diff = rgbtotal_diff1_13.r * 0.5 + rgbtotal_diff2_24.r * 0.5;
                    //g_diff = rgbtotal_diff1_13.g * 0.5 + rgbtotal_diff2_24.g * 0.5;
                    //b_diff = rgbtotal_diff1_13.b * 0.5 + rgbtotal_diff2_24.b * 0.5;

                    if(TexCoordAlpha.y < 0.5)
                    {

                        rgbFront.r = rgbFront.r + r_diff;
                        rgbFront.g = rgbFront.g + g_diff;
                        rgbFront.b = rgbFront.b + b_diff;

                        rgbBack = vec4(0.0, 0.0, 0.0, 1.0);
                        rgbLeft = vec4(0.0, 0.0, 0.0, 1.0);
                        rgbRight = vec4(0.0, 0.0, 0.0, 1.0);
                    }
                    else
                    {
                        rgbBack.r = rgbBack.r - r_diff;
                        rgbBack.g = rgbBack.g - g_diff;
                        rgbBack.b = rgbBack.b - b_diff;
                        rgbFront = vec4(0.0, 0.0, 0.0, 1.0);
                        rgbLeft = vec4(0.0, 0.0, 0.0, 1.0);
                        rgbRight = vec4(0.0, 0.0, 0.0, 1.0);
                    }
                }
                else if(ratio_color <=part04)
                {
                    rgbRight = rgbRight - vec4(rgbtotal_diff24, 0.0);
                    if(TexCoordAlpha.y < 0.5)
                    {
                        rgbFront = rgbFront + vec4(rgbtotal_diff2_24, 0.0);
                        rgbBack = vec4(0.0, 0.0, 0.0, 1.0);
                        rgbLeft = vec4(0.0, 0.0, 0.0, 1.0);
                        
                    }
                    else
                    {
                        rgbBack = rgbBack - vec4(rgbtotal_diff2_24, 0.0);
                        rgbFront = vec4(0.0, 0.0, 0.0, 1.0);
                        rgbLeft = vec4(0.0, 0.0, 0.0, 1.0);
                    }
                }
                else
                {
                    rgbRight = rgbRight - vec4(rgbtotal_diff24, 0.0);
                    
                    rgbFront = vec4(0.0, 0.0, 0.0, 1.0);
                    rgbBack = vec4(0.0, 0.0, 0.0, 1.0);
                    rgbLeft = vec4(0.0, 0.0, 0.0, 1.0);
                }

                r =  mix((rgbFront.r + rgbBack.r), ((rgbLeft.r + rgbRight.r)), ratio);
                g =  mix((rgbFront.g + rgbBack.g), ((rgbLeft.g + rgbRight.g)), ratio);
                b =  mix((rgbFront.b + rgbBack.b), ((rgbLeft.b + rgbRight.b)), ratio);

                gl_FragColor = vec4(r, g, b, 1.0);
            }
            else if(windows == 1)
            {
                rgbFront = texture2D(TextureFront, TexCoordFront);
                gl_FragColor = rgbFront;
            }
            else if(windows == 2)
            {
                rgbBack = texture2D(TextureBack, TexCoordBack);
                gl_FragColor = rgbBack;
            }
            else if(windows == 3)
            {
                rgbLeft = texture2D(TextureLeft, TexCoordLeft);
                gl_FragColor = rgbLeft;
            }
            else if(windows == 4)
            {
                rgbRight = texture2D(TextureRight, TexCoordRight);
                gl_FragColor = rgbRight;
            }
            
        }
);
#pragma region Program_main_fb
static const char fragmentShader_main_fb[] = AVM_VALUE(#version 300 es\n
        precision lowp float;
        in lowp vec2 TexCoord;
        in lowp vec2 TexCoordAlpha;

        layout(location = 0) out vec4 outColor;
        // layout(location = 1) out vec4 outColorNoColor;

        uniform sampler2D Texture;
        uniform sampler2D TextureAlpha;
        uniform sampler2D TextureColor;
        // uniform lowp vec3 rgbtotal_diff1_13;
        // uniform lowp vec3 rgbtotal_diff2_24;

        uniform float diff_L;
        uniform float diff_R;
        uniform float avg_L;
        uniform float avg_R;

        uniform int windows;
        uniform vec4 color;
        uniform vec2 startTexCoord ;
        uniform vec2 endTexCoord;

        uniform float gamma_L;
        uniform float gamma_R;
        uniform float gamma_angle;
        uniform vec3 opendoorColor;

        float calcGammaAngle(float l_gamma, float r_gamma, float ratio_color2, vec2 uv, float deg)
        {
            float rad = radians(-deg);               
            vec2  c   = vec2(0.5, 0.5);
            vec2  p   = uv - c;

            mat2 R = mat2( cos(rad), -sin(rad),
                            sin(rad),  cos(rad) );

            vec2 ur = R * p + c;                     

            float xFalloff = pow(abs(ur.x - 0.5) * 2.0, 3.0);
            float yFalloff = 1.0 - ur.y;
            float compensation = xFalloff * yFalloff;
            compensation *= (l_gamma * (1.0 - ratio_color2) + r_gamma * ratio_color2);
            return clamp(compensation, 0.0, 1.0);   
        }
        float yrgb(vec3 rgb)
        {
            float y;
            y = rgb.r * 0.299+rgb.g*-0.169+rgb.b*0.5;
            return y;
        }
        float calColor(float TexY, float avg, float diff)
        {
            float outY;
            if(TexY > avg)
            {
                outY = 1.0 * diff * (1.0 - TexY) / (1.0 - avg);
            }
            else
            {
                outY = 1.0 * diff * (TexY) / (avg);
            }
            return outY;
        }
        float calcGamma(float l_gamma, float r_gamma, float ratio_color2, vec2 uv)
        {
            float xFalloff = pow(abs(uv.x - 0.5) * 2.0, 3.0);
            float yFalloff = 1.0 - uv.y;
            float compensation = xFalloff * yFalloff;
            compensation *= (l_gamma * (1.0 - ratio_color2) + r_gamma * ratio_color2);
            return clamp(compensation, 0.0, 1.0);   
        }

        void main(void)
        {
            float r_diff;
            float g_diff;
            float b_diff;
            float ratio;
            float ratio_color2;
            lowp vec4 rgbTex,yuvTex;
            vec2 ScopeTexCoord;
            if(windows == 0)
            {
                #if 1
                rgbTex = texture(Texture, TexCoord);
                ratio = 1.0-(texture(TextureAlpha, TexCoordAlpha).y);
                ratio_color2 = texture(TextureColor, TexCoordAlpha).r;
                //float gamma_component = calcGamma(gamma_L, gamma_R, ratio_color2, TexCoord);
                float gamma_component = calcGammaAngle(gamma_L, gamma_R, ratio_color2, TexCoord, gamma_angle);
                float yVal = yrgb(rgbTex.rgb);
                float lefty  = calColor(yVal, avg_L, diff_L);
                float righty = calColor(yVal, avg_R, diff_R);
                float totaly = lefty * (1.0 - ratio_color2) + righty * ratio_color2;
                rgbTex.rgb += vec3(totaly + gamma_component);
                outColor = vec4(rgbTex.rgb, ratio);
                #else
                outColor = texture(Texture, TexCoord);
                #endif
            }
            else if(windows == 1)
            {
                ScopeTexCoord = TexCoord * (endTexCoord - startTexCoord) + startTexCoord;
                rgbTex = texture(Texture, ScopeTexCoord);
                outColor = rgbTex;    
            }
            else if(windows == 2)
            {
                rgbTex = texture(Texture, TexCoord);
                outColor = rgbTex;   
            }
            else if(windows == 3)
            {
                ratio = 1.0-(texture(TextureAlpha, TexCoordAlpha).y);
                outColor = vec4(opendoorColor, ratio);
                //outColor = vec4(0.196,0.196,0.196,ratio);
            }
        }
);
#pragma endregion

#pragma region Program_main_lr
static const char fragmentShader_main_lr[] = AVM_VALUE(#version 300 es\n
        precision lowp float;
        in lowp vec2 TexCoord;
        in lowp vec2 TexCoordAlpha;
        layout(location = 0) out vec4 outColor;
        // layout(location = 1) out vec4 outColorNoColor;

        uniform sampler2D Texture;
        uniform sampler2D TextureColor;
        //uniform lowp vec3 rgbtotal_diffLR;

        uniform float diff;
        uniform float avg;

        uniform int windows;
        uniform vec4 color;
        uniform vec2 startTexCoord ;
        uniform vec2 endTexCoord;

        uniform float tip_homo_top;

        uniform int version;
        uniform float gamma_L;
        uniform float gamma_R;
        uniform float gamma_angle;
        uniform float diff_F;
        uniform float diff_B;
        uniform vec3 opendoorColor;
        float yrgb(vec3 rgb)
        {
            float y;
            y = rgb.r * 0.299+rgb.g*-0.169+rgb.b*0.5;
            return y;
        }

        float calColor(float TexY, float avg, float diff)
        {
            float outY;
            if(TexY > avg)
            {
                outY = 1.0 * diff * (1.0 - TexY) / (1.0 - avg);
            }
            else
            {
                outY = 1.0 * diff * (TexY) / (avg);
            }
            return outY;
        }


        float calcGamma(float l_gamma, float r_gamma, float ratio_color2, vec2 uv)
        {
            float xFalloff = pow(abs(uv.x - 0.5) * 2.0, 3.0);
            float yFalloff = 1.0 - uv.y;
            float compensation = xFalloff * yFalloff;
            compensation *= (l_gamma * (1.0 - ratio_color2) + r_gamma * ratio_color2);
            return clamp(compensation, 0.0, 1.0);   
        }
        float calcGammaAngle(float l_gamma, float r_gamma, float ratio_color2, vec2 uv, float deg)
        {
            float rad = radians(-deg);               
            vec2  c   = vec2(0.5, 0.5);
            vec2  p   = uv - c;

            mat2 R = mat2( cos(rad), -sin(rad),
                            sin(rad),  cos(rad) );

            vec2 ur = R * p + c;                     

            float xFalloff = pow(abs(ur.x - 0.5) * 2.0, 3.0);
            float yFalloff = 1.0 - ur.y;
            float compensation = xFalloff * yFalloff;
            compensation *= (l_gamma * (1.0 - ratio_color2) + r_gamma * ratio_color2);
            return clamp(compensation, 0.0, 1.0);   
        }

        float homogen(float y)
        {
            float value = tip_homo_top;
            float newy = 2.0* y - 1.0;
            return 0.5 * value * (1.0 - newy * newy);
        }

        void main(void)
        {
            lowp vec4 rgbTex,yuvTex;
            vec2 ScopeTexCoord;
            float ratio_color2;
            float totaly ;
            if(windows == 0)
            {
                rgbTex               = texture(Texture, TexCoord);
                if(version == 0)
                {
                    float gamma      = 0.0;
                    float homogenization = homogen(TexCoord.x);
                    float diff_y         = calColor(yrgb(rgbTex.rgb) + gamma, avg, diff) - homogenization;
                    rgbTex               = rgbTex - vec4(diff_y, diff_y, diff_y, 0.0);
                }
                else
                {
                    ratio_color2 = texture(TextureColor, TexCoordAlpha).r;
                    //float gamma_component = calcGamma(gamma_L, gamma_R, ratio_color2, TexCoord);                  // //gamma_component = 0.0;
                    float gamma_component = calcGammaAngle(gamma_L, gamma_R, ratio_color2, TexCoord, gamma_angle);
                    float lefty  = diff_F;
                    float righty = diff_B;
                    totaly = lefty * (1.0 - ratio_color2) + righty * (ratio_color2);
                    rgbTex = rgbTex + vec4(totaly + gamma_component, totaly + gamma_component, totaly + gamma_component, 0.0);
                }
                outColor             = vec4(rgbTex.rgb, 1.0);



            }
            else if(windows == 1)
            {
                ScopeTexCoord = TexCoord * (endTexCoord - startTexCoord) + startTexCoord;
                rgbTex = texture(Texture, ScopeTexCoord);
                outColor = rgbTex;   
                // outColorNoColor = rgbTex;   
            }
            else if(windows == 2)
            {
                rgbTex = texture(Texture, TexCoord);
                outColor = rgbTex;   
                // outColorNoColor = rgbTex;   
            }
            else if(windows == 3)
            {
                outColor = vec4(opendoorColor, 1.0);
            }
        
        }
);
#pragma endregion

/** 
 *             @"  #version 300 es
                precision lowp float;
                out vec4 fragColor;

                in lowp vec2 TexCoord;
                in float TexCoordAlpha;
                uniform vec3 color;
                uniform sampler2D Texture;
                uniform int windows;

                void main(void)
                {
                    lowp vec4 rgbTex;
                    if(windows == 0) //default
                    {
                        fragColor = vec4(color, TexCoordAlpha);
                    }
                    else if(windows == 1)
                    {
                        rgbTex = texture(Texture, TexCoord);
                        fragColor = vec4(rgbTex.rgb, TexCoordAlpha);
                    }
                }
            ";
*/


#elif(GLAPI == UYVY || GLAPI == YUYV)

#endif
// test_shadow
static const char fragmentShader_car_shadow[] = AVM_VALUE(#version 300 es\n
        precision highp float;
        in highp float TexCoordAlpha;
        in highp vec2 TexCoord;

        uniform int enable_status;

        out vec4 outColor;

        uniform vec3 color;
        uniform sampler2D Texture;
        uniform sampler2D seethrough_frame;
        uniform sampler2D Texture_adas_ims;
        uniform int windows;

        
        uniform sampler2D ui_enable_Texture;
        uniform sampler2D ui_disable_Texture;
        uniform int flag_blur;
        uniform vec2 startTexCoord ;
        uniform vec2 endTexCoord;
        uniform int baseColorFlag;
        void main(void)
        {
            highp vec4 rgbTex;
            float alpha = TexCoordAlpha;
            if(flag_blur == 0) 
            {
                if(TexCoordAlpha < 0.9f) alpha = 0.0f;
            }

            if(windows == 0) //default
            {
                
                outColor = vec4(color * alpha, alpha);
                //outColor = vec4(color, 0.8);
            }
            else if(windows == 1)
            {
                vec2 ScopeTexCoord;
                ScopeTexCoord = TexCoord * (endTexCoord - startTexCoord) + startTexCoord;
                rgbTex = texture(Texture, ScopeTexCoord);
                outColor = vec4(rgbTex.rgb, TexCoordAlpha);
                    
            }
            else if(windows == 2)
            {
                vec2 ScopeTexCoord;
                ScopeTexCoord = TexCoord * (endTexCoord - startTexCoord) + startTexCoord;
                rgbTex = texture(Texture, ScopeTexCoord);
                outColor = rgbTex;
            }
            else if(windows == 3)
            {
                vec2 ScopeTexCoord;
                ScopeTexCoord = TexCoord * (endTexCoord - startTexCoord) + startTexCoord;
                rgbTex = texture(Texture, ScopeTexCoord);
                lowp vec4 frame = texture(seethrough_frame, TexCoord);
                vec3 finalColor;
                if(frame.a >=0.5 && frame.a <= 0.8 && frame.r == 1.0)
                {
                    outColor = vec4(0.0, 0.0,0.0,0.0);
                }
                else
                {
                    finalColor = mix(color,rgbTex.rgb, 1.0 - frame.a);
                    outColor = vec4(finalColor, TexCoordAlpha);
                }
                //outColor = texture(Texture, TexCoord);
            }
            else if(windows == 4)
            {
                if(enable_status == 1)
                {
                    rgbTex = texture(ui_enable_Texture, TexCoord);
                }
                else
                {
                    rgbTex = texture(ui_disable_Texture, TexCoord);
                }
                outColor = vec4(rgbTex.bgr, 1.0);
            }
            // outColor = vec4(color, TexCoordAlpha);
            //outColor = vec4(1.0,0.0,0.0, TexCoordAlpha);
            //outColor = vec4( color.rgb,  alpha);
        }
);

static const char fragmentShader_rgba[] = AVM_VALUE(
        varying lowp vec2 texCoord;
        uniform sampler2D TextureRgba;

        void main(void)
        {
            gl_FragColor = texture2D(TextureRgba, texCoord);
//            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
);

static const char fragmentShader_line[] = AVM_VALUE(
        precision highp float;
        uniform vec4 color;
        varying highp vec3 positionSh;
        varying highp vec2 texCoord;
        uniform sampler2D colorbar;
        uniform float y_limit;
        uniform int windows;
        void main(void)
        {
            if(windows == 0)
            {
                gl_FragColor = color;
            }
            else if(windows == 2)
            {
                lowp vec4 rgbTex;
                rgbTex = texture2D(colorbar, texCoord);
                gl_FragColor = vec4(rgbTex.rgba);
            }
            else if(windows == 3)
            {
                lowp vec4 rgbTex;
                rgbTex = texture2D(colorbar, texCoord);
                if(positionSh.y >= y_limit)
                {
                    gl_FragColor = vec4(rgbTex.rgba);
                }
                else
                {
                    gl_FragColor = vec4(0,0,0,0);
                }
            }
            else
            {
                gl_FragColor = color;
            }
        }
);

static const char fragmentShader_blur[] = AVM_VALUE(#version 300 es\n
    precision highp float;
    in vec2 TexCoord;
    out vec4 fragColor;

    uniform int GAUSSIAN_SAMPLES;
    uniform sampler2D Texture;
    uniform int horizontal;
    uniform float gaussian_weight[9];
    uniform vec2 tex_offset;
    uniform vec4 MaskArea;
    void main(void)
    {
        vec3 result= vec3(1.0, 0.0, 0.0);
        if(GAUSSIAN_SAMPLES == 0)
        {
            result    = texture(Texture, TexCoord).rgb;
            fragColor = vec4(result, 1.0);
        }
        else
        {
            vec2 offsetX = vec2(1.0 / float(textureSize(Texture, 0).x), 0.0);
            vec2 offsetY = vec2(0.0, 1.0 / float(textureSize(Texture, 0).y));
            bool flag1 = (TexCoord.x >= MaskArea.x && TexCoord.x <= MaskArea.z)?true:false;
            bool flag2 = (TexCoord.y >= MaskArea.y && TexCoord.y <= MaskArea.w)?true:false;
            if(flag1 && flag2)
            //if(true)
            {
                result = texture(Texture, TexCoord).rgb * gaussian_weight[0]; 
                if(horizontal == 0)
                {
                    for(int i = 1; i < GAUSSIAN_SAMPLES; ++i)
                    {
                        result += texture(Texture, TexCoord + vec2(tex_offset.x * float(i), 0.0)).rgb * gaussian_weight[i];
                        result += texture(Texture, TexCoord - vec2(tex_offset.x * float(i), 0.0)).rgb * gaussian_weight[i];
                    }
                    //result = vec3(1.0, 0.0, 0.0);
                }
                else
                {
                    for(int i = 1; i < GAUSSIAN_SAMPLES; ++i)
                    {
                        result += texture(Texture, TexCoord + vec2(0.0, tex_offset.y * float(i))).rgb * gaussian_weight[i];
                        result += texture(Texture, TexCoord - vec2(0.0, tex_offset.y * float(i))).rgb * gaussian_weight[i];
                    }
                    //result = vec3(1.0, 0.0, 0.0);
                }
            }
            else
            {
                result = texture(Texture, TexCoord).rgb;
                //result = vec3(1.0, 1.0, 0.0);
            }

            fragColor = vec4(result, 1.0);
        }
    }
);

static const char fragmentShader_mod[] = AVM_VALUE(
        precision mediump float;

        varying mediump vec3 positionSh;
        varying mediump vec2 TexCoord;
        varying float alphaSh;
        uniform vec4 RectMask;
        uniform vec4 color;
        uniform int type;
        uniform int windows;
        uniform float UV_Ratio;
        void main(void)
        {
            float cur_target = 0.0;
            float maxarea = 0.0;
            float alphaSh_tmp = 0.0;
            if(windows == 0)
            {
                gl_FragColor = color;
            }
            else
            {
                if(type == 0)
                {
                    maxarea    = RectMask.z;
                    cur_target = TexCoord.y;
                    if(cur_target < maxarea)
                    {
                        
                        alphaSh_tmp = (maxarea - cur_target) * 5.0 *  UV_Ratio ;
                    }
                    else
                    {
                        alphaSh_tmp = 0.0;
                    }
                    gl_FragColor = vec4(color.rgb, alphaSh_tmp);
                }
                else if(type == 1)
                {
                    maxarea    = RectMask.w;
                    cur_target = TexCoord.y;
                    if(cur_target > maxarea)
                    {
                        
                        alphaSh_tmp = (cur_target - maxarea) * 5.0 *  UV_Ratio;
                    }
                    else
                    {
                        alphaSh_tmp = 0.0;
                    }
                    gl_FragColor = vec4(color.rgb, alphaSh_tmp);
                }
                else if(type == 2)
                {
                    maxarea = RectMask.x;
                    cur_target = TexCoord.x;
                    if(cur_target < maxarea)
                    {
                        
                        alphaSh_tmp = (maxarea - cur_target) * 5.0;
                    }
                    else
                    {
                        alphaSh_tmp = 0.0;
                    }
                    gl_FragColor = vec4(color.rgb, alphaSh_tmp);
                }
                else if(type == 3)
                {
                    maxarea = RectMask.y;
                    cur_target = TexCoord.x;
                    if(cur_target > maxarea)
                    {
                        
                        alphaSh_tmp = (cur_target - maxarea) * 5.0;
                    }
                    else
                    {
                        alphaSh_tmp = 0.0;
                    }
                    gl_FragColor = vec4(color.rgb, alphaSh_tmp);
                }
            }
        }
);

static const char fragmentShader_uyvy[] = AVM_VALUE(
        precision lowp float;
        varying lowp vec2 texCoord;
        uniform sampler2D TextureRgba;
        
        void main(void)
        {
            float u_Offset;
            u_Offset = 1.0 / 768.0;
            lowp vec4 rgbalisom;
            lowp vec4 rgbalisom_shift;
            lowp vec3 COEF_Y = vec3( 0.299,  0.587,  0.114);
            lowp vec3 COEF_U = vec3(-0.147, -0.289,  0.436);
            lowp vec3 COEF_V = vec3( 0.615, -0.515, -0.100);
            lowp vec2 texelOffset = vec2(u_Offset, 0.0);
            lowp vec2 texCoord_sh;
            texCoord_sh.x = texCoord.x * 2.0;
            texCoord_sh.y = texCoord.y;
            rgbalisom = texture2D(TextureRgba, texCoord_sh);
            rgbalisom_shift = texture2D(TextureRgba, texCoord_sh + texelOffset);
            float y0 = dot(rgbalisom.rgb, COEF_Y);
            float u0 = dot(rgbalisom.rgb, COEF_U) + 0.5;
            float v0 = dot(rgbalisom.rgb, COEF_V) + 0.5;
            float y1 = dot(rgbalisom_shift.rgb, COEF_Y);
            //gl_FragColor = texture2D(TextureRgba, texCoord);
            //gl_FragColor = vec4(rgbalisom.r, rgbalisom.g, rgbalisom.b, rgbalisom.a);
            gl_FragColor = vec4(u0, y0, v0, y1);
            //gl_FragColor = vec4(0.5, 0.5, 0.5, 0.5);
        }
);

static const char fragmentShader_see_through[] = AVM_VALUE(
        precision lowp float;
        varying lowp vec4 v_vertexCoord;
        varying lowp vec2 v_texCoord;
        varying vec3 v_Position;
        uniform sampler2D u_2davm;
        // uniform sampler2D u_seeThroughImage;
        uniform vec3 u_seeThroughArea[2];
        uniform mat4 u_transformationMatrix2;

        uniform float sigmoid_alpha;
        uniform float top2d_alpha_center;
        uniform float top2d_alpha_range;

        void main(void)
        {
            float alpha;
            bool area1;
            bool area2;
            alpha = 0.0;
            area1 = false;
            area2 = false;
            // vec4 seeThrough = texture2D(u_2davm, v_texCoord);
            // vec4 seeThrough = vec4(1.0, 1.0, 0.0, 1.0);
            vec4 avm2d = texture2D(u_2davm, v_texCoord);
 
            
            vec4 test1 = u_transformationMatrix2*vec4(u_seeThroughArea[0], 1.0);
            vec4 test2 = u_transformationMatrix2*vec4(u_seeThroughArea[1], 1.0);
            area1 = (v_vertexCoord.x >= test1.x)&&(v_vertexCoord.y <= test1.y);
            area2 = (v_vertexCoord.x <= test2.x)&&(v_vertexCoord.y >= test2.y);



            bool area1_front;
            bool area2_front;
            area1_front = false;
            area2_front = false;
            vec2 test = v_texCoord*2.0-1.0;

            float blendArea = top2d_alpha_range;//0.307692 ; // 30
            //float start = 0.6153846;      // 80
            float start = top2d_alpha_center;//0.53846;
            vec3 leftUp = u_seeThroughArea[0] - vec3(0.0, 0.0, start) + vec3(0.0, 0.0, -blendArea/2.0);
            vec3 rightDown = vec3(-u_seeThroughArea[0].x, u_seeThroughArea[0].y, u_seeThroughArea[0].z)-vec3(0.0, 0.0, start) + vec3(0.0, 0.0,blendArea/2.0);
            // vec3 leftUp = u_seeThroughArea[0] - vec3(0.0, 0.0, start) + vec3(0.0, 0.0, blendArea);;
            // vec3 rightDown = vec3(-u_seeThroughArea[0].x, u_seeThroughArea[0].y, u_seeThroughArea[0].z)-vec3(0.0, 0.0, start);
            
            vec4 test1_front = u_transformationMatrix2*vec4(leftUp, 1.0);
            vec4 test2_front = u_transformationMatrix2*vec4(rightDown, 1.0);
            area1_front = (test.x >= test1_front.x)&&(test.y <= test1_front.y);
            area2_front = (test.x <= test2_front.x)&&(test.y >= test2_front.y);
    



           if(area1_front&&area2_front)
            {
                alpha = 1.0 / ( 1.0 + exp(-sigmoid_alpha*( (v_Position.z-leftUp.z)/(blendArea)-blendArea )) );
            }
            else if(area1&&area2)
            {
                if(avm2d.a <= 0.0)
                {
                    alpha = avm2d.a;
                    //alpha = 1.0;
                }
                else
                {
                    alpha = 1.0;
                }
            }
            else
            {
                alpha = 0.0;
            }
            // result = seeThrough * alpha + avm2d * (1.0 - alpha);
            gl_FragColor = vec4(avm2d.rgb, alpha);
            // gl_FragColor = -test1;
                
        }
);

static const char fragmentShader_see_through_2davm[] = AVM_VALUE(
        precision lowp float;
        varying lowp vec2 v_texCoord;
        uniform sampler2D u_seeThroughImage;

        void main(void)
        {
            vec4 seeThrough = texture2D(u_seeThroughImage, v_texCoord);
            gl_FragColor = seeThrough;
        }
);

static const char fragmentShader_y[] = AVM_VALUE(
        precision lowp float;

        varying lowp vec2 v_texture;
        uniform sampler2D u_video;
        uniform float windows;
        float yrgb(vec3 rgb)
        {
            float y;
            y = rgb.r * 0.299+rgb.g*+0.587+rgb.b*0.114;
            return y;
        }
        void main()
        {
            vec4 frag_pixel[4];
            mat4 rgb2yuv = mat4(0.299, -0.169,  0.500, 0.000,
                                0.587, -0.331, -0.419, 0.000, 
                                0.114,  0.500, -0.081, 0.000, 
                                0.000,  0.500,  0.500, 1.000);

            lowp vec2 tc = v_texture;

            tc.x = tc.x*4.0;            
            lowp vec2 tc0 = tc;
            lowp vec2 tc1 = tc;
            lowp vec2 tc2 = tc;
            lowp vec2 tc3 = tc;
            
            tc0.x = tc.x  + windows;
            tc1.x = tc0.x + windows;
            tc2.x = tc1.x + windows;
            tc3.x = tc2.x + windows;
            
            float p1 = 0.0;
            float p2 = 0.0;
            float p3 = 0.0;
            float p4 = 0.0;
            if(tc0.x <= 1.1)
            {
                p1 = yrgb(texture2D(u_video, tc0).rgb);
            }
            if(tc1.x <= 1.1)
            {
                p2 = yrgb(texture2D(u_video, tc1).rgb);
            }
            if(tc2.x <= 1.1)
            {
                p3 = yrgb(texture2D(u_video, tc2).rgb);
            }
            if(tc3.x <= 1.1)
            {
                p4 = yrgb(texture2D(u_video, tc3).rgb);
            }
            vec4 frag_result;
            frag_result = vec4(p1, p2, p3, p4);
            gl_FragColor = frag_result;
            
        }
);



static const char fragmentShader_FXAA[] = AVM_VALUE(#version 300 es\n

    precision lowp float;
    in lowp vec2 TexCoord;
    //uniform sampler2D EdgeTexture; 
    //uniform sampler2D BlurTexture; 
    uniform sampler2D RAWImage;
    uniform float screenwidth;
    uniform float screenheight;
    uniform float edgeThreshold;
    uniform int process;
    uniform int EnableFxaa;
    uniform lowp vec2 dir;
    //out vec4 fragColor;
    out vec4 outColor;

    // bool isEdge(vec2 coord) 
    // {
    //     vec4 centerColor = texture(RAWImage, coord);
    //     vec4 leftColor =   texture(RAWImage, coord + vec2(-1.0f/screenwidth, 0.0f));
    //     vec4 rightColor =  texture(RAWImage, coord + vec2(1.0f/screenwidth, 0.0f));
    //     vec4 upColor =     texture(RAWImage, coord + vec2(0.0f, -1.0f/screenheight));
    //     vec4 downColor =   texture(RAWImage, coord + vec2(0.0f, 1.0f/screenheight));
       
    //     float diff = length(centerColor - leftColor) + length(centerColor - rightColor) +
    //                  length(centerColor - upColor) + length(centerColor - downColor);
        
    //     return diff > edgeThreshold;
    // }

    vec4 FXAA_Filter_2(sampler2D image, vec2 uv, float u_lumaThreshold, float u_maxSpan, vec2 u_texelStep ,float u_minReduce, float u_mulReduce)
    {
                    
        vec3 rgbM = texture(image, uv).rgb;
	    // Sampling neighbour texels. Offsets are adapted to OpenGL texture coordinates. 
        vec3 rgbNW = texture(image, uv + vec2(-u_texelStep.x, u_texelStep.y)).rgb;
        vec3 rgbNE = texture(image, uv + vec2( u_texelStep.x, u_texelStep.y)).rgb;
        vec3 rgbSW = texture(image, uv + vec2(-u_texelStep.x,-u_texelStep.y)).rgb;
        vec3 rgbSE = texture(image, uv + vec2( u_texelStep.x,-u_texelStep.y)).rgb;

        const vec3 toLuma = vec3(0.299f, 0.587f, 0.114f);
	    // // Convert from RGB to luma.
	    float lumaNW = dot(rgbNW, toLuma);
	    float lumaNE = dot(rgbNE, toLuma);
	    float lumaSW = dot(rgbSW, toLuma);
	    float lumaSE = dot(rgbSE, toLuma);
	    float lumaM = dot(rgbM, toLuma);
        // Gather minimum and maximum luma.
	    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
        
	    
	    // If contrast is lower than a maximum threshold ...
	    if (lumaMax - lumaMin <= lumaMax * u_lumaThreshold)
	    {
	        return vec4(rgbM, 1.0f);
	        //return texture(image, uv);
	    } 
        vec2 samplingDirection;	
	    samplingDirection.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
        samplingDirection.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

        float samplingDirectionReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25f * u_mulReduce, u_minReduce);
        float minSamplingDirectionFactor = 1.0f / (min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);
        samplingDirection = clamp(samplingDirection * minSamplingDirectionFactor, vec2(-u_maxSpan), vec2(u_maxSpan)) * u_texelStep;

	    vec3 rgbSampleNeg = texture(image, uv + samplingDirection * (1.0f/3.0f - 0.5f)).rgb;
	    vec3 rgbSamplePos = texture(image, uv + samplingDirection * (2.0f/3.0f - 0.5f)).rgb;

        vec3 rgbTwoTab = (rgbSamplePos + rgbSampleNeg) * 0.5f;  
	    // Outer samples on the tab.
	    vec3 rgbSampleNegOuter = texture(image, uv + samplingDirection * (0.0f/3.0f - 0.5f)).rgb;
	    vec3 rgbSamplePosOuter = texture(image, uv + samplingDirection * (3.0f/3.0f - 0.5f)).rgb;
        vec3 rgbFourTab = (rgbSamplePosOuter + rgbSampleNegOuter) * 0.25f + rgbTwoTab * 0.5f;   
        
        float lumaFourTab = dot(rgbFourTab, toLuma);

        float NWAlpha = texture(image, uv + vec2(-u_texelStep.x, u_texelStep.y)).a;
        float NEAlpha = texture(image, uv + vec2( u_texelStep.x, u_texelStep.y)).a;
        float SWAlpha = texture(image, uv + vec2(-u_texelStep.x,-u_texelStep.y)).a;
        float SEAlpha = texture(image, uv + vec2( u_texelStep.x,-u_texelStep.y)).a;

        float alpha = max(max(NWAlpha,NEAlpha),max(SWAlpha,SEAlpha));

	    // Are outer samples of the tab beyond the edge ... 
	    if (lumaFourTab < lumaMin || lumaFourTab > lumaMax)
	    {	    	
	    	return vec4(rgbTwoTab, 1.0f); 
	    }
	    else
	    {
	    	return vec4(rgbFourTab, 1.0f);
	    }
                
    }
                

    void main(void)
    {
        if(process == 0)
        {
            // bool edge = isEdge(OutexCoord);
            // if (edge) 
            // {
            //      outColor = texture(RAWImage, OutexCoord);
            // } 
            // else 
            // {
            //     outColor = vec4(0.0f,0.0f,0.0f,0.0f);
            // }
            outColor = texture(RAWImage, TexCoord);
        }
        else if(process == 1)
        {
	        //float hstep = dir.x;
	        //float vstep = dir.y;
            if(EnableFxaa == 1 && TexCoord.y > 0.3)
            {
                vec2 img_texelStep = vec2(1.0f/screenwidth, 1.0f/screenheight);
                outColor = FXAA_Filter_2(RAWImage, TexCoord, edgeThreshold,8.0f,img_texelStep, 0.1f, 0.06f);
            }
            else
            {
                outColor = texture(RAWImage, TexCoord);
                //outColor = vec4(1.0f,0.0f,0.0f,1.0f);
            }
            //outColor = texture(RAWImage, TexCoord);
        }
        else
        {
            outColor = texture(RAWImage,TexCoord);
            //outColor = vec4(1.0f,0.0f,0.0f,1.0f);
        }
    }


);
