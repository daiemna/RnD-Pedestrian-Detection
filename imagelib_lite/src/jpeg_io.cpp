/*!
 * \file jpeg_io.cpp
 * \brief Loading and saving of JPEG images
 */
   

#include "jpeg_io.h"
#include "image.h"

#ifndef NO_JPEG

extern "C" {
#include <jpeglib.h>
}

#include <cassert>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#ifndef NO_LCMS
#include <lcms2.h>
#endif


namespace image
{

struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf               setjmp_buffer;
};
    

void imglib_error_exit(j_common_ptr cinfo)
{
    my_error_mgr *myerr = (my_error_mgr*) cinfo->err;
    (*cinfo->err->output_message)(cinfo);     /* display error message */
    
    longjmp(myerr->setjmp_buffer, 1);         /* return from error */
}


#ifndef NO_LCMS
/*
 * Since an ICC profile can be larger than the maximum size of a JPEG marker
 * (64K), we need provisions to split it into multiple markers.  The format 
 * defined by the ICC specifies one or more APP2 markers containing the
 * following data:
 *      Identifying string      ASCII "ICC_PROFILE\0"  (12 bytes)
 *      Marker sequence number  1 for first APP2, 2 for next, etc (1 byte)
 *      Number of markers       Total number of APP2's used (1 byte)
 *      Profile data            (remainder of APP2 data)
 * Decoders should use the marker sequence numbers to reassemble the profile,
 * rather than assuming that the APP2 markers appear in the correct sequence.
 */
   
#define ICC_MARKER  (JPEG_APP0 + 2)     /* JPEG marker code for ICC */
#define ICC_OVERHEAD_LEN  14            /* size of non-profile data in APP2 */
#define MAX_BYTES_IN_MARKER  65533      /* maximum data len of a JPEG marker */
#define MAX_DATA_BYTES_IN_MARKER  (MAX_BYTES_IN_MARKER - ICC_OVERHEAD_LEN)


/*
 * Handy subroutine to test whether a saved marker is an ICC profile marker.
 */

static boolean marker_is_icc(jpeg_saved_marker_ptr marker)
{
    return (marker->marker == ICC_MARKER && 
        marker->data_length >= ICC_OVERHEAD_LEN &&
        /* verify the identifying string */
        GETJOCTET(marker->data[0]) == 0x49 &&
        GETJOCTET(marker->data[1]) == 0x43 &&
        GETJOCTET(marker->data[2]) == 0x43 &&
        GETJOCTET(marker->data[3]) == 0x5F &&
        GETJOCTET(marker->data[4]) == 0x50 &&
        GETJOCTET(marker->data[5]) == 0x52 &&
        GETJOCTET(marker->data[6]) == 0x4F &&
        GETJOCTET(marker->data[7]) == 0x46 &&
        GETJOCTET(marker->data[8]) == 0x49 &&
        GETJOCTET(marker->data[9]) == 0x4C &&
        GETJOCTET(marker->data[10]) == 0x45 && 
        GETJOCTET(marker->data[11]) == 0x0);
}


/*
 * See if there was an ICC profile in the JPEG file being read;
 * if so, reassemble and return the profile data.
 *
 * true is returned if an ICC profile was found, false if not.
 * If true is returned, *icc_data_ptr is set to point to the
 * returned data, and *icc_data_len is set to its length.
 *
 * IMPORTANT: the data at **icc_data_ptr has been allocated with malloc()
 * and must be freed by the caller with free() when the caller no longer
 * needs it.  (Alternatively, we could write this routine to use the
 * IJG library's memory allocator, so that the data would be freed implicitly
 * at jpeg_finish_decompress() time.  But it seems likely that many apps
 * will prefer to have the data stick around after decompression finishes.)
 *
 * NOTE: if the file contains invalid ICC APP2 markers, we just silently
 * return false.  You might want to issue an error message instead.
 */

bool read_icc_profile(j_decompress_ptr dinfo, JOCTET ** icc_data_ptr, unsigned int *icc_data_len)
{
    jpeg_saved_marker_ptr marker;

    int num_markers = 0;

    int seq_no;

    JOCTET *icc_data;

    unsigned int total_length;

#define MAX_SEQ_NO  255         /* sufficient since marker numbers are bytes */
    char marker_present[MAX_SEQ_NO + 1];        /* 1 if marker found */

    unsigned int data_length[MAX_SEQ_NO + 1];   /* size of profile data in marker */

    unsigned int data_offset[MAX_SEQ_NO + 1];   /* offset for data in marker */

    *icc_data_ptr = NULL;       /* avoid confusion if false return */
    *icc_data_len = 0;

    /* This first pass over the saved markers discovers whether there are
     * any ICC markers and verifies the consistency of the marker numbering.
     */

    for (seq_no = 1; seq_no <= MAX_SEQ_NO; seq_no++)
        marker_present[seq_no] = 0;

    for (marker = dinfo->marker_list; marker != NULL; marker = marker->next)
    {
        if (marker_is_icc(marker))
        {
            if (num_markers == 0)
                num_markers = GETJOCTET(marker->data[13]);
            else if (num_markers != GETJOCTET(marker->data[13]))
                return false;   /* inconsistent num_markers fields */
            seq_no = GETJOCTET(marker->data[12]);
            if (seq_no <= 0 || seq_no > num_markers)
                return false;   /* bogus sequence number */
            if (marker_present[seq_no])
                return false;   /* duplicate sequence numbers */
            marker_present[seq_no] = 1;
            data_length[seq_no] = marker->data_length - ICC_OVERHEAD_LEN;
        }
    }

    if (num_markers == 0)
        return false;

    /* Check for missing markers, count total space needed,
     * compute offset of each marker's part of the data.
     */

    total_length = 0;
    for (seq_no = 1; seq_no <= num_markers; seq_no++)
    {
        if (marker_present[seq_no] == 0)
            return false;       /* missing sequence number */
        data_offset[seq_no] = total_length;
        total_length += data_length[seq_no];
    }

    if (total_length <= 0)
        return false;           /* found only empty markers? */

    /* Allocate space for assembled data */
    icc_data = (JOCTET *) malloc(total_length * sizeof(JOCTET));
    if (icc_data == NULL)
        return false;           /* oops, out of memory */

    /* and fill it in */
    for (marker = dinfo->marker_list; marker != NULL; marker = marker->next)
    {
        if (marker_is_icc(marker))
        {
            JOCTET FAR *src_ptr;

            JOCTET *dst_ptr;

            unsigned int length;

            seq_no = GETJOCTET(marker->data[12]);
            dst_ptr = icc_data + data_offset[seq_no];
            src_ptr = marker->data + ICC_OVERHEAD_LEN;
            length = data_length[seq_no];
            while (length--)
            {
                *dst_ptr++ = *src_ptr++;
            }
        }
    }

    *icc_data_ptr = icc_data;
    *icc_data_len = total_length;

    return true;
}
#endif            

/* Common part of SaveJPEG and  LoadJPEG and LoadJPEGfromMemory
 * \return NULL if load operation failed, pointer to the image if successful
 */
IMAGE *LoadJPEGint(struct jpeg_decompress_struct *dinfo)
{
    IMAGE *img;

//    for (int m = 0; m < 16; m++)
//        jpeg_save_markers(&Decompressor, JPEG_APP0 + m, 0xFFFF);
                        
#ifndef NO_LCMS    
    jpeg_save_markers(dinfo, ICC_MARKER, 0xFFFF);
#endif

    (void) jpeg_read_header(dinfo, TRUE);
    (void) jpeg_start_decompress(dinfo);

    if (dinfo->jpeg_color_space==JCS_RGB)
        img = MallocImage(dinfo->image_width, dinfo->image_height, RGB, false);
    else if (dinfo->jpeg_color_space==JCS_GRAYSCALE)
        img = MallocImage(dinfo->image_width, dinfo->image_height, GRAYSCALE, false);
    else if (dinfo->jpeg_color_space==JCS_CMYK)
          img = MallocImage(dinfo->image_width, dinfo->image_height, CMYK, false);
    else
    {
        img = MallocImage(dinfo->image_width, dinfo->image_height, RGB, false);
        dinfo->out_color_space = JCS_RGB;
    }
    
    if (img==NULL) 
        return NULL;
    
    img->dpi = 0;
    if (dinfo->saw_JFIF_marker && (dinfo->density_unit != 0))
    {
        unsigned xres = dinfo->X_density;
        unsigned yres = dinfo->Y_density;
       
        if (xres == yres)
        {
            if (dinfo->density_unit == 1)  //directly in dpi ?
              img->dpi = ((int)xres);
            else
              img->dpi = ((int)(xres*2.54+0.5));  //res. unit is in cm, so convert it
        }
    }

#ifndef NO_LCMS
    // read ICC profile
    cmsUInt32Number EmbedLen;
    cmsUInt8Number* EmbedBuffer;
                      
    if (read_icc_profile(dinfo, &EmbedBuffer, &EmbedLen)==true)
    {
        img->icc = cmsOpenProfileFromMem(EmbedBuffer, EmbedLen);
        free(EmbedBuffer);
    }
#endif

    int row_stride = dinfo->output_width * dinfo->output_components; /* physical row width in output buffer */
    JSAMPARRAY buffer = (*dinfo->mem->alloc_sarray)((j_common_ptr) dinfo, JPOOL_IMAGE, row_stride, 1); /* Output row buffer */

    while (dinfo->output_scanline < dinfo->output_height) 
    {
        (void) jpeg_read_scanlines(dinfo, buffer, 1);
        int y = dinfo->output_scanline-1;
        unsigned char *p = *buffer;
        if (img->cdim==3)
            for (int x=0; x<img->xdim; x++) 
            {
               img->color[0][y][x] = (int)*(p++);
               img->color[1][y][x] = (int)*(p++); 
               img->color[2][y][x] = (int)*(p++);
            }
        else
            if (img->cdim==4)
                for(int x=0; x<img->xdim; x++) 
                {
                    img->color[0][y][x] = 255 - (int)*(p++);
                    img->color[1][y][x] = 255 - (int)*(p++); 
                    img->color[2][y][x] = 255 - (int)*(p++);
                    img->color[3][y][x] = 255 - (int)*(p++);
                }
        else
            for (int x=0; x<img->xdim; x++,p++)
                img->gray[y][x]=*p;   
    }
    (void) jpeg_finish_decompress(dinfo);

    return img;
}

/*!Loads an JPEG-Image from file. This function is intended to supports all
 * usual JPEG-formats. Up to now it needs improvement on the
 * handling and interpretation of metadata.
 * \param[in] filename name of the input file
 * \return NULL if load operation failed, pointer to the image if successful
 */
IMAGE *LoadJPEG(const char *filename)
{
    struct jpeg_decompress_struct dinfo;
    struct my_error_mgr jerr;
    IMAGE *img;

    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) 
        return NULL;
    
    dinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit     = imglib_error_exit;
        
    if (setjmp(jerr.setjmp_buffer)) 
    {
         /* if we're here, it blowed up... */
         jpeg_destroy_decompress(&dinfo);
         fclose(infile);
         return NULL;
    }
                                              
  //  dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);
    jpeg_stdio_src(&dinfo, infile);

//    for (int m = 0; m < 16; m++)
//        jpeg_save_markers(&Decompressor, JPEG_APP0 + m, 0xFFFF);

    img = LoadJPEGint(&dinfo);                        
                        
    jpeg_destroy_decompress(&dinfo);

    fclose(infile);
    return img;
}

/*!Loads an JPEG-Image from memory buffer. This function is intended to supports all
 * usual JPEG-formats. Up to now it needs improvement on the
 * handling and interpretation of metadata.
 * \param[in] inbuffer pointer to the input buffer
 * \param[in] insize size of input buffer
 * \return NULL if load operation failed, pointer to the image if successful
 */
IMAGE *LoadJPEGfromMemory(unsigned char *inbuffer, unsigned long insize)
{
    struct jpeg_decompress_struct dinfo;
    struct my_error_mgr jerr;
    IMAGE *img;

    dinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit     = imglib_error_exit;
        
    if (setjmp(jerr.setjmp_buffer)) 
    {
         /* if we're here, it blowed up... */
         jpeg_destroy_decompress(&dinfo);
         return NULL;
    }
                                              
  //  dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);
    jpeg_mem_src(&dinfo, inbuffer, insize);

    img = LoadJPEGint(&dinfo);                        
                        
    jpeg_destroy_decompress(&dinfo);

    return img;
}

/*!Returns the width, height and the scanning resolution for the given JPEG image file name.
 * In case of an error (e.g. inexistent file, invalid header, etc.), all return values are 
 * set to -1. 
 */
void JPEGInfo(const char *filename, int& width, int& height, int& dpi)
{
    const int UNKNOWN_VALUE=-1;
    
    struct jpeg_decompress_struct dinfo;
    struct my_error_mgr jerr;
    FILE *infile;

    infile = fopen(filename, "rb");
    if (infile == NULL) 
    {
        width = height = dpi = UNKNOWN_VALUE;
        return;
    } 
 
    dinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit     = imglib_error_exit;
      
    if (setjmp(jerr.setjmp_buffer)) 
    {
        // if we're here, it blowed up... 
        jpeg_destroy_decompress(&dinfo);
        fclose(infile);
        width = height = dpi = UNKNOWN_VALUE;
        return;
    }

    jpeg_create_decompress(&dinfo);
    jpeg_stdio_src(&dinfo, infile);

    (void) jpeg_read_header(&dinfo, TRUE);
    
    width = dinfo.image_width;
    height = dinfo.image_height;
    dpi = 0;
    if (dinfo.saw_JFIF_marker && (dinfo.density_unit != 0))
    {
        unsigned xres=dinfo.X_density;
        unsigned yres=dinfo.Y_density;
         
        if (xres == yres)
        {
            if (dinfo.density_unit == 1)  //directly in dpi ?
                dpi=((int)xres);
            else
                dpi=((int)(xres*0.3937008));  //res. unit is in cm, so convert it
        }
    }    
   
    jpeg_destroy_decompress(&dinfo);

    fclose(infile);
}

IMAGE *LoadJPEGtoColorspace(const char *filename, COLORSPACE cs)
{
    struct jpeg_decompress_struct dinfo;
    //struct jpeg_error_mgr jerr;
    struct my_error_mgr jerr;
    FILE *infile;		        /* source file */
    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */
    IMAGE *img;
    int x,y;
    unsigned char *p;
    
    infile = fopen(filename, "rb");
    if (infile == NULL) return(NULL);


    dinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = imglib_error_exit;
        
    if (setjmp(jerr.setjmp_buffer)) 
    {
       /* if we're here, it blowed up... */
       jpeg_destroy_decompress(&dinfo);
       fclose(infile);
       return NULL;
    }
                                              
  //  dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);
    jpeg_stdio_src(&dinfo, infile);

    (void) jpeg_read_header(&dinfo, TRUE);

    switch (cs) 
    {
        case GRAYSCALE: 
            dinfo.out_color_space=JCS_GRAYSCALE; 
            break;
        case RGB:
            dinfo.out_color_space=JCS_RGB; 
            break;
        case YCBCR:
            if (dinfo.jpeg_color_space==JCS_GRAYSCALE)
                dinfo.out_color_space=JCS_GRAYSCALE;
            else
                dinfo.out_color_space=JCS_YCbCr; 
            break;
        case CMYK:
            dinfo.out_color_space=JCS_CMYK;
            break;
        default:
            fclose(infile);
            return NULL;
            break;
    }

    img = MallocImage(dinfo.image_width,dinfo.image_height,cs, false);
    if (img==NULL) 
    {
        fclose(infile); 
        return NULL;
    }
    
    img->dpi=0;
    if (dinfo.saw_JFIF_marker && (dinfo.density_unit != 0))
    {
        unsigned xres=dinfo.X_density;
        unsigned yres=dinfo.Y_density;
       
        if (xres == yres)
        {
            if (dinfo.density_unit == 1)  //directly in dpi ?
                img->dpi = ((int)xres);
            else
                img->dpi = ((int)(xres*0.3937008));  //res. unit is in cm, so convert it
        }
    }

    (void) jpeg_start_decompress(&dinfo);

    row_stride = dinfo.output_width * dinfo.output_components;
    buffer = (*dinfo.mem->alloc_sarray)((j_common_ptr) &dinfo, JPOOL_IMAGE, row_stride, 1);

    while (dinfo.output_scanline < dinfo.output_height) 
    {
        (void) jpeg_read_scanlines(&dinfo, buffer, 1);
        y = dinfo.output_scanline-1;
        if (img->colorspace==GRAYSCALE)
            for (x=0,p=*buffer; x<img->xdim; x++,p++)
                img->gray[y][x]=*p;   
        else if (img->colorspace==YCBCR) 
        {
            if (dinfo.jpeg_color_space==JCS_GRAYSCALE)
                for (x=0,p=*buffer; x<img->xdim; x++) 
                {
                    img->color[0][y][x] = (int)*(p++); 
                    img->color[1][y][x] = img->color[2][y][x] = 128;
                }
            else
                for (x=0,p=*buffer; x<img->xdim; x++) 
                {
                    img->color[0][y][x] = (int)*(p++); 
                    img->color[1][y][x] = (int)*(p++);
                    img->color[2][y][x] = (int)*(p++);
                }
        }  
        else if (img->colorspace == CMYK)
            for (x=0,p=*buffer; x<img->xdim; x++) 
            {
                img->color[0][y][x] = 255 - (int)*(p++);
                img->color[1][y][x] = 255 - (int)*(p++); 
                img->color[2][y][x] = 255 - (int)*(p++);
                img->color[3][y][x] = 255 - (int)*(p++);
            }
        else
            for (x=0,p=*buffer; x<img->xdim; x++) 
            {
                img->color[0][y][x] = (int)*(p++);
                img->color[1][y][x] = (int)*(p++); 
                img->color[2][y][x] = (int)*(p++);
            }
    }
    (void) jpeg_finish_decompress(&dinfo);

    jpeg_destroy_decompress(&dinfo);

    fclose(infile);
    return img;
}

SIMAGE *LoadJPEGtoJPEG(const char *filename)
{
    struct jpeg_decompress_struct dinfo;
    struct my_error_mgr jerr;
   
    COLORSPACE cs;
    
    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) return(NULL);
    
    dinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = imglib_error_exit;
        
    if (setjmp(jerr.setjmp_buffer)) 
    {
       /* if we're here, it blowed up... */
       jpeg_destroy_decompress(&dinfo);
       fclose(infile);
       return NULL;
    }
                                              
  //  dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);
    jpeg_stdio_src(&dinfo, infile);

    (void) jpeg_read_header(&dinfo, TRUE);

  /*  dinfo.out_color_space=dinfo.jpeg_color_space;*/
    switch (dinfo.jpeg_color_space) 
    {
        case JCS_GRAYSCALE: 
            cs = GRAYSCALE; 
            break;
        case JCS_RGB:
            cs = RGB; 
            break;
        case JCS_YCbCr:
            cs = YCBCR; 
            break;
        case JCS_CMYK:
            cs = CMYK;
            break;
        default:
            fclose(infile);
            return NULL;
            break;
     }
     
    SIMAGE *img = MallocShortImage(dinfo.image_width, dinfo.image_height, cs, false);
    if (img==NULL) 
    {
        fclose(infile); 
        return NULL;
    }
    ClearShortImage(img);
    img->transform = JPEG;
    
    /* Read source file as DCT coefficients */  
    jvirt_barray_ptr *coef_arrays = jpeg_read_coefficients(&dinfo);
      
    for (int ci=0; ci<dinfo.num_components; ci++) 
    {
        jpeg_component_info *compptr = dinfo.comp_info + ci;
        JQUANT_TBL *quant_ptr = dinfo.quant_tbl_ptrs[compptr->quant_tbl_no];
        printf("ci %d quant_tbl_no %d\n",ci,compptr->quant_tbl_no);
            
        for (JDIMENSION blk_y = 0; blk_y < compptr->height_in_blocks; blk_y += compptr->v_samp_factor) 
        {
            JBLOCKARRAY buffer = (dinfo.mem->access_virt_barray) ((j_common_ptr) &dinfo, coef_arrays[ci], blk_y, (JDIMENSION) compptr->v_samp_factor, TRUE);
            for (int offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++) 
            {
                int imgy = (blk_y+offset_y)*DCTSIZE*(dinfo.max_v_samp_factor/compptr->v_samp_factor);
                for (JDIMENSION blk_x = 0; blk_x < compptr->width_in_blocks; blk_x++) 
                {
                    int imgx = blk_x*DCTSIZE*(dinfo.max_h_samp_factor/compptr->h_samp_factor);
                    for (int i=0; i<DCTSIZE; i++)
                        for (int j=0; j<DCTSIZE; j++)
                            img->color[ci][imgy+j][imgx+i] = buffer[offset_y][blk_x][i+DCTSIZE*j]*quant_ptr->quantval[i+DCTSIZE*j];
                    if (ci==0)    
                        img->color[ci][imgy][imgx]+=1024;   
                }
            }
        }
    }

    (void) jpeg_finish_decompress(&dinfo);

    jpeg_destroy_decompress(&dinfo);
    fclose(infile);
    return img;
}


#ifndef NO_LCMS
/*
 * This routine writes the given ICC profile data into a JPEG file.
 * It *must* be called AFTER calling jpeg_start_compress() and BEFORE
 * the first call to jpeg_write_scanlines().
 * (This ordering ensures that the APP2 marker(s) will appear after the
 * SOI and JFIF or Adobe markers, but before all else.)
 */
   
void write_icc_profile (j_compress_ptr cinfo, const JOCTET *icc_data_ptr, unsigned int icc_data_len) 
{
    unsigned int num_markers;     /* total number of markers we'll write */
    int cur_marker = 1;           /* per spec, counting starts at 1 */
    unsigned int length;          /* number of bytes to write in this marker */

    /* Calculate the number of markers we'll need, rounding up of course */
    num_markers = icc_data_len / MAX_DATA_BYTES_IN_MARKER;
    if (num_markers * MAX_DATA_BYTES_IN_MARKER != icc_data_len)
        num_markers++;

    while (icc_data_len > 0) 
    {
        /* length of profile to put in this marker */
        length = icc_data_len;
        if (length > MAX_DATA_BYTES_IN_MARKER)
          length = MAX_DATA_BYTES_IN_MARKER;  
        icc_data_len -= length;

        /* Write the JPEG marker header (APP2 code and marker length) */
        jpeg_write_m_header(cinfo, ICC_MARKER,
                            (unsigned int) (length + ICC_OVERHEAD_LEN));

        /* Write the marker identifying string "ICC_PROFILE" (null-terminated).
         * We code it in this less-than-transparent way so that the code works 
         * even if the local character set is not ASCII.
         */
        jpeg_write_m_byte(cinfo, 0x49);
        jpeg_write_m_byte(cinfo, 0x43);
        jpeg_write_m_byte(cinfo, 0x43);
        jpeg_write_m_byte(cinfo, 0x5F);
        jpeg_write_m_byte(cinfo, 0x50);
        jpeg_write_m_byte(cinfo, 0x52);
        jpeg_write_m_byte(cinfo, 0x4F);
        jpeg_write_m_byte(cinfo, 0x46);
        jpeg_write_m_byte(cinfo, 0x49);
        jpeg_write_m_byte(cinfo, 0x4C);
        jpeg_write_m_byte(cinfo, 0x45);
        jpeg_write_m_byte(cinfo, 0x0); 

        /* Add the sequencing info */
        jpeg_write_m_byte(cinfo, cur_marker);
        jpeg_write_m_byte(cinfo, (int) num_markers);

        /* Add the profile data */
        while (length--) 
        {
            jpeg_write_m_byte(cinfo, *icc_data_ptr);
            icc_data_ptr++;
        }
        cur_marker++;
    }
}  
#endif

/*! Common part of SaveJPEG and SaveJPEGtoMemory
 */
void SaveJPEGint(IMAGE *img, struct jpeg_compress_struct *cinfo, const char *comment)
{
    jpeg_start_compress(cinfo, TRUE);

#ifndef NO_LCMS
    // write icc profile
    if (img->icc!=NULL)
    {
        cmsUInt32Number BytesNeeded;
        if (cmsSaveProfileToMem(img->icc, NULL, &BytesNeeded)==TRUE)
        {
            void *buf = malloc(BytesNeeded);
            cmsSaveProfileToMem(img->icc, buf, &BytesNeeded);
            write_icc_profile(cinfo, (JOCTET *)buf, BytesNeeded);
            free(buf);  
        }
                                                                                        
    }
#endif

    if (comment != NULL)
        jpeg_write_marker(cinfo, JPEG_COM, (JOCTET*)comment, strlen(comment));  // write comment
        
    int row_stride = img->xdim * img->cdim;   /* physical row width in image buffer */
    JSAMPARRAY buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr) cinfo, JPOOL_IMAGE, row_stride, 1); /* Output row buffer */

    while (cinfo->next_scanline < cinfo->image_height) 
    {
        int y = cinfo->next_scanline;
        unsigned char *p = *buffer;
        if (img->colorspace==GRAYSCALE)
        {
            for (int x=0; x<img->xdim; x++)
                *(p++) = img->gray[y][x];   
        }
        else if (img->colorspace == CMYK)    
        {
            for (int x=0; x<img->xdim; x++) 
            {
                *(p++) = 255 - img->color[0][y][x]; //in order to be displayed properly, it needs to be saved like this..
                *(p++) = 255 - img->color[1][y][x];
                *(p++) = 255 - img->color[2][y][x];
                *(p++) = 255 - img->color[3][y][x];
            }
        }
        else 
        {
            for (int x=0; x<img->xdim; x++) 
            {
                *(p++) = img->color[0][y][x];
                *(p++) = img->color[1][y][x];
                *(p++) = img->color[2][y][x];
            }
        }
        jpeg_write_scanlines(cinfo, buffer, 1);
    }
    (void) jpeg_finish_compress(cinfo);
}


/*!Saves the given (valid) image into the specified file, using the given quality setting.
 * \param[in] img the image to save
 * \param[in] filename name of the output file
 * \param[in] quality compression quality 0..100 
 * \param[in] comment identification string
 * \pre img!=NULL points to a valid IMAGE object, filename!=NULL
 * \return !=0 if save operation failed, 0 if successful
 */
int SaveJPEG(IMAGE *img, const char *filename, int quality, const char *comment)
{
    assert (img!=NULL);
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfile;                /* target file */

    outfile = fopen(filename, "wb");
    if (outfile == NULL) 
        return -1;
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = img->xdim;      /* image width and height, in pixels */
    cinfo.image_height = img->ydim;
    cinfo.input_components = img->cdim;           /* # of color components per pixel */
    switch (img->colorspace) 
    {
        case GRAYSCALE: 
            cinfo.in_color_space = JCS_GRAYSCALE; 
            break;
        case RGB:
            cinfo.in_color_space = JCS_RGB; 
            break;
        case YCBCR:
            cinfo.in_color_space = JCS_YCbCr; 
            break;
        case CMYK:
            cinfo.in_color_space = JCS_CMYK;
            break;
        default:
            printf("cannot handle colorspace\n");
            jpeg_destroy_compress(&cinfo);
            fclose(outfile);
            return -1;
    }

    jpeg_set_defaults(&cinfo);
    cinfo.density_unit = 1;
    if (img->dpi > 0)
    {
        cinfo.X_density = img->dpi;
        cinfo.Y_density = img->dpi;
    }
    else
    {
        // set default values
        cinfo.X_density = 72;
        cinfo.Y_density = 72;
    }

    jpeg_set_quality(&cinfo, quality, TRUE); // limit to baseline-JPEG values
    
    cinfo.optimize_coding = TRUE;  // causes the compressor to compute optimal Huffman coding tables
    
    //cinfo.dct_method = JDCT_FLOAT;  // floating-point method for DCT

    SaveJPEGint(img, &cinfo, comment);

    fclose(outfile);

    jpeg_destroy_compress(&cinfo);

    return 0;
}

/*!Saves the given (valid) image into a memory buffer using the given quality setting.
 * The memory buffer is allocated by the function and has to be freed by the caller.
 * \param[in] img the image to save
 * \param[out] outbuffer pointer to memory buffer
 * \param[out] outsize size of buffer
 * \param[in] quality compression quality 0..100
 * \param[in] comment identification string
 * \pre img!=NULL points to a valid IMAGE object, filename!=NULL
 * \return !=0 if save operation failed, 0 if successful
 */
//#if JPEG_LIB_VERSION >= 70
int SaveJPEGtoMemory(IMAGE *img, unsigned char **outbuffer, unsigned long *outsize, int quality, const char *comment)
{
    assert (img!=NULL);

    *outbuffer = NULL;
    *outsize = 0;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, outbuffer, outsize);
    cinfo.image_width = img->xdim;      /* image width and height, in pixels */
    cinfo.image_height = img->ydim;
    cinfo.input_components = img->cdim;           /* # of color components per pixel */
    switch (img->colorspace) 
    {
        case GRAYSCALE: 
            cinfo.in_color_space = JCS_GRAYSCALE; 
            break;
        case RGB:
            cinfo.in_color_space = JCS_RGB; 
            break;
        case YCBCR:
            cinfo.in_color_space = JCS_YCbCr; 
            break;
        case CMYK:
            cinfo.in_color_space = JCS_CMYK;
            break;
        default:
            jpeg_destroy_compress(&cinfo);
            printf("cannot handle colorspace\n");
            return -1;
    }

    jpeg_set_defaults(&cinfo);
    cinfo.density_unit = 1;
    if (img->dpi > 0)
    {
        cinfo.X_density = img->dpi;
        cinfo.Y_density = img->dpi;
    }
    else
    {
        // set default values
        cinfo.X_density = 72;
        cinfo.Y_density = 72;
    }

    jpeg_set_quality(&cinfo, quality, TRUE); // limit to baseline-JPEG values
    
    cinfo.optimize_coding = TRUE;  // causes the compressor to compute optimal Huffman coding tables
    
    //cinfo.dct_method = JDCT_FLOAT;  // floating-point method for DCT

    SaveJPEGint(img, &cinfo, comment);

    jpeg_destroy_compress(&cinfo);

    return 0;
}
//#endif

/*!Saves the given (valid) image into the specified file. The parameters for
 * the compression are read from infilename. This gives identical filesizes
 * for rewritung of JPEG-images.
 * \param[in] img the image to save
 * \param[in] outfilename name of the output file
 * \param[in] infilename name of the input file to get compression parameters
 * \param[in] comment identification string
 * \pre img!=NULL points to a valid IMAGE object, filename!=NULL
 * \return !=0 if save operation failed, 0 if successful
 */
int SaveJPEGwithQualityFromFile(IMAGE *img, const char *outfilename, char *infilename, const char *comment)
{
    assert (img!=NULL);
    
    // read decompression info (quantization tables) from input file
    struct jpeg_decompress_struct dinfo;
    struct my_error_mgr jerr;
    FILE *infile;

    infile = fopen(infilename, "rb");
    if (infile == NULL) 
        return -1;
 
    dinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit     = imglib_error_exit;
      
    if (setjmp(jerr.setjmp_buffer)) 
    {
        // if we're here, it blowed up... 
        jpeg_destroy_decompress(&dinfo);
        fclose(infile);
        return -1;
    }

    jpeg_create_decompress(&dinfo);
    jpeg_stdio_src(&dinfo, infile);

    (void) jpeg_read_header(&dinfo, TRUE);
    
    
    // create output file
    struct jpeg_compress_struct cinfo;
    FILE *outfile;                /* target file */

    outfile = fopen(outfilename, "wb");
    if (outfile == NULL) 
        return -1;
    
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit     = imglib_error_exit;
    
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = img->xdim;      /* image width and height, in pixels */
    cinfo.image_height = img->ydim;
    cinfo.input_components = img->cdim;           /* # of color components per pixel */
    switch (img->colorspace) 
    {
        case GRAYSCALE: 
            cinfo.in_color_space = JCS_GRAYSCALE; 
            break;
        case RGB:
            cinfo.in_color_space = JCS_RGB; 
            break;
        case YCBCR:
            cinfo.in_color_space = JCS_YCbCr; 
            break;
        case CMYK:
            cinfo.in_color_space = JCS_CMYK;
            break;
        default:
            printf("cannot handle colorspace\n");
            fclose(outfile);
            return -1;
    }

    jpeg_set_defaults(&cinfo);
    cinfo.density_unit = 1;
    if (img->dpi > 0)
    {
        cinfo.X_density = img->dpi;
        cinfo.Y_density = img->dpi;
    }
    else
    {
        // set default values
        cinfo.X_density = 72;
        cinfo.Y_density = 72;
    }
    
    // copy quantization tables, sampling factos and colorspace
    jpeg_set_colorspace(&cinfo, dinfo.jpeg_color_space);
    if (cinfo.num_components > dinfo.num_components)
    {
        fclose(outfile);
    
        jpeg_destroy_compress(&cinfo);
        jpeg_destroy_decompress(&dinfo);
                   
        fclose(infile);
        return -1;
    }

    for (int ci=0; ci<dinfo.num_components; ci++)
    {
        int tableno = cinfo.comp_info[ci].quant_tbl_no;
        cinfo.comp_info[ci].quant_tbl_no = tableno;
        
        JQUANT_TBL *inquant = dinfo.quant_tbl_ptrs[tableno];
        //printf("ci %d quant_tbl_no %d\n", ci, tableno);
                                    
        cinfo.comp_info[ci].h_samp_factor = dinfo.comp_info[ci].h_samp_factor;
        cinfo.comp_info[ci].v_samp_factor = dinfo.comp_info[ci].v_samp_factor;

        //jpeg_add_quant_table (&cinfo, compptr->quant_tbl_no, quant_ptr->quantval, 100, false);
        // copy of above function
        if (cinfo.quant_tbl_ptrs[tableno] == NULL)
            cinfo.quant_tbl_ptrs[tableno] = jpeg_alloc_quant_table((j_common_ptr) &cinfo);
            
        JQUANT_TBL *outquant = cinfo.quant_tbl_ptrs[tableno];

        for (int i=0; i<DCTSIZE2; i++) 
            outquant->quantval[i] = inquant->quantval[i];

        /* Initialize sent_table FALSE so table will be written to JPEG file. */
        outquant->sent_table = FALSE;
                
    }
    
    
//    cinfo.optimize_coding = TRUE;  // causes the compressor to compute optimal Huffman coding tables
    
    //cinfo.dct_method = JDCT_FLOAT;  // floating-point method for DCT

    SaveJPEGint(img, &cinfo, comment);

    fclose(outfile);

    jpeg_destroy_compress(&cinfo);
    jpeg_destroy_decompress(&dinfo);

    fclose(infile);


    return 0;
}

const float jpegcomprates[101] = { 0.0047211667f, 0.0047211667f, 0.0047271667f, 0.0054575f, 0.0068955f, 0.0085175f,
                                0.010165f, 0.0116313333f, 0.0132278333f, 0.0148225f, 0.0164501667f, 0.0179925f,
                                0.019329f, 0.0207028333f, 0.0218555f, 0.0231641667f, 0.0247963333f, 0.0261513333f,
                                0.0272871667f, 0.0284471667f, 0.0294393333f, 0.030534f, 0.0315693333f,
                                0.0325713333f, 0.0336336667f, 0.0345573333f, 0.035629f, 0.0365513333f,
                                0.0375898333f, 0.0383928333f, 0.0392803333f, 0.0400843333f, 0.0408933333f,
                                0.042111f, 0.0426726667f, 0.0437941667f, 0.0447845f, 0.0453498333f, 0.046475f,
                                0.0471856667f, 0.0475616667f, 0.0487696667f, 0.0493238333f, 0.049891f, 0.05095f,
                                0.0515688333f, 0.0525585f, 0.0529815f, 0.0538013333f, 0.0548961667f, 0.0551045f,
                                0.0553268333f, 0.0565036667f, 0.0574508333f, 0.0579566667f, 0.0586386667f,
                                0.0595403333f, 0.0605483333f, 0.0615966667f, 0.0621898333f, 0.0629185f,
                                0.0640963333f, 0.0648816667f, 0.066134f, 0.0667898333f, 0.0681058333f, 0.0690495f,
                                0.0700101667f, 0.0716611667f, 0.0727086667f, 0.0742058333f, 0.0756576667f,
                                0.076673f, 0.0784241667f, 0.0806653333f, 0.0811448333f, 0.0829508333f,
                                0.0858253333f, 0.0881613333f, 0.0898308333f, 0.0922971667f, 0.0952051667f,
                                0.0975883333f, 0.1014121667f, 0.1042646667f, 0.107139f, 0.1117328333f, 0.1148305f,
                                0.120633f, 0.1248298333f, 0.1321038333f, 0.1380373333f, 0.144346f, 0.1551451667f,
                                0.1681733333f, 0.1834315f, 0.20145f, 0.2227965f, 0.2463993333f, 0.2894348333f,
                                0.3162011667f };


/*!Saves the given (valid) image into the specified file, using the given
 *comperssion rate.  The images is saved to a memory buffer with varying
 *quality settings, until the desired compression rate is reached.  This
 *function is significantly slower than SaveJPEGwithQualityFromFile.
 * \param[in] img the image to save 
 * \param[in] outfilename name of the output file 
 * \param[in] rate desired compression rate 
 * \param[in] comment identification string 
 * \pre img!=NULL points to a valid IMAGE object, filename!=NULL 
 * \return !=0 if save operation failed, 0 if successful
 */
int SaveJPEGwithCompressionrate(IMAGE *img, const char *outfilename, float rate, const char *comment)
{
    unsigned long outsize;
    unsigned char *outbuffer;
    int imgsize = img->xdim * img->ydim * img->cdim;

    int qbottom = 100;
    for (qbottom=100; qbottom>=0; qbottom--)
        if (jpegcomprates[qbottom] < 0.5*rate)
            break;
//    qbottom = 40;
    SaveJPEGtoMemory(img, &outbuffer, &outsize, qbottom);
    free(outbuffer);
    outbuffer = NULL;

    float rbottom = (float)outsize/imgsize;
             
    int qtop = 0;                            
    for (qtop=0; qtop<=100; qtop++)
        if (jpegcomprates[qtop] > 2*rate)
            break;
//    qtop = 80;
//    unsigned long outsize2;
    SaveJPEGtoMemory(img, &outbuffer, &outsize, qtop);
    //free(outbuffer);  // don't free, keep it if iteration breaks early
    
    float rtop = (float)outsize/imgsize;
    
    int numiterations = 0;
    while (numiterations <7)
    {   
         int qintp = (rate - (qtop*rbottom-qbottom*rtop)/(qtop-qbottom)) * ((qtop-qbottom)/(rtop-rbottom)) + 0.5;
//printf("q1 %d r1 %f  q2 %d r2 %f\n", q1, r1, q2, r2);
//printf("q3 %d\n", q3);

        if (qintp>100)
            qintp = 100;
        
        if (qintp<0)
            qintp = 0;
        
        if (qintp == qbottom || qintp==qtop)
            break;
            
        if (outbuffer != NULL)
        {
            free(outbuffer);
            outbuffer = NULL;
        }
        SaveJPEGtoMemory(img, &outbuffer, &outsize, qintp);
        
        float rmeasure = (float)outsize/imgsize;
        
        if (rmeasure>0.9*rate && rmeasure<1.1*rate)
            break;
        
        if (rmeasure>rate)
        {
            rtop = rmeasure;
            qtop = qintp;
        }
        else
        {
            rbottom = rmeasure;
            qbottom = qintp;
        }
                        
        numiterations++;
    }
    
    int retvalue = 0;
    FILE *fp = fopen(outfilename, "w");
    if (fp!=NULL)
    {
        if (fwrite(outbuffer, 1, outsize, fp)!=outsize)
            retvalue = -1;
        fclose(fp);
    }
                             
    return retvalue;
}





}

#endif //ndef NO_JPEG
