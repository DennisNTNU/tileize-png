#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h> // for mkdir()
#include <libgen.h> // for dirname()
#include <math.h> // for log2()

#include "lodepng.h"

// /home/den/2020-12-29-153707_1920x1080_scrot.png
// /home/den/Downloads/m.png

int import_image(unsigned char** image, unsigned int* width, unsigned int* height, const char* path)
{
    LodePNGState state;
    lodepng_state_init(&state);
    state.decoder.zlibsettings.ignore_adler32 = 1;

    //unsigned int ret = lodepng_decode32_file(&image_buffer, &width, &height, argv[1]);

    unsigned char* pngfile = NULL;
    unsigned long int pngsize = 0;
    unsigned int ret = lodepng_load_file(&pngfile, &pngsize, path);
    if (ret != 0)
    {
        printf("error reading image file: %u, %s\n", ret, lodepng_error_text(ret));
        return -1;
    }
    ret = lodepng_decode(image, width, height, &state, pngfile, pngsize);
    if (ret != 0)
    {
        printf("error reading image file: %u, %s\n", ret, lodepng_error_text(ret));
        return -2;
    }
    return 0;
}

void copyTile(unsigned char* image, unsigned int width,
              int x, int y, unsigned char* tile)
{
    for (int j = 0; j < 256; j++)
    {
        for (int i = 0; i < 256; i++)
        {
            unsigned int tile_coord = j*256*4 + i*4;
            unsigned long int image_coord = y*width*256*4 + x*256*4 + j*width*4 + i*4;
            tile[tile_coord + 0] = image[image_coord + 0];
            tile[tile_coord + 1] = image[image_coord + 1];
            tile[tile_coord + 2] = image[image_coord + 2];
            tile[tile_coord + 3] = image[image_coord + 3];
        }
    }
}

void copyTileCheck(unsigned char* image, unsigned int width, unsigned int height,
                   int x, int y, unsigned char* tile)
{
    for (int j = 0; j < 256; j++)
    {
        for (int i = 0; i < 256; i++)
        {
            unsigned int tile_coord = j*256*4 + i*4;
            unsigned long int image_x_coord = x*256 + i;
            unsigned long int image_y_coord = y*256 + j;
            if (image_x_coord >= width || image_y_coord >= height)
            {
                memset(&(tile[tile_coord]), 0, 4);
                continue;
            }
            unsigned long int image_coord = image_y_coord*width*4 + image_x_coord*4;
            tile[tile_coord + 0] = image[image_coord + 0];
            tile[tile_coord + 1] = image[image_coord + 1];
            tile[tile_coord + 2] = image[image_coord + 2];
            //tile[tile_coord + 3] = image[image_coord + 3];
            tile[tile_coord + 3] = 255;
        }
    }
}

void makeTiles(unsigned char* image, unsigned int width, unsigned int height, const char* dir_name)
{
    unsigned char tile_buffer[256*256*4];
    unsigned int tile_count_x = (width + 255) / 256;
    unsigned int tile_count_y = (height + 255) / 256;
    printf("Tile count %i, %ix%i\nTile y: ", tile_count_x*tile_count_y, tile_count_x, tile_count_y);

    for (unsigned int y = 0; y < tile_count_y; y++)
    {
        for (unsigned int x = 0; x < tile_count_x; x++)
        {
            if (x == tile_count_x || y == tile_count_y)
            {
                copyTile(image, width, x, y, tile_buffer);
            }
            else
            {
                copyTileCheck(image, width, height, x, y, tile_buffer);
            }
            char filename[256] = "";
            snprintf(filename, 255, "%s/%i_%i.png", dir_name, x, y);
            lodepng_encode32_file(filename, tile_buffer, 256, 256);
        }
        printf("%i ", y); fflush(stdout);
    }
    printf("\n");
}

void makeMipmap(unsigned char* image, unsigned long int width, unsigned long int height,
                unsigned char** image_mipmap, unsigned long int* width_mipmap, unsigned long int* height_mipmap)
{
    *width_mipmap = width/2;
    *height_mipmap = height/2;

    *image_mipmap = (unsigned char*)malloc(4*(*width_mipmap)*(*height_mipmap));

    printf("Making mipmap\ny: ");
    for (unsigned long int y = 0; y < (*height_mipmap); y++)
    {
        for (unsigned long int x = 0; x < (*width_mipmap); x++)
        {
            unsigned long int image_coord_tl = (2*y + 0)*width*4 + (2*x + 0)*4;
            unsigned long int image_coord_tr = (2*y + 0)*width*4 + (2*x + 1)*4;
            unsigned long int image_coord_bl = (2*y + 1)*width*4 + (2*x + 0)*4;
            unsigned long int image_coord_br = (2*y + 1)*width*4 + (2*x + 1)*4;
            unsigned long int image_mipmap_coord = y*(*width_mipmap)*4 + x*4;
            
            unsigned short red = (unsigned short)image[image_coord_tl + 0] + 
                                 (unsigned short)image[image_coord_tr + 0] + 
                                 (unsigned short)image[image_coord_bl + 0] + 
                                 (unsigned short)image[image_coord_br + 0];
            unsigned short green = (unsigned short)image[image_coord_tl + 1] + 
                                   (unsigned short)image[image_coord_tr + 1] + 
                                   (unsigned short)image[image_coord_bl + 1] + 
                                   (unsigned short)image[image_coord_br + 1];
            unsigned short blue = (unsigned short)image[image_coord_tl + 2] + 
                                  (unsigned short)image[image_coord_tr + 2] + 
                                  (unsigned short)image[image_coord_bl + 2] + 
                                  (unsigned short)image[image_coord_br + 2];
            unsigned short alpha = (unsigned short)image[image_coord_tl + 3] + 
                                   (unsigned short)image[image_coord_tr + 3] + 
                                   (unsigned short)image[image_coord_bl + 3] + 
                                   (unsigned short)image[image_coord_br + 3];

            (*image_mipmap)[image_mipmap_coord + 0] = red / 4;
            (*image_mipmap)[image_mipmap_coord + 1] = green / 4;
            (*image_mipmap)[image_mipmap_coord + 2] = blue / 4;
            (*image_mipmap)[image_mipmap_coord + 3] = alpha / 4;
        }

        //printf("%i ", y); fflush(stdout);
    }

    //printf("\n");
}

int main(int argc, char** argv)
{
    printf("hello png\n");
    if (argc >= 2)
    {
        printf("arg: %s %s\n", argv[0], argv[1]);
        unsigned char* image_buffer = NULL;
        unsigned int width = 0;
        unsigned int height = 0;
        int ret = import_image(&image_buffer, &width, &height, argv[1]);
        if (ret != 0)
        {
            return 0;
        }

        printf("Image is %ux%u\n", width, height);



        int mipmap_levels = 0;
        if (width > height)
        {
            mipmap_levels = log2(width/256) + 2;
        }
        else
        {
            mipmap_levels = log2(height/256) + 2;
        }


        // creating base folder
        char dir_name_base[256] = "";
        snprintf(dir_name_base, 255, "%s/tiles", dirname(argv[1]));
        mkdir(dir_name_base, 0775);

        // creating lowest zoom level folder and saving tiles there
        char dir_name[512] = "";
        snprintf(dir_name, 512, "%s/%i", dir_name_base, mipmap_levels);
        mkdir(dir_name, 0775);
        printf("Saving tiles in %s/\n", dir_name);
        makeTiles(image_buffer, width, height, dir_name);

        // saving tiles for the other zoom/mipmap lavels
        unsigned char* previous_image_buffer_mipmap = image_buffer;
        unsigned long int previous_width_mipmap = width;
        unsigned long int previous_height_mipmap = height;

        for (int i = mipmap_levels-1; i > 0; i--)
        {
            snprintf(dir_name, 512, "%s/%i", dir_name_base, i);
            mkdir(dir_name, 0775);

            unsigned char* image_buffer_mipmap = NULL;
            unsigned long int width_mipmap = 0;
            unsigned long int height_mipmap = 0;
            makeMipmap(previous_image_buffer_mipmap, previous_width_mipmap, previous_height_mipmap,
                &image_buffer_mipmap, &width_mipmap, &height_mipmap);




            printf("Saving tiles in %s/\n", dir_name);

            makeTiles(image_buffer_mipmap, width_mipmap, height_mipmap, dir_name);

            free(previous_image_buffer_mipmap);
            previous_image_buffer_mipmap = image_buffer_mipmap;
            previous_width_mipmap = width_mipmap;
            previous_height_mipmap = height_mipmap;
        }
    }

    return 0;
}