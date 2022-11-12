//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <util.h>


// float map(float v, float v_min, float v_max, float out_min, float out_max)
// {
//     return (((v - v_min) / (v_max - v_min)) * (out_max - out_min)) + out_min;
// }

double map(double v, double v_min, double v_max, double out_min, double out_max)
{
    return (((v - v_min) / (v_max - v_min)) * (out_max - out_min)) + out_min;
}

char** get_files_in_dir(char* dir_path, size_t *filecount)
{
    size_t c = 0;

    DIR *d;
    struct dirent *dir;

    //Count files in dir
    d = opendir(dir_path);
    if(!d)
        return NULL;

    while ((dir = readdir(d)) != NULL)
    {
        if(dir->d_type != DT_DIR)
            c++;
    }
    closedir(d);

    char **result = xmalloc(sizeof(char*) * c);

    //Keep file names
    d = opendir(dir_path);
    if(!d)
        return NULL;

    int i = 0;
    while ((dir = readdir(d)) != NULL)
    {
        if(dir->d_type != DT_DIR)
        {
            result[i] = xmalloc(strlen(dir->d_name) * sizeof(char));
            strcpy(result[i], dir->d_name);
            i++;
        }else
        {
            continue;
        }
    }
    closedir(d);

    *filecount = c;
    return result;
}

uint32_t load_image_gl_rgb(char* image_path)
{
    //Load to memory
    int x, y, n;
    unsigned char *data = stbi_load(image_path, &x, &y, &n, 3);
    ASSERT(data, "Image load error !");
    ASSERT(n == 3 , "Wrong image format loaded !");

    //int format = n == 3 ? GL_RGB : GL_RGBA;

    //Generate texture
    GLuint tex = 0;

    //Load texture
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return tex;
}