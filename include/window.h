#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <log.h>
#include <assert.h>
#include <gl_error.h>

//Nuklear
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <nuklear.h>
#include <nuklear_sdl_gl3.h>

/*
    This translation unit provides a GUI/Window interface
    In this specific case an SDL2 implementation

    (Might be changed to GLFW3 later)
*/

struct window_ctx_t
{
    SDL_Window* window;
    SDL_Renderer* rend;
    SDL_Event event;

    struct nk_context *nk_ctx;

    bool GL;
    SDL_GLContext gl_ctx;
    void (*resize_callback)(int width, int height);

    void (*event_callback)(SDL_Event event, struct window_ctx_t *ctx);

    uint32_t width;
    uint32_t height;
};

void window_create(struct window_ctx_t *ctx, const char* title, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t flags, bool use_gl);
void window_init_gui(struct window_ctx_t *ctx);
void window_set_resize_callback(struct window_ctx_t *ctx, void (*resize_callback)(int width, int height));
void window_set_event_callback(struct window_ctx_t *ctx, void (*event_callback)(SDL_Event event, struct window_ctx_t *ctx));
SDL_Window *window_get_window_ptr(struct window_ctx_t *ctx);
void window_loop(struct window_ctx_t *ctx, void (*loop_callback)(struct window_ctx_t *ctx));
void window_clear(struct window_ctx_t *ctx);

#endif