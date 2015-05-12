#ifndef IBINARY_H_
#define IBINARY_H_

#include "definitionImageLib.h"
#include "image.h"

namespace image
{

/*!\addtogroup BinaryImages Binary Image-Specific Operations 
\brief Include header file(s): "ibinary.h".
 */
//@{

EXPORT_IMAGELIB bool IsBinaryImage(const IMAGE* img);
EXPORT_IMAGELIB int BinaryMedian(IMAGE *img);
EXPORT_IMAGELIB void DoHorizontalSmearing(IMAGE* im, unsigned int len, bool whiteborder = false);
EXPORT_IMAGELIB void DoVerticalSmearing(IMAGE* im, unsigned int len, bool whiteborder = false);
EXPORT_IMAGELIB void BinaryImageUnion(const IMAGE *src1, const IMAGE* src2, IMAGE *dest);

//@}

}

#endif //IBINARY_H_
