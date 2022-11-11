#ifndef __GL_ERROR_H__
#define __GL_ERROR_H__

#include<GL/glew.h>
#include<assert.h>
#include<log.h>
#include<stdbool.h>

/*
  This translation unit provides methods to catch OpenGL errors
*/

char const* gl_error_string(GLenum const err);
void flush_gl_err(const char* filename, const int line, const char *function_name, const char* statement);
void check_gl_err(const char* filename, const int line, const char *function_name, const char* statement);

#define GL_ERR(s) {flush_gl_err(__FILE__, __LINE__, __func__, #s); s; check_gl_err(__FILE__, __LINE__, __func__, #s);}

#endif // __GL_ERROR_H__