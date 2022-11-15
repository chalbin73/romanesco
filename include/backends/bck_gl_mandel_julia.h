#ifndef __BCK_GL_MANDEL_JULIA_H__
#define __BCK_GL_MANDEL_JULIA_H__

//This translation unit provides a backend to render julia and mandelbrot sets with OpenGL

//Local OpenGL Compute shader work group size, 
//(used to define the number of work groups dispatched)
#define LOCAL_WORK_GROUP_SIZE 16
#define HISTO_SSBO_LENGTH 501

//Minimum time between redraws
#define MIN_REDRAW_MS 30

#include <os_name.h>
#if OS_WINDOWS
    #define COLOR_MAPS_DIR "color_maps\\"
#else
    #define COLOR_MAPS_DIR "color_maps/"
#endif

#if OS_WINDOWS
    #include <window.h> //Used for GetTickCount() function on windows
#endif

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <nuklear.h>

#include <stdint.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <nuklear.h>
#include <log.h>
#include <gl_error.h>
#include <shader.h>
#include <time.h>
#include <math.h>
#include <stb_image.h>
#include <backend_proxy.h>
#include <util.h>

extern float screen_quad[12];
extern uint32_t indices[6];

extern const char* FRACTAL_NAMES[2];
extern const char* COLORING_METHODS_NAMES[3];

//Fractals that can be rendered by this backend (the user can chose which sets to render)
enum mj_set_t
{
    MJ_MANDELBROT_SET = 0,
    MJ_JULIA_SETS = 1
};

enum color_method_t
{
    COL_MOD = 0, //Loops the color map as you zoom
    COL_HIST, //Uses the histogram method
    COL_ITER //Maps the iterations on the color map
};

// Stores the position of a window
// By its center in the complex plane (c_r and c_i)
// And its "magnification" as zoom, which is 1 over the window width
typedef struct
{
    double c_r;
    double c_i;
    double zoom;
    double aspect_ratio;
} plt_win_t;

// Stores the parameters chosed by the user to render
// Such as what fractal to render (Mandelbrot or julia)
// Or exponent
typedef struct
{
    //specifies if a redraw is needed
    bool updated;

    //Constant for julia sets
    double jrp;
    double jip;

    uint32_t max_iterations;

    double exponent;
    //Wether or not to use the moivre's exponentiation formula
    bool use_moivre;

    enum mj_set_t fractal;
    enum color_method_t color_method;
} plt_params_t;

// GL Plotter context
typedef struct
{
    //The current viewing window
    plt_win_t window;

    //The settings of the render
    plt_params_t params;

    //Size used by the texture
    uint32_t plot_width;
    uint32_t plot_height;

    //Texture on which the rendering is done
    uint32_t render_texture_ID;
    uint32_t coloring_texture_ID;

    //OpenGL buffers
    uint32_t quad_VBO;
    uint32_t quad_EBO;
    uint32_t quad_VAO;

    //Rendering context
    uint32_t ctx_height;
    uint32_t ctx_width;

    //Programsgl_plotter_ctx_t
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

    uint32_t col_method_loc;
    uint32_t compute_histogram_loc;

    //Last redraw time in millis
    uint64_t last_redraw;

    //Histogram coloring SSBOs:
    uint32_t iteration_histogram_ssbo;
    uint32_t total_histogram_ssbo;


    //Data for UI
    int res_width_ui;
    int res_height_ui;
    bool show_iteration_histogram;

    //Color maps paths
    size_t color_maps_count;
    char** color_maps;
    struct nk_image *color_maps_imgs;

    int selected_color_map;

    uint32_t histo[HISTO_SSBO_LENGTH];

} bck_gl_mj_ctx_t;

//Returns true if a equals b
int plt_params_eq(plt_params_t a, plt_params_t b);

//Backend methods

//All methods from the interface start with bck_ (for backend)
//Then gl_mj for opengl mandelbrot julia
uint32_t bck_gl_mj_init(void *gctx);
uint32_t bck_gl_mj_resize(void *gctx, uint32_t width, uint32_t height);
uint32_t bck_gl_mj_render(void *gctx);
uint32_t bck_gl_mj_plot_save(void *gctx, char* filename);
uint32_t bck_gl_mj_end(void *gctx);
uint32_t bck_gl_mj_ui_control(void *gctx, struct nk_context *nk_ctx);
uint32_t bck_gl_mj_ui_custom(void *gctx, struct nk_context *nk_ctx);
uint32_t bck_gl_mj_mouse_drag(void *gctx, double xoff, double yoff);
uint32_t bck_gl_mj_mouse_zoom(void *gctx, double offset);

//Functions inherent to the backend
void gl_mj_load_coloring_scheme(void *gctx, char *filename);
void gl_mj_rendering_resolution(void *gctx, uint32_t width, uint32_t height);

//Proxy interface :
//Function pointers
extern struct backend_proxy_t bck_gl_mandel_julia_proxy;

extern struct backend_t bck_gl_mandel_julia;

#endif //__BCK_GL_MANDEL_JULIA_H__