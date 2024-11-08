#pragma once


#include <wxpex/wxshim.h>
#include <tau/color.h>
#include "draw/mono_image.h"


WXSHIM_PUSH_IGNORES
#include <wx/bitmap.h>
WXSHIM_POP_IGNORES


namespace draw
{


tau::RgbPixels<uint8_t> GetRgbPixels(const wxBitmap &bitmap);

MonoImage<int32_t> GetMonoImage(const wxBitmap &bitmap, int32_t maximumValue);

wxBitmap GetBitmap(const tau::RgbPixels<uint8_t> &pixels);


} // end namespace draw
