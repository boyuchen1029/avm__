/**
 *******************************************************************************
 * @file : glShaderUtils.h
 * @describe : Header of glShaderUtils.c.
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20211220 0.1.0 Linda.
 *******************************************************************************
*/
#ifndef _GLSHADERUTILS_H_
#define _GLSHADERUTILS_H_

#include "avm/GLHeader.h"

int create_shader(int shaderType, const char shader[]);

int create_program(const char v[], const char f[],int program_count);
void LoadBinary2Program(GLuint *pProgram, char *binFilePath);
void destroy_shaders(GLuint *vertShaderNum, GLuint *pixelShaderNum, GLuint *programHandle);
void write_offline_shader_program(int program, int program_count);
#endif //_GLSHADERUTILS_H_