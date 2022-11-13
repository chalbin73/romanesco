#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <log.h>
#include <string.h>
#include <GL/glew.h>
#include <stb_image.h>

//Wrapper arround Posix's dirent and Windows FindNextFile
#include <tinydir.h>

/*
    This translation unit provides utilities functions that might be used in different parts of the program
*/

//float map(float v, float v_min, float v_max, float out_min, float out_max);
double map(double v, double v_min, double v_max, double out_min, double out_max);
size_t get_file_count_in_dir(char* dir_path);
char** get_files_in_dir(char* dir_path, size_t *filecount);

//Load image from disk with GL_RGB format, needed for nuklear
uint32_t load_image_gl_rgb(char* image_path);

#endif