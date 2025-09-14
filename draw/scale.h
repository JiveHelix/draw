#pragma once


#include <tau/scale.h>
#include <pex/group.h>
#include <pex/range.h>
#include <pex/selectors.h>
#include <pex/endpoint.h>
#include <wxpex/slider.h>


namespace draw
{


struct ScaleTraits: pex::DefaultConverterTraits
{
    static constexpr int width = 5;
    static constexpr int precision = 3;
};


template<typename T>
struct FormatScale:
    public pex::Converter<T, ScaleTraits>
{
    template<typename U>
    static std::string ToString(U &&value)
    {
        return pex::Converter<T, ScaleTraits>::ConvertToString::Call(
            std::forward<U>(value)) + "x";
    }
};


using ScaleType = double;
using Scale = tau::Scale<ScaleType>;

// Initial range for Scale is from 0.25x to 16.0x
using ScaleRange =
    pex::MakeRange<ScaleType, pex::Limit<0, 25, 100>, pex::Limit<16>>;

using ScaleGroup =
    pex::Group
    <
        tau::ScaleFields,
        tau::ScaleTemplate<ScaleRange>::template Template,
        pex::PlainT<Scale>
    >;

using ScaleModel = typename ScaleGroup::Model;
using ScaleControl = typename ScaleGroup::DefaultControl;

template<typename Observer>
using ScaleEndpoint = pex::EndpointGroup<Observer, ScaleControl>;


using LogarithmicScale = pex::control::LogarithmicRange
    <
        pex::ModelSelector<ScaleRange>,
        2,
        100
    >;

using LinearScale =
    typename pex::ControlSelector<ScaleRange>::Value;

using ScaleSlider =
    wxpex::ValueSliderConvert
    <
        LogarithmicScale,
        LinearScale,
        FormatScale<ScaleType>
    >;


} // end namespace draw
