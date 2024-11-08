#include "draw/png.h"
#include <jive/overflow.h>
#include <iostream>

#include "draw/detail/png_image.h"


namespace draw
{


template<typename Pixel>
PlanarRgb<Pixel> DoReadPng(const std::string &fileName, png_uint_32 pngFormat)
{
    detail::PngImage image;
    auto interleaved = image.ReadRgb<Pixel>(fileName, pngFormat);

    return PlanarRgb<Pixel>::FromInterleaved(
        interleaved,
        image.GetHeight(),
        image.GetWidth());
}


PlanarRgb<uint8_t> ReadPng(const std::string &fileName)
{
    return DoReadPng<uint8_t>(fileName, PNG_FORMAT_RGB);
}


PlanarRgb<uint16_t> ReadPng48(const std::string &fileName)
{
    return DoReadPng<uint16_t>(fileName, PNG_FORMAT_LINEAR_RGB);
}


Gray<uint8_t> ReadPngGray8(const std::string &fileName)
{
    return detail::PngImage{}.ReadGray<uint8_t>(fileName, PNG_FORMAT_GRAY);
}


Gray<uint16_t> ReadPngGray16(const std::string &fileName)
{
    return detail::PngImage{}.ReadGray<uint16_t>(fileName, PNG_FORMAT_LINEAR_Y);
}


void WritePng(
    const std::string &fileName,
    const PlanarRgb<uint8_t> &planarRgb)
{
    detail::PngImage{}.WriteRgb(fileName, planarRgb, PNG_FORMAT_RGB, false);
}


void WritePng(
    const std::string &fileName,
    const tau::RgbPixels<uint8_t> &rgbPixels)
{
    detail::PngImage{}.WriteRgb(fileName, rgbPixels, PNG_FORMAT_RGB, false);
}


void WritePng48(
    const std::string &fileName,
    const PlanarRgb<uint16_t> &planarRgb,
    bool convert)
{
    detail::PngImage{}.WriteRgb(
        fileName,
        planarRgb,
        PNG_FORMAT_LINEAR_RGB,
        convert);
}


void WritePngGray8(
    const std::string &fileName,
    const Gray<uint8_t> &values)
{
    detail::PngImage{}.WriteGray(fileName, values, PNG_FORMAT_GRAY);
}


void WritePngGray16(
    const std::string &fileName,
    const Gray<uint16_t> &values)
{
    detail::PngImage{}.WriteGray(fileName, values, PNG_FORMAT_LINEAR_Y);
}


} // end namespace draw
