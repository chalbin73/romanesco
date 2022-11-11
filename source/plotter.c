//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include<plotter.h>

static struct plotter_protocol_t gl_plot_protocol =
{
    .plot_init = gl_plotter_init,
    .plot_render = gl_plotter_render,
    .plot_size = gl_plotter_viewport_size,
    .plot_resolution = gl_plotter_resolution,
    .plot_clean = gl_plotter_clean,
    .context_size = sizeof(gl_plotter_ctx_t)
};

//Not implemented
//static struct plotter_protocol_t cpu_plot_protocol = {0};

void plotting_init(enum plotting_method plt_meth, struct plot_ctx_t *plt_ctx, uint32_t width, uint32_t height)
{
    LOG("Initializing plotter");
    switch (plt_meth)
    {
    case PLOT_GL:
        plt_ctx->method = gl_plot_protocol;
        LOG("Using PLOT_GL");
        break;
    
    default:
        FAIL("Method unsupported or unimplemented");
        break;
    }

    LOG("Initializing plotting method");
    plt_ctx->ctx = malloc(plt_ctx->method.context_size);
    plt_ctx->method.plot_init(plt_ctx->ctx, width, height);
}

void plotting_render(struct plot_ctx_t *plt_ctx, struct plot_params_t *params)
{
    plt_ctx->method.plot_render(plt_ctx->ctx, params);
}

void plotting_size(struct plot_ctx_t *plt_ctx, uint32_t width, uint32_t height)
{
    plt_ctx->method.plot_size(plt_ctx->ctx, width, height);
}

void plotting_resolution(struct plot_ctx_t *plt_ctx, uint32_t width, uint32_t height)
{
    plt_ctx->method.plot_resolution(plt_ctx->ctx, width, height);
}

void plotting_clean(struct plot_ctx_t *plt_ctx)
{
    LOG("Cleaning plotter");
    plt_ctx->method.plot_clean(plt_ctx->ctx);
    free(plt_ctx->ctx);
}

int plot_params_eq(struct plot_params_t a, struct plot_params_t b)
{
    if(a.exponent == b.exponent &&
        a.ip == b.ip &&
        a.iterations == b.iterations &&
        a.rp == b.rp &&
        a.updated == b.updated &&
        a.use_moivre == b.use_moivre &&
        a.window.aspect_ratio == b.window.aspect_ratio &&
        a.window.c_i == b.window.c_i &&
        a.window.c_r == b.window.c_r &&
        a.window.zoom == b.window.zoom &&
        a.fractal == b.fractal)
    {
        return 1;
    }

    return 0;
}