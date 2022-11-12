#ifndef __BACKEND_PROXY_H__
#define __BACKEND_PROXY_H__

#include <stdlib.h>
#include <stdint.h>
#include <nuklear.h>
#include <log.h>

/*
*   Rendering software pieces are called backends
*   This translation unit provides an interface that backends
*   Should follow in order to interface with the codebase
*/

//Is the actual interface
//Contains function pointers to the backend
struct backend_proxy_t
{
    //All methods return 0 if success
    //All methods shall take a pointer to a context (to save stat inherent to the backend)
    size_t bctx_size;
    //Shall initialize the backend with default state
    uint32_t (*backend_init)(void *bctx);

    //Shall render the next frame
    uint32_t (*backend_render)(void *bctx);

    //Shall end and clean necessary things
    uint32_t (*backend_end)(void *bctx);

    //Ui function must be done with Nuklear API
    //Will be called when the control widget need to be rendred (in control window)
    uint32_t (*backend_ui_control)(void *bctx, struct nk_context *nk_ctx);

    //Will be called when custom windows can be rendered
    uint32_t (*backend_ui_custom)(void *bctx, struct nk_context *nk_ctx);

    //Will be called when the window is resized
    uint32_t (*backend_resize)(void *bctx, uint32_t width, uint32_t height);

    //Will be called when the plot is to be saved to disk
    uint32_t (*backend_plot_save)(void *bctx, char* filename);

    //Will be called when the mouse is dragging(clicked) (normalized coords)
    uint32_t (*backend_mouse_drag)(void *bctx, double xoff, double yoff);

    //Will be called when mouse is zooming (mouse wheel)
    uint32_t (*backend_mouse_zoom)(void *bctx, double offset);
};

//Holds pointers to backend implementation plus
//Information about the current backend
struct backend_t
{
    const char *backend_name;
    struct backend_proxy_t *proxy;
};

//The actual "instance" of the backend with its data
struct backen_instance_t
{
    //The backend that we are "instancing"
    struct backend_t interface;

    //A pointer to the backend context
    void* bctx;
};

//Functions called by the top level that actually interface with the rendering backend
uint32_t backend_proxy_init(struct backen_instance_t *backend_instance);
uint32_t backend_proxy_render(struct backen_instance_t *backend_instance);
uint32_t backend_proxy_end(struct backen_instance_t *backend_instance);
uint32_t backend_proxy_ui_control(struct backen_instance_t *backend_instance, struct nk_context *nk_ctx);
uint32_t backend_proxy_ui_custom(struct backen_instance_t *backend_instance, struct nk_context *nk_ctx);
uint32_t backend_proxy_resize(struct backen_instance_t *backend_instance, uint32_t width, uint32_t height);
uint32_t backend_proxy_plot_save(struct backen_instance_t *backend_instance, char* filename);
uint32_t backend_proxy_mouse_drag(struct backen_instance_t *backend_instance, double xoff, double yoff);
uint32_t backend_proxy_mouse_zoom(struct backen_instance_t *backend_instance, double offset);

#endif //__BACKEND_PROXY_H__