#pragma once


#include <string>
#include <tau/eigen.h>
#include <tau/color.h>
#include "draw/error.h"
#include "draw/size.h"
#include "draw/planar.h"
#include "draw/gray.h"


namespace draw
{


PlanarRgb<uint8_t> ReadPng(const std::string &fileName);
PlanarRgb<uint16_t> ReadPng48(const std::string &fileName);
Gray<uint8_t> ReadPngGray8(const std::string &fileName);
Gray<uint16_t> ReadPngGray16(const std::string &fileName);


void WritePng(
    const std::string &fileName,
    const PlanarRgb<uint8_t> &planarRgb);


void WritePng(
    const std::string &fileName,
    const tau::RgbPixels<uint8_t> &rgbPixels);


void WritePng48(
    const std::string &fileName,
    const PlanarRgb<uint16_t> &planarRgb,
    bool convert);


void WritePngGray8(
    const std::string &fileName,
    const Gray<uint8_t> &gray);


void WritePngGray16(
    const std::string &fileName,
    const Gray<uint16_t> &gray);


template<typename Pixel>
class Png
{
public:
    using Rgb = PlanarRgb<Pixel>;
    using Hsv = PlanarHsv<double>;

    template<typename Scalar>
    using PngMatrix =
        Eigen::Matrix
        <
            Scalar,
            Eigen::Dynamic,
            Eigen::Dynamic,
            Eigen::RowMajor
        >;

    using Matrix = PngMatrix<Pixel>;

    Png() = default;

    Png(const std::string &fileName, bool high)
    {
        if (high)
        {
            assert(sizeof(Pixel) >= 2);
            this->rgb_ = ReadPng48(fileName).template Cast<Pixel>();
        }
        else
        {
            this->rgb_ = ReadPng(fileName).template Cast<Pixel>();
        }
    }

    void Write(const std::string &fileName, bool high)
    {
        if (high)
        {
            WritePng48(fileName, this->rgb_->template Cast<uint16_t>(), false);
        }
        else
        {
            WritePng(fileName, this->rgb_->template Cast<uint8_t>());
        }
    }

    operator bool ()
    {
        return this->rgb_.has_value();
    }

    Rgb & GetRgb()
    {
        return *this->rgb_;
    }

    const Rgb & GetRgb() const
    {
        return *this->rgb_;
    }

    PlanarRgb<Pixel> GetRgbPixels() const
    {
        return this->rgb_;
    }

    Hsv GetHsv() const
    {
        if (!this->rgb_)
        {
            throw std::runtime_error("No data");
        }

        return tau::RgbToHsv<double>(*this->rgb_);
    }

#if 0
    Matrix GetValue(double scale) const
    {
        if (!this->rgb_)
        {
            throw std::runtime_error("No data");
        }

        using Sum = PngMatrix<Pixel>;
        auto size = this->rgb_->GetSize();
        Sum sum = Sum::Zero(size.height, size.width);
        sum.array() += tau::GetRed(*this->rgb_).array();
        sum.array() += tau::GetGreen(*this->rgb_).array();
        sum.array() += tau::GetBlue(*this->rgb_).array();

        using AsDouble = PngMatrix<double>;
        AsDouble values = sum.template cast<double>();
        values.array() *= scale;

        return values.template cast<Pixel>();
    }
#endif

    Size GetSize() const
    {
        if (!this->rgb_)
        {
            throw std::runtime_error("No png data");
        }

        return this->rgb_->GetSize();
    }

    std::optional<Rgb> rgb_;
};


template<typename Pixel>
class GrayPng
{
public:
    using Values = Gray<Pixel>;

    GrayPng() = default;

    GrayPng(const std::string &fileName)
        :
        values_(Values())
    {
        if constexpr (sizeof(Pixel) >= 2)
        {
            this->values_ =
                ReadPngGray16(fileName).template cast<Pixel>();
        }
        else
        {
            this->values_ =
                ReadPngGray8(fileName).template cast<Pixel>();
        }
    }

    operator bool ()
    {
        return this->values_.has_value();
    }

    Values & GetValues()
    {
        if (!this->values_)
        {
            throw std::runtime_error("No png data");
        }

        return *this->values_;
    }

    const Values & GetValues() const
    {
        if (!this->values_)
        {
            throw std::runtime_error("No png data");
        }

        return *this->values_;
    }

    Size GetSize() const
    {
        if (!this->values_)
        {
            throw std::runtime_error("No png data");
        }

        auto columns = this->values_->cols();
        auto rows = this->values_->rows();

        assert(rows <= std::numeric_limits<SizeType>::max());
        assert(columns <= std::numeric_limits<SizeType>::max());

        return Size(SizeType(columns), SizeType(rows));
    }

    void Write(const std::string &fileName, bool high)
    {
        if (high)
        {
            WritePngGray16(fileName, this->values_->template cast<uint16_t>());
        }
        else
        {
            WritePngGray8(fileName, this->values_->template cast<uint8_t>());
        }
    }

    std::optional<Values> values_;
};


} // end namespace draw
