#ifndef __SHADER_H__
#define __SHADER_H__
#include <stdlib.h>
#include <stdio.h>
#include <log.h>
#include <GL/glew.h>
#include <assert.h>
#include <string.h>

// To create includes in GLSL
#include <stb_include.h>

/*
    This translation unit provides a way to load, and compile OpenGL shaders
*/

#define TEMP_SOURCE_BUFFER 8192

size_t read_whole_file(FILE* file, char* dest, size_t max);
void compile_shader(uint32_t shad, GLenum shader_type);
uint32_t make_shader(char* vert, char* frag);
uint32_t make_compute_shader(char* path);
void print_work_group_capabilities();

#endif