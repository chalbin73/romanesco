#include <backends/bck_gl_mandel_julia.h>


const char* FRACTAL_NAMES[2] = 
{
    "Mandlebrot set",
    "Julia sets"
};

const char* COLORING_METHODS_NAMES[3] =
{
    "Modular coloring (loop)",
    "Histogram coloring",
    "Iteration coloring"
};

//Quad to render on
float screen_quad[12] =
{
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f, 1.0f, 0.0f
};

uint32_t indices[6] = 
{
    0, 1, 3,
    0, 3, 2
};

//Proxy interface :
//Function pointers
struct backend_proxy_t bck_gl_mandel_julia_proxy =
{
    .bctx_size = sizeof(bck_gl_mj_ctx_t),
    .backend_init = bck_gl_mj_init,
    .backend_resize = bck_gl_mj_resize,
    .backend_render = bck_gl_mj_render,
    .backend_plot_save = bck_gl_mj_plot_save,
    .backend_end = bck_gl_mj_end,
    .backend_ui_control = bck_gl_mj_ui_control,
    .backend_ui_custom = bck_gl_mj_ui_custom,
    .backend_mouse_drag = bck_gl_mj_mouse_drag,
    .backend_mouse_zoom = bck_gl_mj_mouse_zoom,
    .backend_mouse_click = bck_gl_mj_mouse_click
};

struct backend_t bck_gl_mandel_julia =
{
    .proxy = &bck_gl_mandel_julia_proxy,
    .backend_name = "OpenGL Mandelbrot and Julia sets renderer"
};

uint32_t bck_gl_mj_init(void *gctx)
{
    bck_gl_mj_ctx_t *ctx = gctx;
    LOG("Initializing OpenGL julia and mandelbrot rendering backend !");

    //Creating rendering quad
    GL_ERR(glGenVertexArrays(1, &ctx->quad_VAO));
    GL_ERR(glBindVertexArray(ctx->quad_VAO));


    //Creating quad VAO/VBO
    GL_ERR(glGenBuffers(1, &ctx->quad_VBO));
    GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, ctx->quad_VBO));
    GL_ERR(glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad), screen_quad, GL_STATIC_DRAW));
    GL_ERR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0));
    GL_ERR(glEnableVertexAttribArray(0));

    //Creating quad EBO
    GL_ERR(glGenBuffers(1, &ctx->quad_EBO));
    GL_ERR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quad_EBO));
    GL_ERR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    //Creating histogram SSBO (for histogram rendering (see wikipedia))
    glGenBuffers(1, &ctx->iteration_histogram_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->iteration_histogram_ssbo);
    //Init buffer with zeros
    uint32_t *data = (uint32_t*)calloc(HISTO_SSBO_LENGTH, sizeof(uint32_t));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * HISTO_SSBO_LENGTH, data, GL_DYNAMIC_DRAW);
    free(data);

    //Create a litte ssbo to hold the total value for histogram
    glGenBuffers(1, &ctx->total_histogram_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->total_histogram_ssbo);
    uint32_t zero = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t), &zero, GL_DYNAMIC_DRAW);


    LOG("Loading GL computing program");
    //Preparing program
    GL_ERR(ctx->gl_program = make_shader("gl_shaders/fractals.vert", "gl_shaders/fractals.frag"));

    //Preparing compute shader
    GL_ERR(ctx->compute_program = make_compute_shader("gl_shaders/fractal.comp"));

    //Getting all uniform loactions

    GL_ERR(glUseProgram(ctx->compute_program));
    GL_ERR(ctx->rp_loc = glGetUniformLocation(ctx->compute_program, "rp"));
    GL_ERR(ctx->ip_loc = glGetUniformLocation(ctx->compute_program, "ip"));
    
    GL_ERR(ctx->r_loc = glGetUniformLocation(ctx->compute_program, "r"));
    GL_ERR(ctx->i_loc = glGetUniformLocation(ctx->compute_program, "i"));
    GL_ERR(ctx->z_loc = glGetUniformLocation(ctx->compute_program, "z"));
    GL_ERR(ctx->ar_loc = glGetUniformLocation(ctx->compute_program, "aspect_ratio"));

    GL_ERR(ctx->it_loc = glGetUniformLocation(ctx->compute_program, "iters"));
    GL_ERR(ctx->e_loc = glGetUniformLocation(ctx->compute_program, "exponent"));
    GL_ERR(ctx->m_loc = glGetUniformLocation(ctx->compute_program, "moivre"));
    GL_ERR(ctx->fractal_loc = glGetUniformLocation(ctx->compute_program, "fractal"));
    GL_ERR(ctx->width_loc = glGetUniformLocation(ctx->compute_program, "width"));
    GL_ERR(ctx->height_loc = glGetUniformLocation(ctx->compute_program, "height"));
    GL_ERR(ctx->pass_loc = glGetUniformLocation(ctx->compute_program, "pass"));
    GL_ERR(ctx->col_method_loc = glGetUniformLocation(ctx->compute_program, "col_method"));
    GL_ERR(ctx->compute_histogram_loc = glGetUniformLocation(ctx->compute_program, "compute_histogram"));

    //Setting defaults:
    ctx->window.aspect_ratio = 1920.0/1080.0;
    ctx->window.zoom = 1.0;
    ctx->window.c_r = 0.0;
    ctx->window.c_i = 0.0;

    ctx->params.fractal = MJ_JULIA_SETS;
    ctx->params.jip = 0.0;
    ctx->params.jrp = 0.5;
    ctx->params.max_iterations = 100;
    ctx->params.exponent = 2;
    ctx->params.use_moivre = false;
    ctx->params.updated = true;
    ctx->params.color_method = COL_HIST;

    ctx->res_width_ui = 1920;
    ctx->res_height_ui = 1080;

    gl_mj_rendering_resolution(gctx, 1920, 1080);

    //Scan color maps dir
    ctx->color_maps = get_files_in_dir(COLOR_MAPS_DIR, &ctx->color_maps_count);
    ctx->color_maps_imgs = xmalloc(ctx->color_maps_count * sizeof(struct nk_image));
    LOG("Loading color maps :");

    //Load color maps ui images
    for(int i = 0; i < ctx->color_maps_count; i++)
    {
        LOGF("\tLoading \"%s\".", ctx->color_maps[i]);
        char filename[4096] = "";
        strcat(filename, COLOR_MAPS_DIR); //TODO: Make safe
        strcat(filename, ctx->color_maps[i]);
        ctx->color_maps_imgs[i] = nk_image_id((int)load_image_gl_rgb(filename));
    }

    ctx->selected_color_map = 0;
    gl_mj_load_coloring_scheme(gctx, ctx->color_maps[ctx->selected_color_map]);

    SUC("OpenGL julia, mandelbrot backend initialization success !");
    return 0; // SUCCESS !
}

uint32_t bck_gl_mj_resize(void *gctx, uint32_t width, uint32_t height)
{
    //This is called once after init
    
    bck_gl_mj_ctx_t *ctx = gctx;
    ctx->ctx_width = width;
    ctx->ctx_height = height;
    ctx->window.aspect_ratio = (double)width / (double)height;
    ctx->params.updated = true;
    GL_ERR(glViewport(0, 0, width, height));

    return 0;
}

uint32_t bck_gl_mj_render(void *gctx)
{
    bck_gl_mj_ctx_t *ctx = gctx;
    plt_params_t *params = &ctx->params;
    //Renders the fractal to the texture
    if(ctx->params.updated)
    {
        uint32_t tm = 0; //Time in milliseconds
#if OS_WINDOWS
        tm = GetTickCount();
#else
        //If linux or apple (posix compliant)
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        tm = ((uint64_t)time.tv_sec * 1e3l) + ((uint64_t)time.tv_nsec / 1e6);
        
#endif
        //Limiting render every 30 milliseconds
        if(tm - ctx->last_redraw > MIN_REDRAW_MS)
        {
            GL_ERR(glUseProgram(ctx->compute_program))
            GL_ERR(
                //Update uniforms
                glUniform1f(ctx->rp_loc, params->jrp);
                glUniform1f(ctx->ip_loc, params->jip);
                glUniform1d(ctx->r_loc, ctx->window.c_r);
                glUniform1d(ctx->i_loc, ctx->window.c_i);
                glUniform1d(ctx->z_loc, ctx->window.zoom);
                glUniform1d(ctx->ar_loc, ctx->window.aspect_ratio);
                glUniform1i(ctx->it_loc, params->max_iterations);
                glUniform1d(ctx->e_loc, params->exponent);
                glUniform1i(ctx->width_loc, ctx->plot_width);
                glUniform1i(ctx->height_loc, ctx->plot_height);
                glUniform1i(ctx->fractal_loc, params->fractal);
                glUniform1i(ctx->m_loc, params->use_moivre);
                glUniform1i(ctx->pass_loc, 0);
                glUniform1i(ctx->col_method_loc, params->color_method);
                glUniform1i(ctx->compute_histogram_loc, ctx->compute_histogram_loc);
            );


            //ZEROING histogram total
            GL_ERR(
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->total_histogram_ssbo);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ctx->total_histogram_ssbo);

                uint32_t zero = 0;
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &zero);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->iteration_histogram_ssbo);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ctx->iteration_histogram_ssbo);
            );
 
            //Zeroing histogram data
            uint32_t *hist = (uint32_t*)calloc(HISTO_SSBO_LENGTH, sizeof(uint32_t));
            assert(hist);
            GL_ERR(glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t) * HISTO_SSBO_LENGTH, hist));
            free(hist);

            // PASS 1
            GL_ERR(
                glBindTexture(GL_TEXTURE_2D, ctx->render_texture_ID);
                glBindImageTexture(0, ctx->render_texture_ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
                glBindImageTexture(1, ctx->coloring_texture_ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
                glDispatchCompute(ctx->work_groups_width, ctx->work_groups_height,1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                params->updated = false;
            );

            //TODO: Remove this and test
            GL_ERR(

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->total_histogram_ssbo);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ctx->total_histogram_ssbo);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->iteration_histogram_ssbo);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ctx->iteration_histogram_ssbo);

                //Reading histogram (only if shown):
                if(ctx->show_iteration_histogram)
                {
                    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t) * HISTO_SSBO_LENGTH, ctx->histo);
                }
            );

            // PASS 2 (Coloring pass)
            GL_ERR(glUniform1i(ctx->pass_loc, 1));
            GL_ERR(
                glBindTexture(GL_TEXTURE_2D, ctx->render_texture_ID);
                glBindImageTexture(0, ctx->render_texture_ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
                glDispatchCompute(ctx->work_groups_width, ctx->work_groups_height,1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                params->updated = false;
            );

            ctx->last_redraw = tm;
        }
    }

    //Clear and display texture
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GL_ERR(
    glUseProgram(ctx->gl_program);

    glUniform1i(glGetUniformLocation(ctx->gl_program, "tex"), 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quad_EBO);
    glBindVertexArray(ctx->quad_VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,ctx->render_texture_ID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    );

    return 0;
}

uint32_t bck_gl_mj_plot_save(void *gctx, char* filename)
{
    bck_gl_mj_ctx_t *ctx = gctx;
    LOGF("Saving texture %dx%d", ctx->plot_width, ctx->plot_height);
    float *pixels = (float*)malloc(ctx->plot_width * ctx->plot_height * sizeof(float) * 4);
    glBindTexture(GL_TEXTURE_2D, ctx->render_texture_ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

    FILE* f = fopen(filename, "w");
    assert(f);

    fprintf(f, "P6\n");
    fprintf(f, "%d %d\n255\n", ctx->plot_width, ctx->plot_height);

    //RENORM

    float max = 0.0f;
    for(int i = 0; i < ctx->plot_width * ctx->plot_height; i++)
    {
        if(pixels[i*4] > max)
            max = pixels[i*4];
    }    

    LOGF("Image max is %f", max);
    max += 0.01f;
    for(int i = 0; i < ctx->plot_width * ctx->plot_height; i++)
    {
        unsigned char col[3];
        col[0] = (uint8_t)(pixels[i * 4]/max*256.0);
        col[1] = (uint8_t)(pixels[i * 4]/max*256.0);
        col[2] = (uint8_t)(pixels[i * 4]/max*256.0);
        fwrite(col, 1, 3, f);
    }
    fclose(f);
    free(pixels);
    return 0;
}
uint32_t bck_gl_mj_end(void *gctx)
{
    bck_gl_mj_ctx_t *ctx = gctx;
    LOG("Deletting GL buffers");
    glDeleteBuffers(1, &ctx->quad_EBO);
    glDeleteBuffers(1, &ctx->quad_VBO);
    glDeleteBuffers(1, &ctx->iteration_histogram_ssbo);
    glDeleteVertexArrays(1, &ctx->quad_VAO);

    LOG("Deletting GL programs");
    glDeleteProgram(ctx->gl_program);

    SUC("Successfully cleaned GL mandelbrot julia backend !");
    return 0;
}

uint32_t bck_gl_mj_ui_control(void *gctx, struct nk_context *nk_ctx)
{
    bck_gl_mj_ctx_t *ctx = gctx;
    plt_params_t *params = &ctx->params;
    plt_win_t *window = &ctx->window;

    plt_params_t old_params = *params;

    //Display current position on complex plane
    nk_layout_row_dynamic(nk_ctx, 10, 1);
    nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "Current position : ");
    nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "%.15f +", window->c_r);
    nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "%.15fi", window->c_i);
    nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "Zoom : %f", window->zoom);
    nk_layout_row_dynamic(nk_ctx, 10, 1);
    nk_layout_row_dynamic(nk_ctx, 20, 1);

    //Maximum iterations control
    if(nk_widget_is_hovered(nk_ctx))
    {
        nk_tooltip(nk_ctx, "Sets the maximum ammount of iterations per-pixel /!\\ Slow if high !");
    }
    int u_iters = (int)params->max_iterations; //Must convert ot int for nuklear
    nk_property_int(nk_ctx, "Max iterations", 10, &u_iters, 500, 5, 1);
    params->max_iterations = u_iters;
    
    //Chose which sets to render
    params->fractal = nk_combo(nk_ctx, FRACTAL_NAMES, 2, params->fractal, 15, nk_vec2(200, 200));

    params->use_moivre = nk_check_label(nk_ctx, "Use moivre exponentation formula (slower)", params->use_moivre);

    if(params->use_moivre)
    {
        nk_property_double(nk_ctx, "Exponent", 1.0, &params->exponent, 5.0, 0.05, 0.05f);
    }else
    {
        int exp = params->exponent; //Must convert to int for nuklear
        nk_property_int(nk_ctx, "Exponent", 1, &exp, 5, 1, 1);
        params->exponent = (double)exp;
    }

    nk_layout_row_dynamic(nk_ctx, 10, 1);
    nk_layout_row_dynamic(nk_ctx, 20, 1);

    //Controls for julia sets
    if(params->fractal == MJ_JULIA_SETS)
    {
        nk_label(nk_ctx, "Julia set constant", NK_TEXT_ALIGN_CENTERED);
        nk_label(nk_ctx, "(z = z^2 + c)", NK_TEXT_ALIGN_LEFT);
        nk_label(nk_ctx, "C =", NK_TEXT_ALIGN_LEFT);
        nk_property_double(nk_ctx, "Real part", -1.0, &params->jrp, 1.0f, 0.01f, 0.05f);
        nk_property_double(nk_ctx, "Imaginary part", -1.0, &params->jip, 1.0f, 0.01f, 0.05f);
    }

    nk_layout_row_dynamic(nk_ctx, 20, 1);
    nk_label(nk_ctx, "Color", NK_TEXT_ALIGN_CENTERED);
    params->color_method = nk_combo(nk_ctx, COLORING_METHODS_NAMES, 3, params->color_method, 10, nk_vec2(200, 200));

    nk_layout_row_dynamic(nk_ctx, 30, 1);
    
    if(nk_combo_begin_image_label(nk_ctx, ctx->color_maps[ctx->selected_color_map], ctx->color_maps_imgs[ctx->selected_color_map], nk_vec2(400, 300)))
    {
        nk_layout_row_dynamic(nk_ctx, 30, 1);
        for(int i = 0; i < ctx->color_maps_count; i++)
        {
            if(ctx->selected_color_map != i)
            {
                if(nk_button_image_label(nk_ctx, ctx->color_maps_imgs[i], ctx->color_maps[i], NK_TEXT_ALIGN_CENTERED))
                {
                    ctx->selected_color_map = i;
                    gl_mj_load_coloring_scheme(gctx, ctx->color_maps[i]);
                    ctx->params.updated = true;
                }
            }
        }
        nk_combo_end(nk_ctx);
    }
   

    nk_layout_row_dynamic(nk_ctx, 10, 1);
    nk_layout_row_dynamic(nk_ctx, 20, 1);

    //Chose the resolution to render at
    nk_label(nk_ctx, "Rendering resolution", NK_TEXT_ALIGN_CENTERED);
    nk_layout_row_dynamic(nk_ctx, 20, 2);
    nk_property_int(nk_ctx, "Width", 10, &ctx->res_width_ui, 7680, 1, 1);
    nk_property_int(nk_ctx, "Height", 10, &ctx->res_height_ui, 4320, 1, 1);
    nk_layout_row_dynamic(nk_ctx, 20, 1);

    //Update resolution
    if(nk_widget_is_hovered(nk_ctx))
    {
        nk_tooltip(nk_ctx, "Changes the rendering texture resolution.");
    }
    if(nk_button_label(nk_ctx, "Update resolution"))
    {
        gl_mj_rendering_resolution(gctx, ctx->res_width_ui, ctx->res_height_ui);
        params->updated = true;
    }

    //Print position
    if(nk_widget_is_hovered(nk_ctx))
    {
        nk_tooltip(nk_ctx, "Prints window position in stdout.");
    }
    if(nk_button_label(nk_ctx, "Print position"))
    {
        LOG("Position :");
        LOGF("%.20f", window->c_r);
        LOGF("%.20fi", window->c_i);
        LOGF("%.20fz", window->zoom);
    }

    //Save GL Texture
    if(nk_widget_is_hovered(nk_ctx))
    {
        nk_tooltip(nk_ctx, "Saves the rendered texture to disk.");
    }
    if(nk_button_label(nk_ctx, "Save GL Texture"))
    {
        bck_gl_mj_plot_save(gctx, "out.ppm");
    }
   
    //Histogram
    nk_layout_row_dynamic(nk_ctx, 20, 1);
    nk_label(nk_ctx, "Iteration histogram", NK_TEXT_ALIGN_CENTERED);
    bool sih_temp = ctx->show_iteration_histogram;
    sih_temp = nk_check_label(nk_ctx, "Show iteration histogram", sih_temp);

    if(sih_temp != ctx->show_iteration_histogram)
    {
        params->updated = true;
    }

    ctx->show_iteration_histogram = sih_temp;
    if(ctx->show_iteration_histogram)
    {
        nk_layout_row_dynamic(nk_ctx, 200, 1);
        int max = 1;
        for(int i = 0; i < params->max_iterations; i++)
        {
            max = ctx->histo[i] > max ? ctx->histo[i] : max;
        }
        if(nk_chart_begin(nk_ctx, NK_CHART_LINES, params->max_iterations, 0.0f, (float)max))
        {
            for(int i = 0; i < params->max_iterations; i++)
            {
                nk_chart_push(nk_ctx, (float)ctx->histo[i]);
            }
            nk_chart_end(nk_ctx);
        }
    }


    if(!plt_params_eq(*params, old_params))
    {
        params->updated = true;
    }

    return 0;
}

uint32_t bck_gl_mj_ui_custom(void *gctx, struct nk_context *nk_ctx)
{
    //No additional windows needed
    return 0;
}

uint32_t bck_gl_mj_mouse_drag(void *gctx, double xoff, double yoff)
{
    bck_gl_mj_ctx_t *ctx = gctx;
    xoff /= ctx->window.zoom / ctx->window.aspect_ratio;
    yoff /= ctx->window.zoom;

    ctx->window.c_r -= xoff;
    ctx->window.c_i += yoff;

    ctx->params.updated = true;
    return 0;
}


uint32_t bck_gl_mj_mouse_zoom(void *gctx, double offset)
{
    bck_gl_mj_ctx_t *ctx = gctx;
    //TODO: Make speed a macro
    ctx->window.zoom += ctx->window.zoom * (offset / 10);

    ctx->params.updated = true;
    return 0;
}

uint32_t bck_gl_mj_mouse_click(void *gctx, enum click_type_t type, enum click_btn_t btn)
{
    if(type == BCK_CLK_DOWN)
    {
        LOG("DOWN");
    }else
    {
        LOG("UP");
    }

    return 0;
}

//Functions inherent to the backend
void gl_mj_load_coloring_scheme(void *gctx, char *filename)
{
    bck_gl_mj_ctx_t *ctx = gctx;

    //Image must be 256 wide, x=0 representing 1 iteration, x=256 representing "max iterations".
    //Only the top row will be read, so the image can be 1 pixel high 

    //Load image using stb_image
    int32_t width;
    int32_t height;
    int32_t n;


    char filename_with_dir[4096] = "";
    strcat(filename_with_dir, COLOR_MAPS_DIR); //TODO: Make safe
    strcat(filename_with_dir, filename);

    uint8_t *data = stbi_load(filename_with_dir, &width, &height, &n, 3);
    ASSERT(data, "Color histogram load failed.");
    ASSERT(width == 256, "Color histogram must be 256 wide.");

    //Allocate memory for loaded texture
    uint32_t *loaded = (uint32_t*)calloc(256, sizeof(uint32_t));

    //Data is y scanlines of x
    //Read top row
    for(int i = 0; i < 256; i++)
    {
        int index = n == 3 ? i * 3 : i * 4;

        uint32_t R = data[index];
        uint32_t G = data[index + 1];
        uint32_t B = data[index + 2];
        //Discard alpha

        loaded[i] = (R << 24) + (G << 16) + (B << 8);
    }

    //Generate texture
    GL_ERR(glDeleteTextures(1, &ctx->coloring_texture_ID));
    GL_ERR(glGenTextures(1, &ctx->coloring_texture_ID));
    GL_ERR(glBindTexture(GL_TEXTURE_2D, ctx->coloring_texture_ID));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_ERR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 1, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, loaded));
    GL_ERR(glGenerateMipmap(GL_TEXTURE_2D));
    stbi_image_free(data);

    free(loaded);
}

void gl_mj_rendering_resolution(void *gctx, uint32_t width, uint32_t height)
{
    bck_gl_mj_ctx_t *ctx = gctx;
    LOGF("Setting gl mandelbrot julia resolution to %dx%d", width, height);
    ctx->plot_height = height;
    ctx->plot_width = width;

    //Create texture
    GL_ERR(glDeleteTextures(1, &ctx->render_texture_ID));

    GL_ERR(glGenTextures(1, &ctx->render_texture_ID));
    GL_ERR(glBindTexture(GL_TEXTURE_2D, ctx->render_texture_ID));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_ERR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL));

    //Set work groups ammount
    ctx->work_groups_width = ceil((float)width / (float)LOCAL_WORK_GROUP_SIZE);
    ctx->work_groups_height = ceil((float)height / (float)LOCAL_WORK_GROUP_SIZE);

    LOGF("  New compute shader work groups width=%d height=%d", ctx->work_groups_width, ctx->work_groups_height);
}

int plt_params_eq(plt_params_t a, plt_params_t b)
{
    if(a.exponent == b.exponent &&
        a.jip == b.jip &&
        a.jrp == b.jrp &&
        a.max_iterations == b.max_iterations &&
        a.updated == b.updated &&
        a.use_moivre == b.use_moivre &&
        a.fractal == b.fractal &&
        a.color_method == b.color_method
        )
    {
        return 1;
    }

    return 0;
}
