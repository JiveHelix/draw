#pragma once


#include <string>
#include <png.h>
#include <tau/eigen.h>
#include <tau/color.h>
#include "draw/error.h"
#include "draw/planar.h"
#include "draw/gray.h"


namespace draw
{


CREATE_EXCEPTION(PngError, DrawError);


namespace detail
{


class PngImage
{
public:
    PngImage()
    {
        memset(&this->image_, 0, sizeof(png_image));
        this->image_.version = PNG_IMAGE_VERSION;
    }

    ~PngImage()
    {
        if (this->image_.opaque)
        {
            png_image_free(&this->image_);
        }
    }

    static png_uint_32 ToPngInt(Eigen::Index value);
    void SetWidth(Eigen::Index width);
    void SetHeight(Eigen::Index height);

    Eigen::Index GetWidth() const;
    Eigen::Index GetHeight() const;

    template<typename Pixel>
    using Interleaved =
        Eigen::Matrix<Pixel, Eigen::Dynamic, 3, Eigen::RowMajor>;

    template<typename Pixel>
    Interleaved<Pixel>
    ReadRgb(const std::string &fileName, png_uint_32 pngFormat)
    {
        int pngResult =
            png_image_begin_read_from_file(&this->image_, fileName.c_str());

        this->CheckReadErrors_(fileName, pngResult);

        this->image_.format = pngFormat;

        Interleaved<Pixel> interleaved(this->GetWidth() * this->GetHeight(), 3);

        pngResult = png_image_finish_read(
            &this->image_,
            nullptr,
            interleaved.data(),
            0,
            nullptr);

        this->CheckReadErrors_(fileName, pngResult);

        return interleaved;
    }

    template<typename Pixel>
    Gray<Pixel> ReadGray(const std::string &fileName, png_uint_32 pngFormat)
    {
        this->image_.version = PNG_IMAGE_VERSION;

        int pngResult =
            png_image_begin_read_from_file(&this->image_, fileName.c_str());

        this->CheckReadErrors_(fileName, pngResult);

        this->image_.format = pngFormat;

        Gray<Pixel> result(this->GetHeight(), this->GetWidth());

        pngResult = png_image_finish_read(
            &this->image_,
            nullptr,
            result.data(),
            0,
            nullptr);

        this->CheckReadErrors_(fileName, pngResult);

        return result;
    }

    template<typename Pixel>
    void WriteRgb(
        const std::string &fileName,
        const PlanarRgb<Pixel> &planarRgb,
        png_uint_32 pngFormat)
    {
        this->image_.format = pngFormat;
        this->SetWidth(planarRgb.GetColumnCount());
        this->SetHeight(planarRgb.GetRowCount());

        Interleaved<Pixel> interleaved =
            planarRgb.template GetInterleaved<Eigen::RowMajor>();

        int pngResult = png_image_write_to_file(
            &this->image_,
            fileName.c_str(),
            0,
            interleaved.data(),
            0,
            nullptr);

        this->CheckWriteErrors_(fileName, pngResult);
    }

    template<typename Pixel>
    void WriteRgb(
        const std::string &fileName,
        const tau::RgbPixels<Pixel> &rgbPixels,
        png_uint_32 pngFormat)
    {
        this->image_.format = pngFormat;
        this->SetWidth(rgbPixels.size.width);
        this->SetHeight(rgbPixels.size.height);

        int pngResult = png_image_write_to_file(
            &this->image_,
            fileName.c_str(),
            0,
            rgbPixels.data.data(),
            0,
            nullptr);

        this->CheckWriteErrors_(fileName, pngResult);
    }

    template<typename Pixel>
    void WriteGray(
        const std::string &fileName,
        const Gray<Pixel> &values,
        png_uint_32 pngFormat)
    {
        this->image_.format = pngFormat;
        this->SetWidth(values.cols());
        this->SetHeight(values.rows());

        int pngResult = png_image_write_to_file(
            &this->image_,
            fileName.c_str(),
            0,
            values.data(),
            0,
            nullptr);

        this->CheckWriteErrors_(fileName, pngResult);
    }

private:
    void CheckErrors_() const;
    void CheckReadErrors_(const std::string &fileName, int pngResult) const;
    void CheckWriteErrors_(const std::string &fileName, int pngResult) const;

private:
    png_image image_;
};


} // end namespace detail


} // end namespace draw
