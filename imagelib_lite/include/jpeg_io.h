#ifndef JPEG_IO_H_
#define JPEG_IO_H_

#include "definitionImageLib.h"
#include "image.h"

#ifndef NO_JPEG


namespace image
{

/*!\defgroup JPEGIO JPEG Format Input/Output
 * \brief Include header file(s): "jpeg_io.h".
 * \ingroup ImageIO
 */
//@{
   

EXPORT_IMAGELIB IMAGE*  LoadJPEG(const char *filename);
EXPORT_IMAGELIB IMAGE* LoadJPEGfromMemory(unsigned char *inbuffer, unsigned long insize);
EXPORT_IMAGELIB IMAGE*  LoadJPEGtoColorspace(const char *filename, COLORSPACE cs);
EXPORT_IMAGELIB SIMAGE* LoadJPEGtoJPEG(const char *filename);
EXPORT_IMAGELIB int     SaveJPEG(IMAGE *img, const char *filename, int quality=75, const char *comment=NULL);
EXPORT_IMAGELIB void    JPEGInfo(const char *filename, int& width, int& height, int& dpi);
EXPORT_IMAGELIB int     SaveJPEGwithQualityFromFile(IMAGE *img, const char *outfilename, char *infilename, const char *comment=NULL);
//#if JPEG_LIB_VERSION >= 70
EXPORT_IMAGELIB int     SaveJPEGtoMemory(IMAGE *img, unsigned char **outbuffer, unsigned long *outsize, int quality, const char *comment=NULL);
EXPORT_IMAGELIB int     SaveJPEGwithCompressionrate(IMAGE *img, const char *outfilename, float rate, const char *comment=NULL);

//#endif


//@}

}

#endif //ndef NO_JPEG

#endif //def JPEG_IO_H
