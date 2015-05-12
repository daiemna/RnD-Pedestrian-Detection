#include "ibinary.h"
#include "image.h"

#include <cassert>
#include <algorithm>


namespace image
{

/*!Checks if the given grayscale(!) image is binary i.e. it contains only the 
 * 0 (black) or 255 (white) gray values.
 * \param img - a properly initialized grayscale IMAGE structure
 * \pre img is an existing GRAYSCALE image
 * \return true, if the given image is binary; false, otherwise
 */
bool IsBinaryImage(const IMAGE* img)
{
    assert (img!=NULL);
    
//    if (img->range == BINARY)
//      return true;

    if (img->colorspace!=GRAYSCALE)
        return false;
    
    const int xdim = img->xdim;
    const int ydim = img->ydim;
    for (int y=0; y<ydim; y++)
    {
        PIX *pic = img->gray[y];
        for (int x=0; x<xdim; x++)
            if (pic[x] && (pic[x]!=255))
            {
//                img->range = UNKNOWN;
                return false;
            }
    }
//    img->range = BINARY;
    return true;
}

/*!applies a median filter to the input binary image img using a centered 5x5 
 * mask. The mask pixels are the whole 5x5 rectangle, excluding the topleft, 
 * topright, bottomleft and bottomright pixels.
 * \sa IsBinaryImage()
 */
int BinaryMedian(IMAGE *img)
{
    assert(img!=NULL);
    assert(IsBinaryImage(img));
    
    IMAGE *img2;
    int i,j;
    int count;

    if (img==NULL) return(-1);
    if (!IsBinaryImage(img)) return(-1);
 
    img2 = MallocImage(img->xdim,img->ydim,GRAYSCALE,false);
    for (int j=2; j<img->ydim-2; j++)
        for (int i=2; i<img->xdim-2; i++)
        {
            count=0;
#if 0
            if (img->gray[j-1][i-1]>0) count++;
            if (img->gray[j-1][i  ]>0) count++;
            if (img->gray[j-1][i+1]>0) count++;
            if (img->gray[j  ][i-1]>0) count++;
            if (img->gray[j  ][i+1]>0) count++;
            if (img->gray[j+1][i-1]>0) count++;
            if (img->gray[j+1][i  ]>0) count++;
            if (img->gray[j+1][i+1]>0) count++;
            if (count>4) img2->gray[j][i]= 255;
            if (count<4) img2->gray[j][i]= 0;
#else

            if (img->gray[j-2][i-1]>0) count++;
            if (img->gray[j-2][i  ]>0) count++;
            if (img->gray[j-2][i+1]>0) count++;

            if (img->gray[j-1][i-2]>0) count++;
            if (img->gray[j-1][i-1]>0) count++;
            if (img->gray[j-1][i  ]>0) count++;
            if (img->gray[j-1][i+1]>0) count++;
            if (img->gray[j-1][i+2]>0) count++;

            if (img->gray[j  ][i-2]>0) count++;
            if (img->gray[j  ][i-1]>0) count++;
            if (img->gray[j  ][i+1]>0) count++;
            if (img->gray[j  ][i+1]>0) count++;

            if (img->gray[j+1][i-2]>0) count++;
            if (img->gray[j+1][i-1]>0) count++;
            if (img->gray[j+1][i  ]>0) count++;
            if (img->gray[j+1][i+1]>0) count++;
            if (img->gray[j+1][i+2]>0) count++;

            if (img->gray[j+2][i-1]>0) count++;
            if (img->gray[j+2][i  ]>0) count++;
            if (img->gray[j+2][i+1]>0) count++;

            if (count>10) img2->gray[j][i]= 255;
            if (count<8) img2->gray[j][i]= 0;
#endif
        }
 
    for(j=0; j<img->ydim; j++)
        for (i=0; i<img->xdim; i++)
            img->gray[j][i] = img2->gray[j][i];
 
    FreeImage(img2);
    return 0;
}

/*!performs a linear horizontal smearing with element of length len on the input 
 * binary image im (i.e. transforms white pixels into black ones if interval
 * is strictly shorter than len). It is assumed that the image is surrounded by
 * (black) object pixel in normal operation. 
 * \param[in,out] im image
 * \param[in] len minimal unchanged runlength
 * \param[in] whiteborder disables the black border assumtion
 * \remarks Binary images are considered to have the value 0 for object
 * pixels and 255 for the background. 
 * \remarks If the length ist larger than the image width in black border mode, the result is a black image.
 * \sa IsBinaryImage()
 */
void DoHorizontalSmearing(IMAGE* im, unsigned int len, bool whiteborder)
{
    assert(im!=NULL);
    assert(IsBinaryImage(im));
    
    if ((len<2u) || (im->xdim<=0) || (im->ydim<=0))
        return;
    
    PIX **pic = im->color[0];
    const int xdim = im->xdim;
    const int ydim = im->ydim;
    for (int y=0; y<ydim; y++)
    {
        int x=0;
        int startp = 0; //index of 1st white pixel of the sequence
        if (whiteborder)
            while ((x<xdim)&&(pic[y][x]))  // find first black pixel
                x++;

        do
        {
            // find white pixel
            while ((x<xdim)&&(!pic[y][x]))
                x++;
                
            if (x<xdim)
            {
                startp = x;

                // find black pixel
                do 
                {
                    x++;
                    if (x>=xdim)
                        break;
                }
                while (pic[y][x]);
                if (x>=xdim)
                    break;
                
                // fill white runlength
                if (x-startp < int(len))
                    for (int jk=startp; jk<x; jk++)
                        pic[y][jk] = 0;
            }
        }
        while (x<xdim);
   
        //do smearing for the last vertical run-length
        if (!whiteborder && (xdim-startp < int(len)))
             for (int jk=startp; jk<xdim; jk++)
                 pic[y][jk] = 0;
    }
}

/*!performs a linear vertical smearing with element of length len on the input 
 * binary image im (i.e. transforms white pixels into black ones if interval
 * is strictly shorter than len). It is assumed that the image is surrounded by
 * (black) object pixel in normal operation.
 * \param[in,out] im image
 * \param[in] len minimal unchanged runlength
 * \param[in] whiteborder disables the black border assumtion
 * \remarks Binary images are considered to have the value 0 for object pixels and 255 for the background.
 * \sa IsBinaryImage()
 */
void DoVerticalSmearing(IMAGE* im, unsigned int len, bool whiteborder)
{
    assert (im!=NULL);
    assert(IsBinaryImage(im));
    
    if ((len<2u) || (im->xdim<=0) || (im->ydim<=0))
        return;
    
    const int xdim = im->xdim;
    const int ydim = im->ydim;
    int *startp = new int[xdim]; //startp = last black pixel index on the current vertical scanline

    PIX **pic = im->color[0];

    if (whiteborder)
    {
        for (int x=0; x<xdim; ++x)
            startp[x]= (pic[0][x]? (-int(len)-1): 0);
    }
    else
    {
        for (int x=0; x<xdim; ++x)
            startp[x]= (pic[0][x]? -1: 0);
    }

    for (int y=1; y<ydim; ++y)
    {
        for (int x=0; x<xdim; ++x)
        {
            if (!pic[y][x]) // black pixel
            {
                if (y-startp[x]-1<int(len))
                    for (int jk=startp[x]+1; jk<y; ++jk)
                        pic[jk][x] = 0;
                startp[x] = y;
            }
        }
    }
    
    //also do smoothing for the last vertical run-lengths
    if (!whiteborder)
        for (int x=0; x<xdim; ++x)
            if (ydim-startp[x]-1<int(len))
                for (int jk=startp[x]+1; jk<ydim; ++jk)
                    pic[jk][x]=0;
    delete[] startp;
}

/*!Computes the logical union of the binary images \c src1 and \c src2 (having the same 
 * dimensions!), and puts the result into the already-allocated image \c dest. Note that 
 * \c dest is allowed to be the same as \c src1 or \c src2, and it may also be larger 
 * than both input images, case in which the values of the pixels located outside the 
 * input images will be preserved.   
 * \pre \c src1, \c src2 and \c dest must be valid binary images; 
 * dest->xdim >= max(src1->xdim, src2->xdim); dest->ydim >=max(src1->ydim, src2->ydim); 
 * src1->xdim == src2->xdim; src1->ydim == src2->ydim 
 * \sa IsBinaryImage()
 */
void BinaryImageUnion(const IMAGE *src1, const IMAGE* src2, IMAGE *dest)
{
    assert((src1!=NULL) && (src2!=NULL) && (dest!=NULL));
    assert((src1->xdim==src2->xdim) && (src1->ydim==src2->ydim));
    assert((dest->xdim >= std::max(src1->xdim, src2->xdim)) && 
        (dest->ydim >= std::max(src1->ydim, src2->ydim)));
    
    if (src1==NULL || src2==NULL || dest==NULL)
        return;
    if (src1->xdim!=src2->xdim || src1->ydim!=src2->ydim)
        return;
    if (!((dest->xdim >= std::max(src1->xdim, src2->xdim)) && 
            (dest->ydim >= std::max(src1->ydim, src2->ydim))))
        return;
    
    int xdim = src1->xdim;
    int ydim = src1->ydim;

    for (int i=0; i<ydim; ++i)
    {
        PIX *pic1=src1->gray[i];
        PIX *pic2=src2->gray[i];
        PIX *picd=dest->gray[i];
        
        for (int j=0; j<xdim; ++j)
            if (pic1[j] && pic2[j])
                picd[j]=255;
            else
                picd[j]=0;
    }
}

};
