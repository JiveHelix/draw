#pragma once

#include <tau/color_map.h>
#include <tau/color_maps/turbo.h>
#include <tau/color_maps/gray.h>

#include "draw/mono_image.h"
#include "draw/pixels.h"
#include "draw/color_map_settings.h"


namespace draw
{


template<typename T, typename Value>
auto MakeColorMap(const ColorMapSettings<Value> &colorMapSettings)
{
    auto low = colorMapSettings.range.low;
    auto high = colorMapSettings.range.high;

    static constexpr auto maximum =
        static_cast<decltype(low)>(std::numeric_limits<T>::max());

    high = std::min(maximum, high);
    low = std::min(high - 1, low);

    assert(low < high);

    size_t count = static_cast<size_t>(1 + high - low);

    if (colorMapSettings.turbo)
    {
        return tau::LimitedColorMap<draw::PixelMatrix, T>(
            tau::turbo::MakeRgb8(count),
            static_cast<T>(low),
            static_cast<T>(high));
    }
    else
    {
        return tau::LimitedColorMap<draw::PixelMatrix, T>(
            tau::gray::MakeRgb8(count),
            static_cast<T>(low),
            static_cast<T>(high));
    }
}


template<typename Value>
class ColorMap
{
public:
    using Matrix = MonoImage<Value>;

    ColorMap(const ColorMapSettings<Value> &colorMapSettings)
        :
        colorMap_(MakeColorMap<Value>(colorMapSettings))
    {

    }

    draw::Pixels Filter(const Matrix &data) const
    {
        draw::Pixels result{{}, {data.cols(), data.rows()}};
        this->colorMap_(data, &result.data);

        return result;
    }

protected:
    tau::LimitedColorMap<draw::PixelMatrix, Value> colorMap_;
};



extern template class ColorMap<int32_t>;


} // end namespace draw
