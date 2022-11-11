#pragma once
#ifndef __PLOTTER_H__
#define __PLOTTER_H__

#include<stdint.h>
#include<stdbool.h>
#include<assert.h>
#include<log.h>
#include<gl_plotter.h>
#include<plotter_params.h>

/*
    This translation unit provides a simple interface for a plotting "backend"
    Thus making expension to other apis for plotting or other fractals easier
    (Vulkan, OpenCL, CUDA, CPU)
*/


void plotting_init(enum plotting_method plt_meth, struct plot_ctx_t *plt_ctx, uint32_t width, uint32_t height);
void plotting_render(struct plot_ctx_t *plt_ctx, struct plot_params_t *params);
void plotting_size(struct plot_ctx_t *plt_ctx, uint32_t width, uint32_t height);
void plotting_resolution(struct plot_ctx_t *plt_ctx, uint32_t width, uint32_t height);
void plotting_clean(struct plot_ctx_t *plt_ctx);

int plot_params_eq(struct plot_params_t a, struct plot_params_t b);

#endif