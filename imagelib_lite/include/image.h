#ifndef _IMAGE_BASIC_H_
#define _IMAGE_BASIC_H_

#ifndef NO_LCMS
#define NO_LCMS
#endif // NO_LCMS


#include "definitionImageLib.h"
#include "ierror_handling.h"
#include "ipixel_types.h"
#include "icolor.h"

#include <algorithm>
#include <cassert>
#include <limits>

#ifndef NO_LCMS
#include <lcms2.h>
#include "lcmshelper.h"
#else
#define cmsHPROFILE void*
#endif

namespace image
{

/*!\addtogroup ImageBasic Image, PalettedImage Classes and (deprecated) Allocation, Deletion Functions 
 *\brief Include header file(s): "image.h", "paletted_image.h".
 */
//@{


/*!Enumeration of all supported transform types for a certain Image<> object.\n
 * \b DFT    discrete fourier transform\n
 * \b DWT    discrete wavelet transform (in place)\n
 * \b DCT    discrete cosine transform\n
 * \b DWPT   discrete wavelet packed transform (in place)\n
 * \b JPEG   colorspace YCbCr, 8x8 DCT Y, 16x16 DCT UV\n
 * \b HADAMARD
 * \b SEDT   squared Euclidean distance transform
 * \b DESKEW   deskew transform
 * \b TT_COUNT the number of supported transform types
 */
enum TRANSFORM
{
    ORIGINAL, 
    DFT, 
    DWT, 
    DCT, 
    DWPT, 
    JPEG, 
    HADAMARD,
    SEDT,
    DESKEW,
    TT_COUNT //do NOT assign values to any enumeration members, as then _COUNT will not work properly!
};


//enum PIXELRANGE
//{ UNKNOWN,    //!< unkown range
//  BINARY,     //!< binary image (0, 255)
//  PIXRANGE,   //!< 0 .. 255
//  NORMALIZED  //!< 0.0 .. 1.0
//};  


/*!Base template Image class used for all image types (specializations) in this library.
 * \note TODO: modify class so now it uses the copy-and-swap idiom! (adds strong exception guarantee
 * and a bit better efficiency (especially in C++11 with move semantics),
 * while code duplication is non-existent just as it is now)
 */
template < class PIXTYPE >
class Image
{
public:
    typedef PIXTYPE value_type;

    //structural data
    COLORSPACE colorspace;   //!< colorspace of image
    int xdim;            //!< horizontal size of image
    int ydim;            //!< vertical size of image
    int cdim;            //!< number of color planes
    PIXTYPE *pixmap;         //!< pointer to pixels of image
    PIXTYPE **gray;          //!< pointer to array of color planes
    PIXTYPE ***color;        //!< ponter to array of lines
    PIXTYPE *alphamap;       //!< pointer to pixels of alpha channal
    PIXTYPE **alpha;         //!< pointer to array of lines of alpha channel
    
    //descriptive data
    TRANSFORM  transform;    //!< applied transform
    int dpi;             //!< scanning resolution in dots per inch (<=0 if not appplicable)
//        PIXELRANGE range;
    cmsHPROFILE icc;


protected:
    void ReleaseStructuralData();
    void ReleaseDescriptiveData();
    void Release();
    
    void InitDefaultsStructuralData();
    void InitDefaultsDescriptiveData();
    void InitDefaults();

    template <class PIXTYPE2  > 
    void CopyDescriptiveDataFrom_Simple(const Image< PIXTYPE2 >& img, bool copyicc = true);

    template <class PIXTYPE2  > 
    void CopyStructuralDataFrom_Simple(const Image< PIXTYPE2 >& img);
    
    template <class PIXTYPE2  > 
    void CopyDataFrom_Simple(const Image< PIXTYPE2 >& img);

public:
    Image()
    {
        InitDefaults();
    }

    Image(int x_dim, int y_dim, COLORSPACE cs, bool with_alpha=false);
    //TODO: only add default alpha AFTER all references to this construcots& MallocImage were found!
    
    Image(const Image< PIXTYPE >& img);
    
    Image(const Image< PIXTYPE >* img);
    
    template <class PIXTYPE2  >
    explicit Image(const Image< PIXTYPE2 >&img);
    
    virtual ~Image();
    
    int AddAlphaChannel();
    int RemoveAlphaChannel();
    bool HasAlpha() const;

#ifndef NO_LCMS
    int AddICC();
    int AddICC(cmsHPROFILE aicc);
    int RemoveICC();
#endif
    
    void Fill(PIXTYPE val);
    void Clear();
    
    PIXTYPE GetMaxGrayValue() const;
    PIXTYPE GetMinGrayValue() const;
    PIXTYPE GetMaxAlphaValue() const;
    PIXTYPE GetMinAlphaValue() const;
    
    const Image< PIXTYPE >& operator=(const Image< PIXTYPE >& img);

    template <class PIXTYPE2  > 
    const Image< PIXTYPE >& operator=(const Image< PIXTYPE2 >& img);
    
    void CopyStructuralDataFrom(const Image< PIXTYPE >& img);

    template <class PIXTYPE2  > 
    void CopyStructuralDataFrom(const Image< PIXTYPE2 >& img);

    void CopyDescriptiveDataFrom(const Image< PIXTYPE >& img, bool copyicc = true);

    template <class PIXTYPE2  > 
    void CopyDescriptiveDataFrom(const Image< PIXTYPE2 >& img, bool copyicc = true);

    void swap(Image< PIXTYPE >& img);
};

//explicit exports of several template instantiations
EXPIMP_TEMPLATE template class EXPORT_IMAGELIB Image<PIX>;
EXPIMP_TEMPLATE template class EXPORT_IMAGELIB Image<SPIX>;
EXPIMP_TEMPLATE template class EXPORT_IMAGELIB Image<IPIX>;
EXPIMP_TEMPLATE template class EXPORT_IMAGELIB Image<LPIX>;
EXPIMP_TEMPLATE template class EXPORT_IMAGELIB Image<FPIX>;
EXPIMP_TEMPLATE template class EXPORT_IMAGELIB Image<DPIX>;

typedef Image<PIX> IMAGE;		//!<IMAGE with elements of type unsigned integer, 8 bits/pixel per channel
typedef Image<SPIX> SIMAGE;		//!<IMAGE with elements of type signed integer, 16 bits/pixel per channel
typedef Image<IPIX> IIMAGE;		//!<IMAGE with elements of type signed integer, 32 bits/pixel per channel
typedef Image<LPIX> LIMAGE;		//!<IMAGE with elements of type signed integer, 64 bits/pixel per channel
typedef Image<FPIX> FIMAGE;		//!<IMAGE with elements of type floating-point, 32 bits/pixel per channel
typedef Image<DPIX> DIMAGE;     //!<IMAGE with elements of type floating-point, 64 bits/pixel per channel


//definitions of deprecated dynamic allocation/deletion functions for Images
EXPORT_IMAGELIB IMAGE*  MallocImage( int xdim, int ydim, COLORSPACE cs, bool with_alpha=false );
EXPORT_IMAGELIB int     FreeImage(IMAGE *img);
EXPORT_IMAGELIB int     ClearImage(IMAGE *img);
EXPORT_IMAGELIB IIMAGE* MallocIntImage( int xdim, int ydim, COLORSPACE cs, bool with_alpha=false );
EXPORT_IMAGELIB int     FreeIntImage(IIMAGE *img);
EXPORT_IMAGELIB int     ClearIntImage(IIMAGE *img);
EXPORT_IMAGELIB SIMAGE* MallocShortImage( int xdim, int ydim, COLORSPACE cs, bool with_alpha=false );
EXPORT_IMAGELIB int     FreeShortImage(SIMAGE *img);
EXPORT_IMAGELIB int     ClearShortImage(SIMAGE *img);
EXPORT_IMAGELIB LIMAGE* MallocLongImage( int xdim, int ydim, COLORSPACE cs, bool with_alpha=false );
EXPORT_IMAGELIB int     FreeLongImage(LIMAGE *img);
EXPORT_IMAGELIB int     ClearLongImage(LIMAGE *img);
EXPORT_IMAGELIB FIMAGE* MallocFloatImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha=false );
EXPORT_IMAGELIB int     FreeFloatImage(FIMAGE *img);
EXPORT_IMAGELIB int     ClearFloatImage(FIMAGE *img);
EXPORT_IMAGELIB DIMAGE* MallocDoubleImage(int xdim, int ydim, COLORSPACE cs, bool with_alpha=false );
EXPORT_IMAGELIB int     FreeDoubleImage(DIMAGE *img);
EXPORT_IMAGELIB int     ClearDoubleImage(DIMAGE *img);
int AddAlphaChannel(IMAGE *img);
int RemoveAlphaChannel(IMAGE *img);

//@}


//---------------------------------------------------------------------------------------
//                    implementations of INLINE/ TEMPLATE functions
//---------------------------------------------------------------------------------------


/*!Fills the Image's descriptive data members with their default values.
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::InitDefaultsDescriptiveData()
{
    dpi= 0;
    transform= ORIGINAL;
    icc = NULL;
}

/*!Fills the Image's structural data members with their default values.
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::InitDefaultsStructuralData()
{
    colorspace= GRAYSCALE;
    pixmap= alphamap= NULL;
    gray= alpha= NULL;
    color= NULL;
    xdim= ydim= cdim= 0;
}

/*!Fills the Image object's members with their default values.
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::InitDefaults()
{
    InitDefaultsStructuralData();
    InitDefaultsDescriptiveData();
}

/*!Copies the complete descriptive data of the given image into the current one. Assumes the 
 * current image does not already contain any dynamically allocated descriptive data (i.e. it does 
 * \b not deallocate dynamic memory!).
 */
template < class PIXTYPE >
template < class PIXTYPE2 >
inline void Image< PIXTYPE >::CopyDescriptiveDataFrom_Simple(const Image< PIXTYPE2 >& img, bool copyicc)
{
    transform = img.transform;
    dpi = img.dpi;
//    range = img.range;
    icc = NULL;
#ifndef NO_LCMS
    if (img.icc!=NULL && copyicc)
    {
        // there is not copy function for icc profiles -> save to memory and load again
        // autopointer may be another solution
        cmsUInt32Number BytesNeeded;
        if (cmsSaveProfileToMem(img.icc, NULL, &BytesNeeded))
        {
            void *buf = malloc(BytesNeeded);
            cmsSaveProfileToMem(img.icc, buf, &BytesNeeded);
            icc = cmsOpenProfileFromMem(buf, BytesNeeded);
            free(buf);
        }
        
    }
#endif
}

/*!Copies the complete structural data of the given image into the current one. Assumes the 
 * current image does not already contain any dynamically allocated data (i.e. it does 
 * \b not deallocate dynamic memory!).
 */
template < class PIXTYPE >
template < class PIXTYPE2 >
void Image< PIXTYPE >::CopyStructuralDataFrom_Simple(const Image< PIXTYPE2 >& img)
{
    cdim = img.cdim;
    xdim = img.xdim;
    ydim = img.ydim;
    colorspace = img.colorspace;

    int n;
    PIXTYPE *h, **hh;

    pixmap = new PIXTYPE[cdim*xdim*ydim];

    gray = new PIXTYPE*[cdim*ydim];
    for (n=0, h=pixmap; n<cdim*ydim; ++n, h+=xdim) 
        gray[n] = h;

    color = new PIXTYPE**[cdim+1];
    for (n=0, hh=gray; n<cdim; ++n, hh+=ydim) 
        color[n] = hh;

    std::copy(img.pixmap, img.pixmap+cdim*xdim*ydim, pixmap);

    if (img.HasAlpha())
    {
        //the alpha channel will be stored in an own array, so as to allow 
        //a more efficient subsequent processing
        alphamap = new PIXTYPE[xdim*ydim];
        alpha = new PIXTYPE*[ydim];
        for (n=0, h=alphamap; n<ydim; ++n, h+=xdim) 
            alpha[n] = h;
        color[cdim]=alpha;
        std::copy(img.alphamap, img.alphamap+xdim*ydim, alphamap);
    }
    else
    {
        alphamap = NULL;
        alpha = NULL;
    }
}

/*!Copies the complete data of the given image into the current one. Assumes the 
 * current image does not already contain any dynamically allocated descriptive data (i.e. it does 
 * \b not deallocate dynamic memory!).
 */
template < class PIXTYPE >
template < class PIXTYPE2 >
inline void Image< PIXTYPE >::CopyDataFrom_Simple(const Image< PIXTYPE2 >& img)
{
    CopyStructuralDataFrom_Simple<PIXTYPE2 >(img);
    CopyDescriptiveDataFrom_Simple<PIXTYPE2 >(img);
}

/*!
 * \pre x_dim >= 0 ; y_dim >= 0
 */
template < class PIXTYPE >
Image< PIXTYPE >::Image(int x_dim, int y_dim, COLORSPACE cs, bool with_alpha)
{
    assert((x_dim>=0) && (y_dim>=0));

    //even if the preconditions aren't met, make sure the members are not "random data"
    InitDefaults();

    colorspace = cs;

    if ((x_dim<=0) || (y_dim<=0))
        return;
    
    int n;
    PIXTYPE *h, **hh;

    cdim = static_cast<int>(ColorSpaceTraits::GetDimension(colorspace));

    xdim = x_dim;
    ydim = y_dim;

    pixmap = new PIXTYPE[cdim*xdim*ydim];

    gray = new PIXTYPE*[cdim*ydim];
    for (n=0, h=pixmap; n<cdim*ydim; ++n, h+=xdim) 
        gray[n] = h;

    color = new PIXTYPE**[cdim+1];
    for (n=0, hh=gray; n<cdim; ++n, hh+=ydim) 
        color[n] = hh;

    if (with_alpha)
    {
        //the alpha channel will be stored in an own array, so as to allow 
        //a more efficient subsequent processing
        alphamap = new PIXTYPE[xdim*ydim];
        alpha = new PIXTYPE*[ydim];
        for (n=0, h=alphamap; n<ydim; ++n, h+=xdim) 
            alpha[n] = h;
        color[cdim] = alpha;
//         if (std::numeric_limits< PIXTYPE >::is_specialized)
//         {
//             std::fill(alphamap, alphamap+xdim*ydim, static_cast<PIXTYPE>(255u));
//         }
        if (std::numeric_limits< PIXTYPE >::is_specialized)
        {
            PIXTYPE *p = alphamap;
//             PIXTYPE v = static_cast<PIXTYPE>(std::numeric_limits< PIXTYPE >::is_specialized*255u);
            PIXTYPE v = (PIXTYPE)(255u);
            for (int i=xdim*ydim; i>0; i--)
                *(p++)= v;
        }
        else
        {
            PIXTYPE *p = alphamap;
            for (int i=xdim*ydim; i>0; i--)
                *(p++) = PIXTYPE();
        }
    }
}

template < class PIXTYPE >
inline Image< PIXTYPE >::Image(const Image< PIXTYPE >& img)
{
    CopyDataFrom_Simple<PIXTYPE >(img);
}

template < class PIXTYPE >
inline Image< PIXTYPE >::Image(const Image< PIXTYPE >* img)
{
    CopyDataFrom_Simple<PIXTYPE >(*img);
}

template < class PIXTYPE >
template < class PIXTYPE2 >
inline Image< PIXTYPE >::Image(const Image< PIXTYPE2 >& img)
{
    CopyDataFrom_Simple<PIXTYPE2 >(img);
}

template < class PIXTYPE >
inline const Image< PIXTYPE >& Image< PIXTYPE >::operator=(const Image< PIXTYPE >& img)
{
    //NOTE: use dynamic_cast so as to correctly be able to compare in hierarchies of classes!
    //dynamic_cast<void*> will point to the most derived class' start (unlike other casts)
    if (dynamic_cast<const void*>(&img) != dynamic_cast<const void*>(this))
    {
        //TODO: optimize this to not perform any new allocations in case size is the same! - >
        //also do this for public structural data copying function...
        Release();
        CopyDataFrom_Simple<PIXTYPE >(img);
    }
    
    return *this;
}

template < class PIXTYPE >
template < class PIXTYPE2 >
inline const Image< PIXTYPE >& Image< PIXTYPE >::operator=(const Image< PIXTYPE2 >& img)
{
    //NOTE: use dynamic_cast so as to correctly be able to compare in hierarchies of classes!
    //dynamic_cast<void*> will point to the most derived class' start (unlike other casts)
    if (dynamic_cast<const void*>(&img) != dynamic_cast<const void*>(this))
    {
        Release();
        CopyDataFrom_Simple<PIXTYPE2 >(img);
    }
    
    return *this;
}

/*!Copies the complete descriptive data of the given image into the current one.
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::CopyDescriptiveDataFrom(const Image< PIXTYPE >& img, bool copyicc)
{
    //NOTE: use dynamic_cast so as to correctly be able to compare in hierarchies of classes!
    //dynamic_cast<void*> will point to the most derived class' start (unlike other casts)
    if (dynamic_cast<const void*>(&img) != dynamic_cast<const void*>(this))
    {
        ReleaseDescriptiveData();
        CopyDescriptiveDataFrom_Simple<PIXTYPE >(img, copyicc);
    }
}

/*!Copies the complete descriptive data of the given image into the current one.
 */
template < class PIXTYPE >
template < class PIXTYPE2 >
inline void Image< PIXTYPE >::CopyDescriptiveDataFrom(const Image< PIXTYPE2 >& img, bool copyicc)
{
    //NOTE: use dynamic_cast so as to correctly be able to compare in hierarchies of classes!
    //dynamic_cast<void*> will point to the most derived class' start (unlike other casts)
    if (dynamic_cast<const void*>(&img) != dynamic_cast<const void*>(this))
    {
        ReleaseDescriptiveData();
        CopyDescriptiveDataFrom_Simple<PIXTYPE2 >(img, copyicc);
    }
}

/*!Copies the complete structural data of the given image into the current one.
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::CopyStructuralDataFrom(const Image< PIXTYPE >& img)
{
    //NOTE: use dynamic_cast so as to correctly be able to compare in hierarchies of classes!
    //dynamic_cast<void*> will point to the most derived class' start (unlike other casts)
    if (dynamic_cast<const void*>(&img) != dynamic_cast<const void*>(this))
    {
        ReleaseStructuralData();
        CopyStructuralDataFrom_Simple<PIXTYPE >(img);
    }
}

/*!Copies the complete structural data of the given image into the current one. Assumes the 
 * current image does not already contain any dynamically allocated data (i.e. it does 
 * \b not deallocate dynamic memory!).
 */
template < class PIXTYPE >
template < class PIXTYPE2 >
inline void Image< PIXTYPE >::CopyStructuralDataFrom(const Image< PIXTYPE2 >& img)
{
    //NOTE: use dynamic_cast so as to correctly be able to compare in hierarchies of classes!
    //dynamic_cast<void*> will point to the most derived class' start (unlike other casts)
    if (dynamic_cast<const void*>(&img) != dynamic_cast<const void*>(this))
    {
        ReleaseStructuralData();
        CopyStructuralDataFrom_Simple<PIXTYPE2 >(img);
    }
}

/*!Releases all dynamic memory allocated for structural data members.
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::ReleaseStructuralData()
{
    if (color!=NULL)
    {
        delete[] color;
        color=NULL;
    }
    if (gray!=NULL)
    {
        delete[] gray;
        gray=NULL;
    }
    if (pixmap!=NULL)
    {
        delete[] pixmap;
        pixmap=NULL;
    }
    if (alpha!=NULL)
    {
        delete[] alpha;
        alpha=NULL;
    }
    if (alphamap!=NULL)
    {
        delete[] alphamap;
        alphamap=NULL;
    }
}

/*!Releases all dynamic memory allocated for descriptive data members.
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::ReleaseDescriptiveData()
{
    //no dynamic descriptive data atm
#ifndef NO_LCMS
    if (icc!=NULL)
    {
        cmsCloseProfile(icc);
        icc = NULL;
    }
#endif
}

/*!Releases all dynamic memory allocated for the image.
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::Release()
{
    ReleaseDescriptiveData();
    ReleaseStructuralData();
}

template < class PIXTYPE >
inline Image< PIXTYPE >::~Image()
{
    Release();
}

template < class PIXTYPE >
inline bool Image< PIXTYPE >::HasAlpha() const
{
    return (alphamap!=NULL);
}

template < class PIXTYPE >
int Image< PIXTYPE >::AddAlphaChannel()
{
    if (HasAlpha())
        return 0;
    
    PIXTYPE *h;
    int n, i;

    alphamap = new PIXTYPE[xdim*ydim];
    alpha = new PIXTYPE*[ydim];
    for (n=0, h=alphamap; n<ydim; n++, h+=xdim) 
        alpha[n] = h;
    color[cdim] = alpha;
    if (std::numeric_limits< PIXTYPE >::is_specialized)
    {
        for (i=xdim*ydim, h=alphamap; i>0; i--)
           *(h++) = static_cast<PIXTYPE>(255u);
    }
    else
    {
        for (i=xdim*ydim, h=alphamap; i>0; i--)
           *(h++) = PIXTYPE();
    }

    return 0;
}

template < class PIXTYPE >
inline int Image< PIXTYPE >::RemoveAlphaChannel()
{
    if (!HasAlpha())
        return 0;

    delete [] alpha;
    alpha = NULL;
    delete [] alphamap;
    alphamap = NULL;
    color[cdim] = NULL;
    
    return 0;
}

#ifndef NO_LCMS
template < class PIXTYPE >
int Image< PIXTYPE >::AddICC()
{
    if (icc != NULL)
        return -1;
        
    switch (colorspace)
    {
        case RGB:
            icc = cmsCreate_sRGBProfile();
            break;
        case GRAYSCALE:
            icc = cmsCreate_sRGB_YProfile();
            break;
        case LAB:
            icc = cmsCreateLab4Profile(NULL);
            break;
        case XYZ:
            icc = cmsCreateXYZProfile();
            break;
        default:
            return -1;
    }
    
    return 0;
}

template < class PIXTYPE >
int Image< PIXTYPE >::AddICC(cmsHPROFILE aicc)
{
    if (aicc == NULL)
        return -1;
        
    if (icc != NULL)
    {
        cmsCloseProfile(icc);
        icc = NULL;
    }
    
    int cs = _cmsLCMScolorSpace(cmsGetColorSpace(aicc));
  
    if ((colorspace == RGB && cs == PT_RGB) ||
        (colorspace == GRAYSCALE && cs == PT_GRAY) ||
        (colorspace == CMYK && cs == PT_CMYK) ||
        (colorspace == YCBCR && cs == PT_YCbCr) ||
        (colorspace == XYZ && cs == PT_XYZ))
    {
        icc = aicc;
        return 0;
    }
    
    return -1;
}

template < class PIXTYPE >
inline int Image< PIXTYPE >::RemoveICC()
{
    if (icc == NULL)
        return 0;

    cmsCloseProfile(icc);
    icc = NULL;
                    
    return 0;
}
#endif

template < class PIXTYPE >
inline PIXTYPE Image< PIXTYPE >::GetMaxGrayValue() const
{
    return (*(std::max_element(pixmap, pixmap+cdim*xdim*ydim)));
}

template < class PIXTYPE >
inline PIXTYPE Image< PIXTYPE >::GetMinGrayValue() const
{
    return (*(std::min_element(pixmap, pixmap+cdim*xdim*ydim)));
}

template < class PIXTYPE >
inline PIXTYPE Image< PIXTYPE >::GetMaxAlphaValue() const
{
    return (*(std::max_element(alphamap, alphamap+xdim*ydim)));
}

template < class PIXTYPE >
inline PIXTYPE Image< PIXTYPE >::GetMinAlphaValue() const
{
    return (*(std::min_element(alphamap, alphamap+xdim*ydim)));
}

template < class PIXTYPE >
inline void Image< PIXTYPE >::Fill(PIXTYPE val)
{
    std::fill(pixmap, pixmap+cdim*xdim*ydim, val);
}

template < class PIXTYPE >
inline void Image< PIXTYPE >::Clear()
{
    Fill(static_cast< PIXTYPE >(0));
}

/*!Swap operator. Also useful for forcefully deallocating the reserved memory image 
 * (i.e. dynamic data) - Image<PixType>(0, 0, GRAYSCALE, false).swap(existingimg)
 */
template < class PIXTYPE >
inline void Image< PIXTYPE >::swap(Image< PIXTYPE >& img)
{
    //NOTE: use dynamic_cast so as to correctly be able to compare in hierarchies of classes!
    //dynamic_cast<void*> will point to the most derived class' start (unlike other casts)
    if (dynamic_cast<const void*>(&img) == dynamic_cast<const void*>(this))
        return;

    //structural
    std::swap(xdim, img.xdim);
    std::swap(ydim, img.ydim);
    std::swap(cdim, img.cdim);
    std::swap(colorspace, img.colorspace);
    std::swap(pixmap, img.pixmap);
    std::swap(gray, img.gray);
    std::swap(color, img.color);
    std::swap(alphamap, img.alphamap);
    std::swap(alpha, img.alpha);

    //descriptive
    std::swap(dpi, img.dpi);
    std::swap(transform, img.transform);
#ifndef NO_LCMS
    std::swap(icc, img.icc);
#endif
}

} //namespace

#endif //_IMAGE_BASIC_H_
