//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <SDL2/SDL_mouse.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#define GLEW_STATIC

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <assert.h>
#include <window.h>
#include <GL/glew.h>
#include <util.h>
#include <time.h>
#include <os_name.h>
#include <static_infos.h>
#include <backend_proxy.h>
#include <backends/bck_gl_mandel_julia.h>

#define LOG_TO_FILE
#define LOG_FUNC
#define LOG_IMPLEMENTATION
#include <log.h>

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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool mouse_pressed = false;
double prev_x_clk;
double prev_y_clk;

struct backen_instance_t bck_inst;

void event_callback(SDL_Event event, struct window_ctx_t *ctx)
{
    bool act = nk_item_is_any_active(ctx->nk_ctx);

    if(event.type == SDL_MOUSEWHEEL && !act)
    {
        backend_proxy_mouse_zoom(&bck_inst, event.wheel.y);
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

        nk_style_set_cursor(ctx->nk_ctx, NK_CURSOR_MOVE);
    }
    if(event.type == SDL_MOUSEMOTION && mouse_pressed && !act)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        int w, h;
        SDL_GetWindowSize(window_get_window_ptr(ctx), &w, &h);

        double n_x_clk = map((double)x,0, w, -1.0, 1.0);
        double n_y_clk = map((double)y,0, h, -1.0, 1.0);

        backend_proxy_mouse_drag(&bck_inst, n_x_clk - prev_x_clk, n_y_clk - prev_y_clk);

        prev_x_clk = n_x_clk;
        prev_y_clk = n_y_clk;
    }
    if(mouse_pressed && !act) 
    {
        nk_style_set_cursor(ctx->nk_ctx, NK_CURSOR_MOVE);
    }
    if(event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
    {
        nk_style_set_cursor(ctx->nk_ctx, NK_CURSOR_ARROW);
        mouse_pressed = false;
    }
    
}

void loop_callback(struct window_ctx_t *wctx)
{
    if(nk_begin(wctx->nk_ctx, "Control", nk_rect(10, 10, 300, 500), NK_WINDOW_MOVABLE | NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_SCALABLE))
    {
        backend_proxy_ui_control(&bck_inst, wctx->nk_ctx);
    }
    nk_end(wctx->nk_ctx);

    backend_proxy_ui_custom(&bck_inst, wctx->nk_ctx);

    backend_proxy_render(&bck_inst);
    GL_ERR(nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY));
}

//When window is resized
void resize_callback(int w, int h)
{
    backend_proxy_resize(&bck_inst, w, h);
}

int main(int argc, char* argv[])
{

    START_LOG_FILE("romanesco_last.log");

    SUCF("Welcome to the Romanesco plotter version %s!", VERSION_TEXT);
    LOG("This program and its source code are provided under the MIT license.");
    LOG("See LICENSE.txt for more details.");
    LOGF("Currently on: %s", OS_NAME);

    struct window_ctx_t wctx;


    window_create(&wctx, "Romanesco plotter", 100, 100, 600, 400, SDL_WINDOW_RESIZABLE, true);
    window_set_resize_callback(&wctx, resize_callback);
    window_set_event_callback(&wctx, event_callback);

    //GLEW Initialization
    int gerr = glewInit();
    ASSERTF(gerr == GLEW_OK, "Glew initialization failed ! Error code : %d !", gerr);

    //Load backend
    bck_inst.interface = bck_gl_mandel_julia;

    backend_proxy_init(&bck_inst);
    window_init_gui(&wctx);

    window_loop(&wctx, loop_callback);

    backend_proxy_end(&bck_inst);
    window_clear(&wctx);

    SUC("Goodbye, program exiting normaly !");
    END_LOG_FILE();
    return 0;
}