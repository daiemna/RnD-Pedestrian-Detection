#ifndef _IMAGE_COLOR_H_
#define _IMAGE_COLOR_H_

#include "definitionImageLib.h"
#include "ipixel_types.h"

#include <cassert>
#include <limits>
#include <cstddef>
#include <string>
#include <vector>
#include <cstdint>

#ifndef NO_LCMS
#include <lcms2.h>
#endif

namespace image
{

/*!\addtogroup ColorSpaceConversions Color Space Conversions
 * \ingroup ImageManipulation
 */
//@{
enum IllObs
{
    TA_2, TC_2, D50_2, D55_2, D65_2, D75_2, F2_2, F7_2, F11_2,
    //TA_10, TC_10, D50_10, D55_10, D65_10, D75_10, F2_10, F7_10, F11_10, 
    NTristimulus
};

const static double TrisValues[NTristimulus][3] =
{
        /* 2 degrees */
        /*ccTA_2*/{109.850, 100.000, 35.585},
        /*ccTC_2*/{98.074,  100.000, 118.232},
        /* D50 - print industry standard */ {96.422,  100.000, 82.521},
        /* D55 */ {95.682,  100.000, 92.149},
        /* D65 - daylight noon photo, sRGB, TV */ {95.047,  100.000, 108.883},
        /* D75 */ {94.972,  100.000, 122.638},
        /* F2 */  {99.187,  100.000, 67.395},
        /* F7 */  {95.044,  100.000, 108.755},
        /* F11 */ {100.966, 100.000, 64.370}//,
//        /* 10 degrees */
//        /* A */   {111.144, 100.000, 35.200},
//        /* C */   {97.285,  100.000, 116.145},
//        /* D50 */ {96.720,  100.000, 81.427},
//        /* D55 */ {95.799,  100.000, 90.926},
//        /* D65 */ {94.811,  100.000, 107.304},
//        /* D75 */ {94.416,  100.000, 120.641},
//        /* F2 */  {103.280, 100.000, 69.026},
//        /* F7 */  {95.792,  100.000, 107.687},
//        /* F11 */ {103.866, 100.000, 65.627}
};

//@}

/*!\addtogroup Colors Color Management
 */

/*!\addtogroup ColorBasic Basic Color Manipulation, Color Spaces and Traits 
 * \ingroup Colors
 * \brief Include header file(s): "icolor.h"
 */
//@{


/*!\c exact_grayvalue_type must be a real type, capable of representing negative, 
 * as well as positive values.
 */
typedef double exact_grayvalue_type; 


/*!32-bitcolor value consisting of 8-bit "unsigned chars": alpha, red, green, blue.
 * \note alpha = 255 <=> fully opaque
 */
//typedef unsigned long ARGB; 
typedef uint32_t ARGB; 

image::PIX GetB(const image::ARGB col);

image::PIX GetG(const image::ARGB col);

image::PIX GetR(const image::ARGB col);

image::PIX GetA(const image::ARGB col);

image::PIX GetC(const int n, const image::ARGB col);

//ARGB MakeARGB(PIX a, PIX r, PIX g, PIX b);

image::ARGB MakeARGB(const int a, const int r, const int g, const int b);

image::ARGB MixARGB (const image::ARGB c1, const image::ARGB c2, const float f);

image::ARGB MulRGBA(const image::ARGB col, const image::PIX f);


/*!Enumeration of all supported colorspaces.
 * It defines the colormodel and a default colorspace. If the colormodel is used with
 * a different colorpace, than an ICC-profile have to be provided in the
 * element icc of the image.  
 * \warning LAB is CIE L*a*b*, not HunterLAB
 */
enum COLORSPACE
{ 
    GRAYSCALE,          //!< luma (Y) using ITU-R BT.601 conversion from sRGB
    RGB,                //!< sRGB 
    YCBCR,              //!< (JPEG conversion) an offset of 128 is used to fit Cb and Cr in the range 0..255
    LAB,                //!< Whitepoint D50 2°, all values range from 0 to 255 for PIX (factor 2.55 for L, offset of 128 for a and b)
    CMYK,               //!< derived from sRGB
    XYZ,                //!< Whitepoint D50 2°
//    SGRAY,              //!< sRGB grayscale
    COLORSPACE_COUNT
};



/*!Class containing information about all supported colorspaces and conversions.
 */
class EXPORT_IMAGELIB  ColorSpaceTraits
{
public:

    static bool IsSpecialized(const image::COLORSPACE cspace);
    static bool IsRGBConnected(const image::COLORSPACE cspace);
    static std::size_t GetDimension(const image::COLORSPACE cspace);
    static std::string GetStringID(const image::COLORSPACE cspace);
};

//@}


//---------------------------------------------------------------------------------------
//                    implementations of INLINEs/ TEMPLATEs
//---------------------------------------------------------------------------------------


inline image::PIX GetB(const image::ARGB col)
{
    return (image::PIX)((col & 0xff));
}

inline image::PIX GetG(const image::ARGB col)
{
    return (image::PIX)((col>>8) & 0xff);
}

inline image::PIX GetR(const image::ARGB col)
{
    return (image::PIX)((col>>16) & 0xff);
}

inline image::PIX GetA(const image::ARGB col)
{
    return (image::PIX)((col>>24)&0xff);
}

/*!Gets the channel components of an ARGB value in the same order as they are used in an RGB image
 * with alpha channel, namely: 0= Red, 1=Green, 2=Blue, 3=Alpha.
 */
inline image::PIX GetC(const int n, const image::ARGB col)
{
    assert((-1<n) && (n<=3));
    
    if (n<3)
        return (image::PIX)((col>>(16-(n*8))) & 0xff);
    return (image::PIX)((col>>24) & 0xff); // alpha
}

//inline ARGB MakeARGB(PIX a, PIX r, PIX g, PIX b)
//{
//    return (((ARGB(a))<<24) | ((ARGB(r))<<16) | ((ARGB(g))<<8) | (ARGB(b)));
//}

inline image::ARGB MakeARGB(const int a, const int r, const int g, const int b)
{
// min and max is defined in windows.h. We havwe to undef them to use min and max functions.
#undef max
#undef min 
    assert((std::numeric_limits<PIX>::min()<=a) && (a<=std::numeric_limits<PIX>::max()));
    assert((std::numeric_limits<PIX>::min()<=r) && (r<=std::numeric_limits<PIX>::max()));
    assert((std::numeric_limits<PIX>::min()<=g) && (g<=std::numeric_limits<PIX>::max()));
    assert((std::numeric_limits<PIX>::min()<=b) && (b<=std::numeric_limits<PIX>::max()));
    
    return (((ARGB(PIX(a)))<<24) | ((ARGB(PIX(r)))<<16) | ((ARGB(PIX(g)))<<8) | (ARGB(PIX(b))));
}

/*!Mixes two ARGB values linearly (per-channel)  = \b alpha \b blending.
 * \param[in] c1
 * \param[in] c2
 * \param[in] f belongs to [0,1]
 * \return result=c1*f+c2*(1-f) for each color channel
 */
inline image::ARGB MixARGB (const image::ARGB c1, const image::ARGB c2, const float f)
{
    assert((f>=0.0f) && (f<=1.0f));
    
    const image::PIX r = static_cast<image::PIX>(GetR(c1)*f + GetR(c2)*(1.0f-f));
    const image::PIX g = static_cast<image::PIX>(GetG(c1)*f + GetG(c2)*(1.0f-f));
    const image::PIX b = static_cast<image::PIX>(GetB(c1)*f + GetB(c2)*(1.0f-f));
    const image::PIX a = static_cast<image::PIX>(GetA(c1)*f + GetA(c2)*(1.0f-f));
    
    return MakeARGB((int)a, (int)r, (int)g, (int)b);
}

inline image::ARGB MulRGBA(const image::ARGB col, const image::PIX f)
{
    return MakeARGB((PIX)(GetA(col)*((unsigned int)f)/256), 
        (PIX)(GetR(col)*((unsigned int)f)/256), 
        (PIX)(GetG(col)*((unsigned int)f)/256), 
        (PIX)(GetB(col)*((unsigned int)f)/256));
}


}


#endif //ndef _IMAGE_COLOR_H_
