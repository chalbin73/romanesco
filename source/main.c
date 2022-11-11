//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#define GLEW_STATIC


#include<stdio.h>
#include<stdlib.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_render.h>
#include<stdbool.h>
#include<assert.h>
#include<window.h>
#include<GL/glew.h>
#include<plotter.h>
#include<util.h>
#include<time.h>
#include<os_name.h>
#include<static_infos.h>

#define LOG_TO_FILE
#define LOG_FUNC
#define LOG_IMPLEMENTATION
#include<log.h>


#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

//Nuklear
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include <nuklear.h>
#include <nuklear_sdl_gl3.h>

struct plot_params_t plot_params = {0};
struct plot_ctx_t plt_ctx;

bool mouse_pressed = false;
double prev_x_clk;
double prev_y_clk;

int res_width_ui = 1920;
int res_height_ui = 1920;

static const char* FRACTAL_NAMES[] =
{
    "Mandlebrot set",
    "Julia sets"
};

void event_callback(SDL_Event event, struct window_ctx_t *ctx)
{
    bool act = nk_item_is_any_active(ctx->nk_ctx);

    if(!act)
    {
        nk_style_show_cursor(ctx->nk_ctx);
    }

    

    if(event.type == SDL_MOUSEWHEEL && !act)
    {
        plot_params.window.zoom += plot_params.window.zoom * ((_flp)event.wheel.y / 10);
        plot_params.updated = true;
    }
    if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && !mouse_pressed && !act)
    {
        mouse_pressed = true;

        int x, y;
        SDL_GetMouseState(&x, &y);

        int w, h;
        SDL_GetWindowSize(window_get_window_ptr(ctx), &w, &h);

        prev_x_clk = map((double)x,0, w, -1.0, 1.0);
        prev_y_clk = map((double)y,0, h, -1.0, 1.0);

        prev_x_clk /= plot_params.window.zoom / plot_params.window.aspect_ratio;
        prev_y_clk /= plot_params.window.zoom;
    }
    if(event.type == SDL_MOUSEMOTION && mouse_pressed && !act)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        int w, h;
        SDL_GetWindowSize(window_get_window_ptr(ctx), &w, &h);

        double n_x_clk = map((double)x,0, w, -1.0, 1.0);
        double n_y_clk = map((double)y,0, h, -1.0, 1.0);

        n_x_clk /= plot_params.window.zoom / plot_params.window.aspect_ratio;
        n_y_clk /= plot_params.window.zoom;

        plot_params.window.c_r -= n_x_clk - prev_x_clk;
        plot_params.window.c_i += n_y_clk - prev_y_clk;

        prev_x_clk = n_x_clk;
        prev_y_clk = n_y_clk;
        plot_params.updated = true;
    }
    if(event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && !act)
    {
        mouse_pressed = false;
    }
    
}

void loop_callback(struct window_ctx_t *wctx)
{
    //GUI
    struct plot_params_t prev_par = plot_params;
    struct nk_context *nk_ctx = wctx->nk_ctx;
    if(nk_begin(nk_ctx, "Control", nk_rect(30, 30, 300, 500), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(nk_ctx, 10, 1);
        nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "Current position : ");
        nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "%.15f +", plot_params.window.c_r);
        nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "%.15fi", plot_params.window.c_i);
        nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "Zoom : %f", plot_params.window.zoom);
        nk_layout_row_dynamic(nk_ctx, 10, 1);
        nk_layout_row_dynamic(nk_ctx, 20, 1);

        if(nk_widget_is_hovered(nk_ctx))
        {
            nk_tooltip(nk_ctx, "Sets the maximum ammount of iterations per-pixel /!\\ Slow if high !");
        }
        int u_iters = (int)plot_params.iterations;
        nk_property_int(nk_ctx, "Max iterations", 10, &u_iters, 500, 5, 1);
        plot_params.iterations = u_iters;
        
        plot_params.fractal = nk_combo(nk_ctx, FRACTAL_NAMES, 2, plot_params.fractal, 15, nk_vec2(200, 200));

        plot_params.use_moivre = !nk_check_label(nk_ctx, "Use moivre exponentation formula (slower)", !plot_params.use_moivre);

        if(plot_params.use_moivre)
        {
            nk_property_double(nk_ctx, "Exponent", 1.0, &plot_params.exponent, 5.0, 0.05, 0.05f);
        }else
        {
            int e = plot_params.exponent;
            nk_property_int(nk_ctx, "Exponent", 1, &e, 5, 1, 1);
            plot_params.exponent = (double)e;
        }

        nk_layout_row_dynamic(nk_ctx, 10, 1);
        nk_layout_row_dynamic(nk_ctx, 20, 1);

        if(plot_params.fractal == FRAC_JULIA_SET)
        {
            nk_label(nk_ctx, "Julia set constant", NK_TEXT_ALIGN_CENTERED);
            nk_label(nk_ctx, "(z = z^2 + c)", NK_TEXT_ALIGN_LEFT);
            nk_label(nk_ctx, "C =", NK_TEXT_ALIGN_LEFT);
            nk_property_double(nk_ctx, "Real part", -1.0, &plot_params.rp, 1.0f, 0.01f, 0.05f);
            nk_property_double(nk_ctx, "Imaginary part", -1.0, &plot_params.ip, 1.0f, 0.01f, 0.05f);
            
        }

        nk_layout_row_dynamic(nk_ctx, 10, 1);
        nk_layout_row_dynamic(nk_ctx, 20, 1);
        nk_label(nk_ctx, "Rendering resolution", NK_TEXT_ALIGN_CENTERED);
        nk_layout_row_dynamic(nk_ctx, 20, 2);
        nk_property_int(nk_ctx, "Width", 10, &res_width_ui, 7680, 1, 1);
        nk_property_int(nk_ctx, "Height", 10, &res_height_ui, 4320, 1, 1);
        nk_layout_row_dynamic(nk_ctx, 20, 1);

        //Update resolution
        if(nk_widget_is_hovered(nk_ctx))
        {
            nk_tooltip(nk_ctx, "Changes the rendering texture resolution.");
        }
        if(nk_button_label(nk_ctx, "Update resolution"))
        {
            plotting_resolution(&plt_ctx, res_width_ui, res_height_ui);
            plot_params.updated = true;
        }

        //Print position
        if(nk_widget_is_hovered(nk_ctx))
        {
            nk_tooltip(nk_ctx, "Prints window position in stdout.");
        }
        if(nk_button_label(nk_ctx, "Print position"))
        {
            LOG("Position :");
            LOGF("%.20f", plot_params.window.c_r);
            LOGF("%.20fi", plot_params.window.c_i);
            LOGF("%.20fz", plot_params.window.zoom);
        }

        //Save GL Texture
        if(nk_widget_is_hovered(nk_ctx))
        {
            nk_tooltip(nk_ctx, "Saves the rendered texture to disk.");
        }
        if(nk_button_label(nk_ctx, "Save GL Texture"))
        {
            gl_plotter_save_current_texture(plt_ctx.ctx, "out.ppm");
        }

    }
    nk_end(nk_ctx);
    
    //Update parameters only if there are modified
    if(!plot_params_eq(prev_par, plot_params))
    {
        plot_params.updated = true;
    }
    plotting_render(&plt_ctx, &plot_params);
    GL_ERR(nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY));

}

//When window is resized
void resize_callback(int w, int h)
{
    plot_params.window.aspect_ratio = (double)w/(double)h;
    plotting_size(&plt_ctx, w, h);
    plot_params.updated = true;
}

int main(int argc, char* argv[])
{
    START_LOG_FILE("romanesco_last.log");
    SUCF("Welcome to the Romanesco plotter version %s!", VERSION_TEXT);
    LOG("This program and its source code are provided under the MIT license.");
    LOG("See LICENSE.txt for more details.");
    LOGF("Currently on: %s", OS_NAME);

    struct window_ctx_t wctx;

    plot_params.updated = true;
    plot_params.fractal = FRAC_JULIA_SET;

    window_create(&wctx, "Hello window !", 100, 100, 600, 400, SDL_WINDOW_RESIZABLE, true);
    window_set_resize_callback(&wctx, resize_callback);
    window_set_event_callback(&wctx, event_callback);

    //GLEW Initialization
    int gerr = glewInit();
    ASSERT(gerr == GLEW_OK, "Glew initialization failed ! Error code : %d !", gerr);


    plot_params.iterations = 100;
    plot_params.window.zoom = 1.0;
    plot_params.rp = 0.5;
    plot_params.exponent = 2.0;
    plot_params.use_moivre = false;

    plotting_init(PLOT_GL, &plt_ctx, 600, 400);
    window_init_gui(&wctx);
    plotting_resolution(&plt_ctx, 1920, 1080);

    window_loop(&wctx, loop_callback);

    plotting_clean(&plt_ctx);
    window_clear(&wctx);

    SUC("Goodbye, program exiting normaly !");
    END_LOG_FILE();
    return 0;
}