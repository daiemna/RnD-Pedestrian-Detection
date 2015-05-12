#ifndef _IMAGE_PIXEL_TYPES_H_
#define _IMAGE_PIXEL_TYPES_H_

#include <cstdint>


namespace image
{


//basic pixel types
//typedef unsigned char PIX;       //!< PIX = unsigned integer, 8 bits
//typedef short int SPIX;           //!< SPIX = signed integer, 16 bits
//typedef int IPIX;                 //!< IPIX = signed integer, 32 bits
//typedef long long LPIX;           //!< LPIX = signed integer, 64 bits
typedef uint8_t PIX;            //!< PIX = unsigned integer, 8 bits
typedef int16_t SPIX;           //!< SPIX = signed integer, 16 bits
typedef int32_t IPIX;           //!< IPIX = signed integer, 32 bits
typedef int64_t LPIX;           //!< LPIX = signed integer, 64 bits

typedef float FPIX;               //!< FPIX = floating-point, 32 bits
typedef double DPIX;              //!< DPIX = floating-point, 64 bits

//NOTE: a 1-bit pixel type class may also be defined here... 


}


#endif //ndef _IMAGE_PIXEL_TYPES_H_
