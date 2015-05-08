#ifndef _LIBIMG_ERROR_HANDLING_
#define _LIBIMG_ERROR_HANDLING_

#include <cstdarg>
#include <cstdio>
#include <cstdlib>


namespace image
{

#ifndef ERRORDEF
#define ERRORDEF
/*     name:   Error
 *     author: Becker
 *     date:   04.97
 *     func.:  print error messages
 */
inline void ErrorImg(const char *message, ...)
{
#ifdef LINUX
    va_list ap;

    fflush(stdout);
    va_start(ap,message);
    fprintf(stderr,"ERROR: ");
    vfprintf(stderr, message, ap);
    va_end(ap);
    fprintf(stderr,"\n\n");
    fflush(stderr);
    exit(EXIT_FAILURE);
#endif //def LINUX
} 

#endif //ndef ERRORDEF

}

#endif //ndef _LIBIMG_ERROR_HANDLING
