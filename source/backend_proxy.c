#include <backend_proxy.h>
#include <stdint.h>

uint32_t backend_proxy_init(struct backen_instance_t *backend_instance)
{
    //Allocate space for the backend context
    backend_instance->bctx = xmalloc(backend_instance->interface.proxy->bctx_size);

    LOGF("Instancing \"%s\" backend.", backend_instance->interface.backend_name);

    return backend_instance->interface.proxy->backend_init(backend_instance->bctx);
}

uint32_t backend_proxy_render(struct backen_instance_t *backend_instance)
{
    return backend_instance->interface.proxy->backend_render(backend_instance->bctx);
}

uint32_t backend_proxy_end(struct backen_instance_t *backend_instance)
{
    uint32_t result = backend_instance->interface.proxy->backend_end(backend_instance->bctx);
    //Free backend context

    free(backend_instance->bctx);

    return result;
}

//"Relay" the function to the backend instance
uint32_t backend_proxy_ui_control(struct backen_instance_t *backend_instance, struct nk_context *nk_ctx)
{
    return backend_instance->interface.proxy->backend_ui_control(backend_instance->bctx, nk_ctx);
}

uint32_t backend_proxy_ui_custom(struct backen_instance_t *backend_instance, struct nk_context *nk_ctx)
{
    return backend_instance->interface.proxy->backend_ui_custom(backend_instance->bctx, nk_ctx);
}

uint32_t backend_proxy_resize(struct backen_instance_t *backend_instance, uint32_t width, uint32_t height)
{
    return backend_instance->interface.proxy->backend_resize(backend_instance->bctx, width, height);
}

uint32_t backend_proxy_plot_save(struct backen_instance_t *backend_instance, char* filename)
{
    return backend_instance->interface.proxy->backend_plot_save(backend_instance->bctx, filename);
}

uint32_t backend_proxy_mouse_drag(struct backen_instance_t *backend_instance, double xoff, double yoff)
{
    return backend_instance->interface.proxy->backend_mouse_drag(backend_instance->bctx, xoff, yoff);
}

uint32_t backend_proxy_mouse_zoom(struct backen_instance_t *backend_instance, double offset)
{
    return backend_instance->interface.proxy->backend_mouse_zoom(backend_instance->bctx, offset);
}

uint32_t backend_proxy_mouse_click(struct backen_instance_t *backend_instance, enum click_type_t click_type, enum click_btn_t button)
{
    return backend_instance->interface.proxy->backend_mouse_click(backend_instance->bctx, click_type, button);
}