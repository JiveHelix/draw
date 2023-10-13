#include "draw/detail/png_image.h"


namespace draw
{


namespace detail
{


png_uint_32 PngImage::ToPngInt(Eigen::Index value)
{
    if (!jive::CheckConvertible<png_uint_32>(value))
    {
        throw PngError("Value is too large for png format.");
    }

    return static_cast<png_uint_32>(value);
}


void PngImage::SetWidth(Eigen::Index width)
{
    this->image_.width = ToPngInt(width);
}


void PngImage::SetHeight(Eigen::Index height)
{
    this->image_.height = ToPngInt(height);
}


Eigen::Index PngImage::GetWidth() const
{
    return static_cast<Eigen::Index>(this->image_.width);
}


Eigen::Index PngImage::GetHeight() const
{
    return static_cast<Eigen::Index>(this->image_.height);
}


void PngImage::CheckErrors_() const
{
    if (this->image_.warning_or_error > 2)
    {
        throw PngError(this->image_.message);
    }

    if (this->image_.warning_or_error == 1)
    {
        std::cerr << "PNG Warning: " << this->image_.message << std::endl;
    }
}


void PngImage::CheckReadErrors_(
    const std::string &fileName,
    int pngResult) const
{
    this->CheckErrors_();

    if (pngResult != 1)
    {
        throw PngError(std::string("Failed to read from ") + fileName);
    }
}


void PngImage::CheckWriteErrors_(
    const std::string &fileName,
    int pngResult) const
{
    this->CheckErrors_();

    if (pngResult != 1)
    {
        throw PngError(std::string("Failed to write to ") + fileName);
    }
}


} // end namespace detail


} // end namespace draw
