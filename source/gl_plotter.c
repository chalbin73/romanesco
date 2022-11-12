//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <gl_plotter.h>
#include <stdint.h>
#include <sys/types.h>

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

// Needs a valid OpenGL context
void gl_plotter_init(void *gctx, uint32_t width, uint32_t height)
{
    gl_plotter_ctx_t *ctx = gctx;
    LOG("Initializing gl_plotter");
    LOG("Creating screen quad");
    //Creating rendering quad
    GL_ERR(glGenVertexArrays(1, &ctx->quad_VAO));
    GL_ERR(glBindVertexArray(ctx->quad_VAO));


    LOG("Creating quad VAO/VBO");
    GL_ERR(glGenBuffers(1, &ctx->quad_VBO));
    GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, ctx->quad_VBO));
    GL_ERR(glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad), screen_quad, GL_STATIC_DRAW));
    GL_ERR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0));
    GL_ERR(glEnableVertexAttribArray(0));

    LOG("Creating quad EBO");
    GL_ERR(glGenBuffers(1, &ctx->quad_EBO));
    GL_ERR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quad_EBO));
    GL_ERR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    LOG("Creating histogram SSBO");
    glGenBuffers(1, &ctx->iteration_histogram_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->iteration_histogram_ssbo);
    //Init buffer with zeros
    uint32_t *data = (uint32_t*)calloc(HISTO_SSBO_LENGTH, sizeof(uint32_t));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * HISTO_SSBO_LENGTH, data, GL_DYNAMIC_DRAW);
    free(data);

    glGenBuffers(1, &ctx->total_histogram_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->total_histogram_ssbo);
    uint32_t zero = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t), &zero, GL_DYNAMIC_DRAW);

    LOG("Setting viewport");
    //Setting viewport
    ctx->ctx_width = width;
    ctx->ctx_height = height;
    GL_ERR(glViewport(0, 0, width, height));

    LOG("Loading program/shader");
    //Preparing program
    GL_ERR(ctx->gl_program = make_shader("gl_shaders/fractals.vert", "gl_shaders/fractals.frag"));

    //Preparing compute shader
    GL_ERR(ctx->compute_program = make_compute_shader("gl_shaders/fractal.comp"));

    LOG("Getting uniform loactions");

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

    gl_plotter_resolution(ctx, width, height);
}

void gl_plotter_load_coloring_scheme(void *gctx, char *filename)
{
    gl_plotter_ctx_t *ctx = gctx;

    //Image must be 256 wide, x=0 representing 1 iteration, x=256 representing "max iterations".
    //Only the top row will be read, so the image can be 1 pixel high 

    //Load image using stb_image
    int32_t width;
    int32_t height;
    int32_t n;
    uint8_t *data = stbi_load(filename, &width, &height, &n, 0);
    ASSERT(data, "Color histogram load failed.");
    ASSERT(width == 256, "Color histogram must be 256 wide.");
    ASSERT(n >= 3, "Image must have between 3 and 4 components (RGB or RGBA).");

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
    GL_ERR(glGenTextures(1, &ctx->coloring_texture_ID));
    GL_ERR(glBindTexture(GL_TEXTURE_2D, ctx->coloring_texture_ID));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_ERR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_ERR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 1, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, loaded));
    stbi_image_free(data);

    free(loaded);

}

//This function changes the opengl viewport size (for resizing and keeping consistent aspect ratio)
void gl_plotter_viewport_size(void *gctx, uint32_t width, uint32_t height)
{
    gl_plotter_ctx_t *ctx = gctx;
    ctx->ctx_width = width;
    ctx->ctx_height = height;
    GL_ERR(glViewport(0, 0, width, height));
}

//This function changes the resolution of the texture on which the actual rendering is done
void gl_plotter_resolution(void *gctx, uint32_t width, uint32_t height)
{
    gl_plotter_ctx_t *ctx = gctx;
    LOGF("Changing gl plotter resolution to %dx%d", width, height);
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

    LOGF("Compute shader work groups width=%d height=%d", ctx->work_groups_width, ctx->work_groups_height);
}

// Needs a valid OpenGL context
void gl_plotter_render(void *gctx, struct plot_params_t *params)
{
    gl_plotter_ctx_t *ctx = gctx;
    //Renders the fractal to the texture
    if(params->updated)
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
        if(tm - ctx->last_redraw > 30)
        {
            GL_ERR(glUseProgram(ctx->compute_program))
            GL_ERR(
                //Update uniforms
                glUniform1f(ctx->rp_loc, params->rp);
                glUniform1f(ctx->ip_loc, params->ip);
                glUniform1d(ctx->r_loc, params->window.c_r);
                glUniform1d(ctx->i_loc, params->window.c_i);
                glUniform1d(ctx->z_loc, params->window.zoom);
                glUniform1d(ctx->ar_loc, params->window.aspect_ratio);
                glUniform1i(ctx->it_loc, params->iterations);
                glUniform1d(ctx->e_loc, params->exponent);
                glUniform1i(ctx->width_loc, ctx->plot_width);
                glUniform1i(ctx->height_loc, ctx->plot_height);
                glUniform1i(ctx->fractal_loc, params->fractal);
                glUniform1i(ctx->m_loc, params->use_moivre);
                glUniform1i(ctx->pass_loc, 0);
            );

            GL_ERR(
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->total_histogram_ssbo);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ctx->total_histogram_ssbo);

                //ZEROING histogram total
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

            GL_ERR(
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->iteration_histogram_ssbo);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ctx->iteration_histogram_ssbo);       

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ctx->total_histogram_ssbo);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ctx->total_histogram_ssbo);
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
    )
}

void gl_plotter_save_current_texture(void *gctx, char* filepath)
{
    gl_plotter_ctx_t *ctx = gctx;
    LOGF("Saving texture %dx%d", ctx->plot_width, ctx->plot_height);
    float *pixels = (float*)malloc(ctx->plot_width * ctx->plot_height * sizeof(float) * 4);
    glBindTexture(GL_TEXTURE_2D, ctx->render_texture_ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

    FILE* f = fopen(filepath, "w");
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
}

// Needs a valid OpenGL context
void gl_plotter_clean(void *gctx)
{
    gl_plotter_ctx_t *ctx = gctx;
    LOG("Deletting buffers");
    glDeleteBuffers(1, &ctx->quad_EBO);
    glDeleteBuffers(1, &ctx->quad_VBO);
    glDeleteBuffers(1, &ctx->iteration_histogram_ssbo);
    glDeleteVertexArrays(1, &ctx->quad_VAO);

    LOG("Deletting programs");
    glDeleteProgram(ctx->gl_program);
}