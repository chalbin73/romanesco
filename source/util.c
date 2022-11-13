//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <stdint.h>
#include <string.h>
#include <util.h>


// float map(float v, float v_min, float v_max, float out_min, float out_max)
// {
//     return (((v - v_min) / (v_max - v_min)) * (out_max - out_min)) + out_min;
// }

double map(double v, double v_min, double v_max, double out_min, double out_max)
{
    return (((v - v_min) / (v_max - v_min)) * (out_max - out_min)) + out_min;
}

size_t get_file_count_in_dir(char* dir_path)
{
    size_t count = 0;
    tinydir_dir dir;
    tinydir_open(&dir, dir_path);

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (!file.is_dir)
        {
            count++;
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);
    return count;
}

char **get_files_in_dir(char* dir_path, size_t *filecount)
{
    size_t count = get_file_count_in_dir(dir_path);

    //allocate some memory for names
    char **result = xmalloc(sizeof(char*) * count);

    tinydir_dir dir;
    tinydir_open(&dir, dir_path);
    uint32_t i = 0;
    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (!file.is_dir)
        {
            result[i] = xmalloc(sizeof(char) * strlen(file.name));
            strcpy(result[i], file.name); //TODO: Make safer.
            i++;
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);

    *filecount = count;

    return result;
}

/* char** get_files_in_dir(char* dir_path, size_t *filecount)
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
} */

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