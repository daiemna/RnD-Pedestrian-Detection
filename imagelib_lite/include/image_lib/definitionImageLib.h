#ifndef DEFINITIONIMAGELIB_H
#define DEFINITIONIMAGELIB_H

#include <cstdio>

/*!\defgroup CommonDefs Common ImageLib Definitions (compile-time flags, macros)  
 * \brief Include header file(s): "definitionImageLib.h"
 */

namespace image
{

/*!\addtogroup CommonDefs  
 */
//@{


#define DEBUG_OUTPUT 0

#if DEBUG_OUTPUT
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG printf("debug %s,%d: ", __FILE__, __LINE__),printf 
#endif

#if !DEBUG_OUTPUT
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG (void)
#endif


//----------------------------------------------------------------------------
// Windows Settings
#if defined( _LIB ) || defined( STATIC_IMAGELIB )
#define EXPORT_IMAGELIB
#define EXPIMP_TEMPLATE
#endif

#ifndef EXPORT_IMAGELIB
    #if defined( __WIN32__ ) || defined( _WIN32 )
    // If we're not including this from a client build, specify that the stuff
    // should get exported. Otherwise, import it.
    #   if defined( __MINGW32__ )
    // Linux compilers don't have symbol import/export directives.
    #       define EXPORT_IMAGELIB
    #       define EXPIMP_TEMPLATE
    #   else
    #       if defined( imagelib_EXPORTS )
    #           define EXPORT_IMAGELIB __declspec( dllexport )
    #           define EXPIMP_TEMPLATE
    #       else
    #           define EXPORT_IMAGELIB __declspec( dllimport )
    #           define EXPIMP_TEMPLATE extern
    #       endif
    #   endif
    #else
    #   if (__GNUC__)*100 >= 400
    #       define EXPORT_IMAGELIB  __attribute__ ((visibility("default")))
    #       define EXPIMP_TEMPLATE
    #   else
    #       define EXPORT_IMAGELIB
    #       define EXPIMP_TEMPLATE
    #   endif
    #endif
#endif

//@}


} //namespace
 
#endif // DEFINITIONIMAGELIB_H



