/**
 *******************************************************************************
 * @file : glShaderUtils.c
 * @describe : Create shader and program.
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

#include "../../system.h"
#include "gl/glShaderUtils.h"
#include "avm/GLHeader.h"
#include <stdlib.h>
#include "../../../setup/AVM_STRUCT.h"
#include "avm/fp_source.h"
#include "avm/stb_image.h"
#include <sys/stat.h>
#include <unistd.h>

int create_shader(int shaderType, const char shader[]) {
    GLint isShaderCompiled;
    GLuint shaderValue = glCreateShader(shaderType);

    // 檢查 shader是否創建成功。失敗為0
    if (!shaderValue) {
        AVM_LOGI("glCreateShader failure type =  %d, value = %d", shaderType, shaderValue);
        return 0;
    }
    glShaderSource(shaderValue, 1, &shader, NULL);
    glCompileShader(shaderValue);

#if 0
    glGetShaderiv(shaderValue, GL_COMPILE_STATUS, &isShaderCompiled);
    
    if (isShaderCompiled != GL_TRUE) {
        AVM_LOGI("---------- CompileShader: failed ----------\n");

        // 检查GL日志中是否有log，没有则为0(log长度为0)；有值则代表有错误信息(log长度不为0)
        GLint infoLength;
        glGetShaderiv(shaderValue, GL_INFO_LOG_LENGTH, &infoLength);

        if (infoLength) {
            char *buf = (char *) malloc(infoLength + 1);
            glGetShaderInfoLog(shaderValue, infoLength, NULL, buf);
            AVM_LOGI("shader info log shader = %d buf = %s\n", shaderValue, buf);
            free(buf);
        }
        return 0; // 或其他适当的错误处理
    }
#endif

    return shaderValue;
}
void LoadBinary2Program(GLuint *pProgram, char *binFilePath) 
{
    FILE *fp = fp_source_app(binFilePath, "rb");
    //printf("BinaryProgramExample LoadBinary2Program fp=%p, file=%s\n", fp, binFilePath);
    if(fp) {
        char version[100] = {0};
        fread(version, sizeof(char), 14, fp);
        AVM_LOGI("[shader version] %s\n", version);
        fseek(fp, 0L, SEEK_END);
        int size = ftell(fp);
        rewind(fp);
        fread(version, sizeof(char), 14, fp);
        unsigned char *buffer = (unsigned char *) malloc(sizeof(unsigned char) * (size-14));
        
        fread(buffer, (size-14), 1, fp);
        fclose(fp);

        *pProgram = glCreateProgram();
        // std::string path(binFilePath);
        // int pos = path.find("format_");
        // std::string strFormat = path.substr(pos + 7);
        int format =36805;
        // LOGCATE("BinaryProgramExample format=%d", format);
        glProgramBinary(*pProgram, format, buffer, (size-14));

        //检查是否加载成功
        GLint status;
        glGetProgramiv(*pProgram, GL_LINK_STATUS, &status);
        if(GL_FALSE == status) {
            //加载失败
            printf("BinaryProgramExample glProgramBinary fail.\n");
            glDeleteProgram(*pProgram);
            *pProgram = GL_NONE;
        }

        // delete[] buffer;
        free(buffer);
    }
    else
    {
        *pProgram = GL_NONE;
    }
}
int create_program(const char v[], const char f[],int program_count)
{
    // AVM_LOGI("createProgram v = %s, f = %s\n", v, f);
    int vetexShader = create_shader(GL_VERTEX_SHADER, v);
    if(!vetexShader)
    {
         AVM_LOGI("createProgram GL_VERTEX_SHADER vetexShader = %d\n", vetexShader);
        return 0;
    }

    int fragShader = create_shader(GL_FRAGMENT_SHADER, f);
    if(!fragShader)
    {
         AVM_LOGI("create_shader GL_FRAG_SHADER fragShader = %d\n", fragShader);
        return 0;
    }

    int program = glCreateProgram();
    glAttachShader(program, vetexShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

#if 0
    //===============off-line compile shader code ==================
    GLint binary_size = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binary_size);

    // Allocate some memory to store the program binary
    unsigned char *program_binary = (unsigned char *)malloc(sizeof(unsigned char) * binary_size);// Now retrieve the binary from the program object
    GLenum binary_format = GL_NONE;
    glGetProgramBinary(program, binary_size, NULL, &binary_format, program_binary);

    printf("binary_size = %d\n", binary_size);

    char binaryFilePath[256] = {0};
    sprintf(binaryFilePath, "./shader_program.bin_format_%d_%d", binary_format, program_count);
    FILE *fp = fopen(binaryFilePath, "wb");
    // LOGCATE("BinaryProgramExample fp=%p, file=%s, binFormat=%d", fp, binaryFilePath, format);
    if (fp)
    {
        fwrite(program_binary, binary_size, 1, fp);
        fclose(fp);
    }
    //===============off-line compile shader code ==================
#endif
    return program;
}

void write_offline_shader_program(int program, int program_count)
{
    //===============off-line compile shader code ==================
    glLinkProgram(program);
    GLint binary_size = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binary_size);

    // Allocate some memory to store the program binary
    unsigned char *program_binary = (unsigned char *)malloc(sizeof(unsigned char) * binary_size);// Now retrieve the binary from the program object
    GLenum binary_format = GL_NONE;
    glGetProgramBinary(program, binary_size, NULL, &binary_format, program_binary);

    printf("progarm_binary_size = %d\n", binary_size);

    char binaryFilePath[256] = {0};
    sprintf(binaryFilePath, "./shader_program.bin_format_%d_%d", binary_format, program_count);
    FILE *fp = fopen(binaryFilePath, "wb");
    //LOGCATE("BinaryProgramExample fp=%p, file=%s, binFormat=%d", fp, binaryFilePath, format);
    if (fp)
    {
        fwrite(program_binary, binary_size, 1, fp);
        fclose(fp);
    }
    
    //===============off-line compile shader code ==================
}

void destroy_shaders(GLuint *vertShaderNum, GLuint *pixelShaderNum, GLuint *programHandle)
{
    glDeleteShader(*vertShaderNum);
    glDeleteShader(*pixelShaderNum);
    glDeleteProgram(*programHandle);
    glUseProgram(0);
}

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    AVM_LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
                                                    = glGetError()) {
        AVM_LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

