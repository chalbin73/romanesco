//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include<shader.h>
#include <stdint.h>

size_t read_whole_file(FILE* file, char* dest, size_t max)
{
    char v = 0;
    size_t c = 0;
    while(fscanf(file, "%c", &v) == 1 && c < max)
    {
        dest[c] = v;
        c++;
    }
    return c;
}

void compile_shader(uint32_t shad, GLenum shader_type)
{
    LOG("Compiling shader");
    glCompileShader(shad);

    int suc;
    char il[512];
    glGetShaderiv(shad, GL_COMPILE_STATUS, &suc);
    if(!suc)
    {
        glGetShaderInfoLog(shad, 512, NULL, il);
        char* shader_type_name;
        switch (shader_type)
        {
        case GL_VERTEX_SHADER:
            shader_type_name = "Vertex";
            break;
        
        case GL_FRAGMENT_SHADER:
            shader_type_name = "Fragment";
            break;

        case GL_COMPUTE_SHADER:
            shader_type_name = "Compute";
            break;

        default:
            assert(0 && "Unreachable");
            break;
        }
        FAILF("ERROR:%s shader compilation failed:\n%s", shader_type_name, il);
    }
    SUC("Shader compile success !");
}

uint32_t make_shader(char* vert, char* frag)
{
    LOG("Reading shader source");
    //Read shaders
    FILE* ff = fopen(frag, "r");
    FILE* vf = fopen(vert, "r");

    char* vert_source = (char*)malloc(TEMP_SOURCE_BUFFER);
    char* frag_source = (char*)malloc(TEMP_SOURCE_BUFFER);

    const int frag_count = read_whole_file(ff, frag_source, TEMP_SOURCE_BUFFER);
    const int vert_count = read_whole_file(vf, vert_source, TEMP_SOURCE_BUFFER);

    fclose(ff);
    fclose(vf);

    LOG("Creating GL shader object");
    uint32_t f = glCreateShader(GL_FRAGMENT_SHADER);
    uint32_t v = glCreateShader(GL_VERTEX_SHADER);

    const char *frag_source_const = frag_source;
    const char *vert_source_const = vert_source;

    glShaderSource(f, 1, &frag_source_const, &frag_count);
    glShaderSource(v, 1, &vert_source_const, &vert_count);

    free(frag_source);
    free(vert_source);

    compile_shader(f, GL_FRAGMENT_SHADER);
    compile_shader(v, GL_VERTEX_SHADER);

    LOG("Linking shaders");    
    uint32_t p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);

    glDeleteShader(v);
    glDeleteShader(f);

    SUC("Shader creation successfull !");
    
    return p;
}

uint32_t make_compute_shader(char* path)
{
    LOGF("Loading compute shader %s", path);
    print_work_group_capabilities();

    int cs = glCreateShader(GL_COMPUTE_SHADER);

    FILE* fp = fopen(path, "r");
    if(!fp)
    {
        FAIL("GL shader open failed !");
    }
    char* source = (char*)malloc(TEMP_SOURCE_BUFFER);
    assert(source);

    LOG("Reading source");
    size_t s = read_whole_file(fp, source, TEMP_SOURCE_BUFFER);


    LOG("Loading source");

    const char *sourc_const = source;
    const int s_const = s;
    glShaderSource(cs, 1, &sourc_const, &s_const);

    LOG("Compiling compute shader");
    compile_shader(cs, GL_COMPUTE_SHADER);

    LOG("Linking compute shader");
    uint32_t prog = glCreateProgram();

    glAttachShader(prog, cs);
    glLinkProgram(prog);
    glDeleteShader(cs);

    return prog; 
}

void print_work_group_capabilities() {
    int workgroup_count[3];
    int workgroup_size[3];
    int workgroup_invocations;

    LOG("Device compute capabilities :")

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workgroup_count[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workgroup_count[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workgroup_count[2]);

    LOGF("   Workgroups max size:\n\tx:%u\n\ty:%u\n\tz:%u\n",
    workgroup_size[0], workgroup_size[1], workgroup_size[2]);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workgroup_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workgroup_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workgroup_size[2]);

    LOGF("   Local workgroup max size:\n\tx:%u\n\ty:%u\n\tz:%u\n",
    workgroup_size[0], workgroup_size[1], workgroup_size[2]);

    glGetIntegerv (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroup_invocations);
    LOGF("   Max workgroup instantiations:\n\t%u\n", workgroup_invocations);
}