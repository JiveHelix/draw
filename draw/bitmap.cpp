#include "draw/bitmap.h"

WXSHIM_PUSH_IGNORES
#include <wx/image.h>
WXSHIM_POP_IGNORES

#include <wxpex/size.h>


namespace draw
{


tau::RgbPixels<uint8_t> GetRgbPixels(const wxBitmap &bitmap)
{
    wxImage image = bitmap.ConvertToImage();

    auto size = wxpex::ToSize<Eigen::Index>(bitmap.GetSize());
    return tau::RgbPixels<uint8_t>::Create(size, image.GetData());
}


MonoImage<int32_t> GetMonoImage(const wxBitmap &bitmap, int32_t maximumValue)
{
    auto rgbPixels = GetRgbPixels(bitmap);     
    auto planarRgb = rgbPixels.GetPlanar();
    auto planarHsv = tau::RgbToHsv<double>(planarRgb);

    // HSV values are in the range 0.0 to 1.0;
    auto value = GetValue(planarHsv);

    // Scale the maximum value.
    return (value.array() * static_cast<double>(maximumValue))
        .template cast<int32_t>();
}


} // end namespace draw
