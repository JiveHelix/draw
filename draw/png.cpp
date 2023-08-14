#include "draw/png.h"
#include <jive/overflow.h>


namespace draw
{


static png_uint_32 ToPngInt(Eigen::Index value)
{
    if (!jive::CheckConvertible<png_uint_32>(value))
    {
        throw PngError("Value is too large for png format.");
    }

    return static_cast<png_uint_32>(value);
}


template<typename Pixel, typename Format>
PlanarRgb<Pixel> DoReadPng(const std::string &fileName, Format pngFormat)
{
    using Eigen::Index;

    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    png_image_begin_read_from_file(&image, fileName.c_str());
    image.format = pngFormat;

    using Interleaved =
        Eigen::Matrix<Pixel, Eigen::Dynamic, 3, Eigen::RowMajor>;

    Interleaved interleaved(Index(image.height * image.width), 3);

    png_image_finish_read(
        &image,
        nullptr,
        interleaved.data(),
        0,
        nullptr);

    if (image.opaque)
    {
        png_image_free(&image);
    }

    if (image.warning_or_error > 2)
    {
        throw PngError(image.message);
    }

    if (image.warning_or_error == 1)
    {
        std::cerr << "PNG Warning: " << image.message << std::endl;
    }

    return PlanarRgb<Pixel>::FromInterleaved(
        interleaved,
        Index(image.height),
        Index(image.width));
}


PlanarRgb<uint8_t> ReadPng(const std::string &fileName)
{
    return DoReadPng<uint8_t>(fileName, PNG_FORMAT_RGB);
}


PlanarRgb<uint16_t> ReadPng48(const std::string &fileName)
{
    return DoReadPng<uint16_t>(fileName, PNG_FORMAT_LINEAR_RGB);
}


template<typename Pixel, typename Format>
Gray<Pixel> DoReadPngGray(const std::string &fileName, Format pngFormat)
{
    using Eigen::Index;

    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    png_image_begin_read_from_file(&image, fileName.c_str());
    image.format = pngFormat;

    using Result = Gray<Pixel>;

    Result result(Index(image.height), Index(image.width));

    png_image_finish_read(
        &image,
        nullptr,
        result.data(),
        0,
        nullptr);

    if (image.opaque)
    {
        png_image_free(&image);
    }

    if (image.warning_or_error > 2)
    {
        throw PngError(image.message);
    }

    if (image.warning_or_error == 1)
    {
        std::cerr << "PNG Warning: " << image.message << std::endl;
    }

    return result;
}


Gray<uint8_t> ReadPngGray8(const std::string &fileName)
{
    return DoReadPngGray<uint8_t>(fileName, PNG_FORMAT_GRAY);
}


Gray<uint16_t> ReadPngGray16(const std::string &fileName)
{
    return DoReadPngGray<uint16_t>(fileName, PNG_FORMAT_LINEAR_Y);
}


void WritePng(const PlanarRgb<uint8_t> &planarRgb, const std::string &fileName)
{
    using Eigen::Index;

    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    image.format = PNG_FORMAT_RGB;
    image.width = ToPngInt(planarRgb.GetColumnCount());
    image.height = ToPngInt(planarRgb.GetRowCount());

    using Interleaved =
        Eigen::Matrix<uint8_t, Eigen::Dynamic, 3, Eigen::RowMajor>;

    Interleaved interleaved = planarRgb.GetInterleaved<Eigen::RowMajor>();

    int pngResult = png_image_write_to_file(
        &image,
        fileName.c_str(),
        0,
        interleaved.data(),
        0,
        nullptr);

    if (image.opaque)
    {
        png_image_free(&image);
    }

    if (image.warning_or_error > 2)
    {
        throw PngError(image.message);
    }

    if (image.warning_or_error == 1)
    {
        std::cerr << "PNG Warning: " << image.message << std::endl;
    }
}


} // end namespace draw
