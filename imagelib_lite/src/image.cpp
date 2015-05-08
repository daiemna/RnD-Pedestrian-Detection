/******************************************************************************
 *
 *       image library 
 *
 *       Speicherverwaltung fuer Bilder   
 *
 *       28.10.97 Stefan Eickeler
 *       12.12.97 STM: Erweiterung von LoadPXM auf P4
 *       16.07.98 Umstellung von short auf int und [y][x][c] auf [c][y][x]
 *        7.12.98 Einfhrung des Alphakanals fr GRAYSCALE, RGB, YUV
 *       20.09.02 Dokumentation mit doxygen
 *
 ******************************************************************************/

#include "image.h"
#include "icolor.h"

//#include <stdio.h>
//#include <stdlib.h>
#include <cstddef>


namespace image
{

/*!
 * \fn MallocImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha)
 * allocate dynamic memory for image
 * \param[in] xdim vertical size of image in pixels
 * \param[in] ydim horizontal size of image in pixels
 * \param[in] cs colorspace of image 
 * \param[in] with_alpha add alpha channel if true
 * \return The value returned is a pointer to the allocated image or NULL if the request fails.
 * \sa image::FreeImage(), image::MallocPImage()
 * \deprecated
 */
IMAGE* MallocImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha)
{
    return new IMAGE(xdim, ydim, cs, with_alpha);
}

/*!Converts an image into an image with alpha map.
 * \param[in,out] img image
 * \return 0 is returned on success, -1 is returned on error
 */
int AddAlphaChannel(IMAGE *img)
{
    if (img==NULL) return -1;
    
    return (img->AddAlphaChannel());
}

/*!Converts an image with alpha map into an image without alpha map.
 * \param[in,out] img image
 * \return 0 is returned on success, -1 is returned on error
 */
int RemoveAlphaChannel(IMAGE *img)
{
 if (img==NULL) return -1;
 
 return (img->RemoveAlphaChannel());
}

/*!
 * \fn FreeImage(IMAGE *img)
 * free dynamic memory used by image
 * \param img pointer to image
 * \return Returns 0 on succes, or -1 if request fails.
 * \sa MallocImage()
 * \deprecated
 */
int FreeImage(IMAGE *img)
{
    delete img;        //works OK also for img==NULL !...
    return(0);
}

/*!set pixels of image to 0
 * \param img image 
 * \return Upon successful completion 0 is returned.  Otherwise,-1 is returned.
 * \deprecated
 */
int ClearImage(IMAGE *img)
{
    if (img==NULL)
        return(-1);
    img->Clear();
    return(0);
}


/*!
 * \deprecated
 */
IIMAGE* MallocIntImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha)
{
    return new IIMAGE(xdim, ydim, cs, with_alpha);
}


/*!
 * \deprecated
 */
int FreeIntImage(IIMAGE *img)
{
/* printf("freeimage %d\n",img);*/
    delete img;         //works OK also for img==NULL !...
    return 0;
}


/*!
 * \deprecated
 */
int ClearIntImage(IIMAGE *img)
{
    if (img==NULL)
        return -1;
    img->Clear();
    return 0;
}


/*!
 * \deprecated
 */
SIMAGE* MallocShortImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha)
{
    return new SIMAGE(xdim, ydim, cs, with_alpha);
}


/*!
 * \deprecated
 */
int FreeShortImage(SIMAGE *img)
{
/* printf("freeimage %d\n",img);*/
    delete img;         //works OK also for img==NULL !...
    return(0);
}

/*!
 * \deprecated
 */
int ClearShortImage(SIMAGE *img)
{
    if (img==NULL)
        return(-1);
    img->Clear();
    return(0);
}


/*!
 * \deprecated
 */
LIMAGE* MallocLongImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha)
{
    return new LIMAGE(xdim, ydim, cs, with_alpha);
}



/*!
 * \deprecated
 */
int FreeLongImage(LIMAGE *img)
{
// printf("freeimage %d\n",img);
    delete img;         //works OK also for img==NULL !...
    return(0);
}


/*!
 * \deprecated
 */
int ClearLongImage(LIMAGE *img)
{
    if (img==NULL)
        return(-1);
    img->Clear();
    return(0);
}


/*!
 * \deprecated
 */
FIMAGE* MallocFloatImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha)
{
    return new FIMAGE(xdim, ydim, cs, with_alpha);
}


/*!
 * \deprecated
 */
int FreeFloatImage(FIMAGE *img)
{
/* printf("freeimage %d\n",img);*/
    delete img;         //works OK also for img==NULL !...
    return(0);
}


/*!
 * \deprecated
 */
int ClearFloatImage(FIMAGE *img)
{
    if (img==NULL)
        return(-1);
    img->Clear();
    return(0);
}

/*!
 * \deprecated
 */
DIMAGE* MallocDoubleImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha)
{
    return new DIMAGE(xdim, ydim, cs, with_alpha);
}


/*!
 * \deprecated
 */
int FreeDoubleImage(DIMAGE *img)
{
// printf("freeimage %d\n",img);
    delete img;         //works OK also for img==NULL !...
    return(0);
}


/*!
 * \deprecated
 */
int ClearDoubleImage(DIMAGE *img)
{
    if (img==NULL)
        return(-1);
    img->Clear();
    return(0);
}

};
