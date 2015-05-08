#ifndef PNG_IO_H_
#define PNG_IO_H_

#include "definitionImageLib.h"
#include "image.h"

#ifndef NO_PNG


namespace image
{

/*!\defgroup PNGIO PNG Format Input/Output
 * \brief Include header file(s): "png_io.h".
 * \ingroup ImageIO
 */
//@{
   

EXPORT_IMAGELIB IMAGE* LoadPNG(const char *filename);
EXPORT_IMAGELIB int    SavePNG(const IMAGE *img, const char *filename, int numbit=8);


//@}

}

#endif //ndef NO_PNG

#endif //ndef PNG_IO_H

