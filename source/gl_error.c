#include<gl_error.h>

char const* gl_error_string(GLenum const err)
{
  switch (err)
  {
    // opengl 2 errors (8)
    case GL_NO_ERROR:
      return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";

    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW";

    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";

    case GL_TABLE_TOO_LARGE:
      return "GL_TABLE_TOO_LARGE";

    // opengl 3 errors (1)
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";

    // gles 2, 3 and gl 4 error are handled by the switch above
    default:
      assert(!"unknown error");
      return NULL;
  }
}

void flush_gl_err(const char* filename, const int line, const char *function_name, const char* statement)
{
    GLenum err = 0;
    while((err = glGetError()) != GL_NO_ERROR)
    {
      //Log non fatal error
      __log_msg(3, filename,line, function_name, "Uncatched GL error %x (%s) when %s !", err, gl_error_string(err), statement);
    }
}

void check_gl_err(const char* filename, const int line, const char *function_name, const char* statement)
{
    bool errored = false;
    GLenum err = 0;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        errored = true;
        //Log fatal error
        __log_msg(4, filename,line, function_name, "Fatal GL error %x (%s) when %s !", err, gl_error_string(err), statement);

    }

    assert(!errored);
}