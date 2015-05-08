#include "icolor.h"

#include <vector>


namespace image
{

//definition and initialization of static class members
//------------------------------------------------------------------------------------
//bool ColorSpaceTraits::is_initialized=false;
//std::vector<ColorSpaceTraits::ColorSpaceInfos > ColorSpaceTraits::col_infos;
//------------------------------------------------------------------------------------

struct ColorSpaceInfos
{
    bool is_specialized;
    bool is_rgb_connected;
    std::size_t dimension;
    std::string stringID;
} col_infos[] = {    
    { true, true,  std::size_t(1), "grayscale" }, 
    { true, true,  std::size_t(3), "rgb"},
    { true, true,  std::size_t(3), "ycbcr"},
    { true, false, std::size_t(3), "l*a*b*"},
    { true, true,  std::size_t(4), "cmyk"},
    { true, false, std::size_t(3), "xyz"},
//    { true, true,  std::size_t(1), "sgray"},
};
                                        

bool ColorSpaceTraits::IsSpecialized(const COLORSPACE cspace)
{
    assert((cspace>=0) && (cspace<COLORSPACE_COUNT));
    
    return col_infos[(std::size_t)cspace].is_specialized;
}

bool ColorSpaceTraits::IsRGBConnected(const COLORSPACE cspace)
{
    assert((cspace>=0) && (cspace<COLORSPACE_COUNT));

    return col_infos[(std::size_t)cspace].is_rgb_connected;
}

std::size_t ColorSpaceTraits::GetDimension(const COLORSPACE cspace)
{
    assert((cspace>=0) && (cspace<COLORSPACE_COUNT));
    
    return col_infos[(std::size_t)cspace].dimension;
}

std::string ColorSpaceTraits::GetStringID(const COLORSPACE cspace)
{
    assert((cspace>=0) && (cspace<COLORSPACE_COUNT));
    
    return col_infos[(std::size_t)cspace].stringID;
}
                                        


}
