#ifndef __GL_PLOTTER_H__
#define __GL_PLOTTER_H__

#define GLEW_STATIC
#include<GL/glew.h>
#include<stdint.h>
#include<log.h>
#include<shader.h>
#include<plotter_params.h>
#include<gl_error.h>
#include<math.h>
#include<time.h>
#include<stdbool.h>

//Include windows header for Clock
#include<os_name.h>
#if OS_WINDOWS
    #include<window.h>
#endif

/*
    This translation unit provides an OpenGL plotter, following the "plotter.h" interface
*/

//Local OpenGL Compute shader work group size, 
//(used to define the number of work groups dispatched)
#define LOCAL_WORK_GROUP_SIZE 16
#define HISTO_SSBO_LENGTH 501



extern float screen_quad[12];

extern uint32_t indices[6];

//Predefinition (normaly in plotter.h)
struct plot_params_t;

// GL Plotter context
typedef struct
{
    //Size used by the texture
    uint32_t plot_width;
    uint32_t plot_height;

    //Texture on which the rendering is done
    uint32_t render_texture_ID;

    //OpenGL buffers
    uint32_t quad_VBO;
    uint32_t quad_EBO;
    uint32_t quad_VAO;

    //Rendering context
    uint32_t ctx_height;
    uint32_t ctx_width;

    //Programs
    //Compute shader; renders fractal
    uint32_t compute_program;
    uint32_t work_groups_width;
    uint32_t work_groups_height;

    //Vert/Frag shader; needed to render texture to quad
    uint32_t gl_program;

    //Uniform location
    //Julia constant
    uint32_t rp_loc;
    uint32_t ip_loc;

    //Window
    uint32_t r_loc;
    uint32_t i_loc;
    uint32_t z_loc;
    uint32_t ar_loc;
    uint32_t width_loc;
    uint32_t height_loc;

    //Plot parameters
    uint32_t it_loc;
    uint32_t e_loc;
    uint32_t m_loc;
    uint32_t fractal_loc;
    uint32_t pass_loc;

    uint64_t last_redraw;

    //Histogram coloring SSBOs:
    uint32_t iteration_histogram_ssbo;
    uint32_t total_histogram_ssbo;
} gl_plotter_ctx_t;


void gl_plotter_init(void *gctx, uint32_t width, uint32_t height);
void gl_plotter_viewport_size(void *gctx, uint32_t width, uint32_t height);
void gl_plotter_resolution(void *gctx, uint32_t width, uint32_t height);
void gl_plotter_render(void *gctx, struct plot_params_t *params);
void gl_plotter_save_current_texture(void *gctx, char* filepath);
void gl_plotter_clean(void *gctx);

#endif