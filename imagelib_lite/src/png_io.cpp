/*!
 * \file png_io.cpp
 * \brief Loading and saving of PNG images
 */
   

#include "png_io.h"
#include "image.h"
#include "ibinary.h"
//#include "paletted_image.h"

#ifndef NO_PNG

#include <png.h>

#include <stdio.h>
#include <cassert>
#include <algorithm>


namespace image
{

//from libpng v1.4.0 these defines were removed from the library
#if PNG_LIBPNG_VER >= 10400
#define png_infopp_NULL (png_infopp)NULL
#define png_bytepp_NULL (png_bytepp)NULL
#define int_p_NULL (int*)NULL
#endif

int const BIT_DEPTH_8 = 8;
int const BIT_DEPTH_1 = 1;

/*!Loads an image file as a IMAGE
 * \remarks It supports images with the following colorspaces:
 *  RGB, RGBA, GRAYSCALE (1 or 8 bits per pixel), GRAYSCALEALPHA \n
 *  Currently it does not support paletted images or any type of interlacing.\n
 *  Bit depths of 16 will be stripped down to 8.\n
 * \param[in] filename name of the output file
 * \pre filename!=NULL
 * \return NULL if load operation failed, the corresponding IMAGE if successful
 */
IMAGE *LoadPNG(const char *filename)
{
    assert(filename!=NULL);

    IMAGE *img;
    FILE *infile;
    
    png_structp png_ptr;
    png_infop info_ptr;
    
    png_uint_32 width, height, res_x, res_y;

    int bit_depth, color_type, interlace_type, res_unit_type;
    
    if ((infile = fopen(filename, "rb")) == NULL)
       return NULL;
    
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL)
    {
        fclose(infile);
        return NULL;
    }
    
   info_ptr = png_create_info_struct(png_ptr);
   
    if (info_ptr == NULL)
    {
        fclose(infile);
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return NULL;
    }


    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(infile);
        return NULL;
    }
   
    png_init_io(png_ptr, infile);
   
    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
           &interlace_type, int_p_NULL, int_p_NULL);

    if (png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &res_unit_type)==0)
    {
        res_x = res_y = 0;
        res_unit_type = PNG_RESOLUTION_UNKNOWN;
    }


    if (color_type==PNG_COLOR_TYPE_RGB)
    {
        img = MallocImage((int)width, (int)height, RGB, false);
    }
    else if (color_type==PNG_COLOR_TYPE_RGB_ALPHA)
    {
        img = MallocImage((int)width, (int)height, RGB, true);
    }
    else if (color_type==PNG_COLOR_TYPE_GRAY)
    {
        img = MallocImage((int)width, (int)height, GRAYSCALE, false);
    }
    else if (color_type==PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        img = MallocImage((int)width, (int)height, GRAYSCALE, true);
    }
    else if (color_type==PNG_COLOR_TYPE_PALETTE)
    {
       img = MallocImage((int)width, (int)height, RGB, false);
       png_set_palette_to_rgb(png_ptr);
    }
    else
    {
        printf("png_io load:cannot handle colorspace\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(infile);
        return NULL;
    }

    /* Standardize the bit_depth to 8 bits */
    if (bit_depth == 16)
    {
        png_set_strip_16(png_ptr);
    }
    else if (bit_depth<8)
    {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    img->dpi = 0;

    if(res_unit_type==PNG_RESOLUTION_METER)
    {
        if (res_x == res_y)
        {
            img->dpi = (int)(res_x*2.54/100.0+0.5);
        }
        else
        {
            img->dpi = (int)(std::min(res_x, res_y)*2.54/100.0+0.5);
        }
    }
    else //PNG_RESOLUTION_UNKNOWN, consider inches as default
    {
        if (res_x == res_y)
        {
            img->dpi = (int)(res_x+0.5);
        }
        else
        {
            img->dpi = (int)(std::min(res_x, res_y)+0.5);
        }
    }

//    int number_passes = png_set_interlace_handling(png_ptr);

    png_read_update_info(png_ptr, info_ptr);


    png_bytep *row_pointers= new png_bytep[height];

    for (int y=0; y<(int)height; y++)
        row_pointers[y] = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));

    png_read_image(png_ptr, row_pointers);
       
    for (int y=0; y<(int)height; y++)
    {
        png_bytep p = row_pointers[y];
       
        for (int x=0; x<img->xdim; x++)
        {
            for (int c=0; c<img->cdim; c++)
                img->color[c][y][x] = *p++;
            if (img->HasAlpha())
                img->alpha[y][x] = *p++;
        }
     }

    png_read_end(png_ptr, info_ptr);
   
    for (int y=0; y<(int)height; y++)
        png_free(png_ptr, row_pointers[y]);
                       
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
   
    delete[] row_pointers;

    fclose(infile);

    return img;
}

/*!Saves the given (valid) image into the specified file
 * \remarks It supports images with the following colorspaces:
 *  RGB, RGBA, GRAYSCALE, GRAYSCALEALPHA \n
 * \param[in] img the image to save
 * \param[in] filename name of the output file
 * \param[in] numbit no. of bits per pixel. Valid options are 8 or 1 for binary images
 *            in the GRAYSCALE colorspace
 * \pre img!=NULL points to a valid IMAGE object, filename!=NULL
 * \return 0 if successful, an eror code!=0 if the save operation failed
 */
int  SavePNG(const IMAGE *img, const char *filename, int numbit)
{
    assert(img!=NULL);
    assert(filename!=NULL);

    if (img == NULL)
        return -1;

    //check that we can handle the requested format before starting the write
    int color_type;

    switch(img->colorspace)
    {
        case RGB:
            if (!img->HasAlpha())
                color_type = PNG_COLOR_TYPE_RGB;
            else
                color_type = PNG_COLOR_TYPE_RGB_ALPHA;
            break;
        case GRAYSCALE:
            if (!img->HasAlpha())
                color_type = PNG_COLOR_TYPE_GRAY;
            else
                color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
            break;
        default:
            printf("png_io save:cannot handle colorspace\n");
            fflush(stdout);
            return -2;
    }

    //atm we only support 1 or 8 bits per pixel
    if(!((numbit == BIT_DEPTH_1) || (numbit == BIT_DEPTH_8)))
    {
        printf("png_io save:number of bits per pixel not valid\n");
        fflush(stdout);
        return -3;
    }

    //1 bit per pixel only allowed for binary images
    if(numbit == BIT_DEPTH_1)
    {
        if(!IsBinaryImage(img))
        {
            printf("png_io save:number of bits per pixel not valid\n");
            fflush(stdout);
            return -4;
        }
    }

    //start the actual writing in the output file
    FILE *outfile;

    png_structp png_ptr;
    png_infop info_ptr;
    png_byte *row_buff = NULL;
    unsigned char *p;

    if ((outfile = fopen(filename, "wb")) == NULL)
        return (-5);

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL)
    {
        fclose(outfile);
        return (-6);
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL)
    {
        fclose(outfile);
        png_destroy_write_struct(&png_ptr, png_infopp_NULL);
        return (-7);
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(outfile);
        return (-8);
    }

    png_init_io(png_ptr, outfile);

    if(numbit == BIT_DEPTH_1)
    {
        png_set_IHDR(png_ptr, info_ptr, img->xdim, img->ydim, BIT_DEPTH_1, color_type, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    }
    else
        if (numbit == BIT_DEPTH_8)
            png_set_IHDR(png_ptr, info_ptr, img->xdim, img->ydim, BIT_DEPTH_8, color_type, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    int res_x_dpc = (int)(img->dpi/2.54+0.5);

    png_set_pHYs(png_ptr, info_ptr, res_x_dpc, res_x_dpc, PNG_RESOLUTION_METER);

    png_write_info(png_ptr, info_ptr);

    if(img->HasAlpha())
        row_buff = (png_byte*) malloc((img->cdim+1)*img->xdim*sizeof(png_bytep));
    else
        row_buff = (png_byte*) malloc((img->cdim)*img->xdim*sizeof(png_bytep));

    for(int y = 0; y < img->ydim; y++)
    {
        p=row_buff;
        if (img->cdim==3)
        {
            if(img->HasAlpha())
            {
                for (int x=0; x<img->xdim; x++)
                {
                   *p++ = img->color[0][y][x];
                   *p++ = img->color[1][y][x];
                   *p++ = img->color[2][y][x];
                   *p++ = img->alpha[y][x];
                }
            }
            else
            {
                for (int x=0; x<img->xdim; x++)
                {
                   *p++ = img->color[0][y][x];
                   *p++ = img->color[1][y][x];
                   *p++ = img->color[2][y][x];
                }
            }
        }
        else if (img->cdim==1 && numbit == BIT_DEPTH_8)
        {
            if(img->HasAlpha())
            {
                for (int x=0; x<img->xdim; x++)
                {
                   *p++ = img->gray[y][x];
                   *p++ = img->alpha[y][x];
                }
            }
            else
            {
                for (int x=0; x<img->xdim; x++)
                {
                   *p++ = img->gray[y][x];
                }
            }
        }
        else if (img->cdim==1 && numbit == BIT_DEPTH_1)
        {
            int x;
            unsigned char tmp;

            for (x=0; x<img->xdim-7; x+=8,p++)
                for (*p=0, tmp=0;tmp<8;tmp++)
                    (*p)|=(unsigned char)((img->gray[y][x+tmp]&1)<<(7-tmp));
            if ((tmp=(img->xdim&7)))
                for (*p=0,x=0;x<tmp;x++)
                    (*p)|=(unsigned char)((img->gray[y][img->xdim+x-tmp]&1)<<(7-x));

        }

        png_write_rows(png_ptr, &row_buff, 1);
    }

    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(outfile);

    if(row_buff)
        free(row_buff);

    return 0;
}

}

#endif //ndef NO_PNG
