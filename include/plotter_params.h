#ifndef __PLOTTER_PARAMS_H__
#define __PLOTTER_PARAMS_H__

/*
    This translation unit provides structures and types used to represent the plotting context
    (It is mostly there to prevent circular dependencies)
*/

#include<stdint.h>
#include<stdlib.h>
#include<stdbool.h>

//Precision used
typedef double _flp;

enum plotting_method
{
    PLOT_CPU = 0, //Not implemented
    PLOT_GL
};

// -- MAKE SURE THEY HAVE THE SAME VALUES IN THE COMPUTATION CODE --
enum fractal
{
    FRAC_MANDELBROT_SET = 0,
    FRAC_JULIA_SET
    //TODO: Newton fractal
};


//Plotting window
struct plt_win_t
{
    //Window center
    _flp c_r;
    _flp c_i;

    //Zoom
    _flp zoom;

    _flp aspect_ratio;
};

//Parameters for the plotting
struct plot_params_t
{
    struct plt_win_t window;

    //Julia c constant
    _flp rp;
    _flp ip;

    uint32_t iterations;

    _flp exponent;

    //Use moivre formula for exponentation (more expensive)
    bool use_moivre;

    //Set to true if a new render is requested
    bool updated;

    //The fractal that is being plotted;
    enum fractal fractal;
};

//Interface to a plotting method
struct plotter_protocol_t
{
    void (*plot_init)(void *ctx, uint32_t width, uint32_t height);
    void (*plot_render)(void *ctx, struct plot_params_t *params);
    // The size of the window, so that the rendered data can be shown properly
    void (*plot_size)(void *ctx, uint32_t width, uint32_t height);
    //The resolution of the plot, independent of the window size
    void (*plot_resolution)(void *ctx, uint32_t width, uint32_t height);
    void (*plot_clean)(void *ctx);
    size_t context_size;
};

struct plot_ctx_t
{
    void *ctx;
    struct plotter_protocol_t method;
};


#endif //__PLOTTER_PARAMS_H__