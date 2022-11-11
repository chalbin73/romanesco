//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include<window.h>

void window_create(struct window_ctx_t *ctx, const char* title, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t flags, bool use_gl)
{
    assert(ctx && "Check for a valid context pointer");
    LOGF("Creating new window (%s)", use_gl ? "GL" : "NO GL");
    flags |= use_gl ? (SDL_WINDOW_OPENGL) : 0;
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    int err_sdl = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    ASSERT(!err_sdl, "Sdl initiliazation failed ! Error code : %d (%x).", err_sdl, err_sdl);

    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    ctx->window = SDL_CreateWindow(title, x, y, width, height, flags);
    assert(ctx->window && "Check window integrity");
    SDL_ShowWindow(ctx->window);


    ctx->GL = use_gl;
    if(use_gl)
    {
        ctx->gl_ctx = SDL_GL_CreateContext(ctx->window);
    }
    assert(ctx->gl_ctx && "Check for a valid GL context");
    SUC("Window created !");
}

void window_init_gui(struct window_ctx_t *ctx)
{
    ctx->nk_ctx = nk_sdl_init(ctx->window);

    {struct nk_font_atlas *atlas;
    
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();
    nk_style_load_all_cursors(ctx->nk_ctx, atlas->cursors);

    }

    SDL_ShowCursor(SDL_DISABLE);
}

void window_set_resize_callback(struct window_ctx_t *ctx, void (*resize_callback)(int width, int height))
{
    ctx->resize_callback = resize_callback;
}

void window_set_event_callback(struct window_ctx_t *ctx, void (*event_callback)(SDL_Event event, struct window_ctx_t *ctx))
{
    ctx->event_callback = event_callback;
}

SDL_Window *window_get_window_ptr(struct window_ctx_t *ctx)
{
    return ctx->window;
}

void window_loop(struct window_ctx_t *ctx, void (*loop_callback)(struct window_ctx_t *ctx))
{
    assert(ctx && "Check for a valid context pointer");
    LOG("Beginning window loop");
    int quit = 0;
    while(!quit)
    {
        nk_input_begin(ctx->nk_ctx);
        while(SDL_PollEvent(&ctx->event))
        {
            if(ctx->event_callback)
            {
                ctx->event_callback(ctx->event, ctx);
            }
            if(ctx->event.type == SDL_QUIT)
            {
                LOG("Quit requested");
                quit = 1;
            }
            if(ctx->event.type == SDL_WINDOWEVENT && ctx->event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                int w, h;
                SDL_GetWindowSize(ctx->window, &w, &h);

                ctx->width = w;
                ctx->height = h;

                if(ctx->resize_callback)
                {
                    ctx->resize_callback(w, h);
                }
            }
            if(ctx->event.type == SDL_KEYDOWN)
            {
                switch (ctx->event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    quit = 1;
                    break;

                default:
                    break;
                }
            }
            nk_sdl_handle_event(&ctx->event);
        }
        nk_input_end(ctx->nk_ctx);

        loop_callback(ctx);

        if(ctx->GL)
        {
            SDL_GL_SwapWindow(ctx->window);
        }
    }
    
    LOG("Quitting window");
}

void window_clear(struct window_ctx_t *ctx)
{
    assert(ctx && "Check for a valid context pointer");
    LOG("Cleaning window");
    SDL_DestroyWindow(ctx->window);
    SDL_GL_DeleteContext(ctx->gl_ctx);
}
