#pragma once


#include <string>
#include <fields/fields.h>
#include <pex/linked_ranges.h>
#include <wxpex/color_picker.h>
#include "draw/size.h"


namespace draw
{


using LowBrightness = pex::Limit<0, 2, 10>;
using HighBrightness = pex::Limit<0, 9, 10>;

using BrightnessRanges =
    pex::LinkedRanges
    <
        double,
        pex::Limit<0>,
        LowBrightness,
        pex::Limit<1>,
        HighBrightness
    >;


template<template<typename> typename T>
struct WaveformColorSchema
{
    T<BrightnessRanges::Group> range;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<64>>> count;
    T<wxpex::HsvGroup> color;
    T<wxpex::HsvGroup> highlightColor;

    static constexpr auto fieldsTypeName = "WaveformColor";
};


struct WaveformColor: public WaveformColorSchema<pex::Identity>
{
    static constexpr size_t defaultCount = 20;

    static constexpr auto defaultHue = 138.0; // green
    static constexpr auto defaultHighlightHue = 292.0; // purple

    WaveformColor()
        :
        WaveformColorSchema<pex::Identity>{
            BrightnessRanges::Settings{},
            defaultCount,
            {{defaultHue, 1.0, 1.0}},
            {{defaultHighlightHue, 1.0, 1.0}}}
    {

    }
};


DECLARE_OUTPUT_STREAM_OPERATOR(WaveformColor)
DECLARE_EQUALITY_OPERATORS(WaveformColor)


using WaveformColorGroup =
    pex::Group
    <
        WaveformColorSchema,
        pex::PlainT<WaveformColor>
    >;

using WaveformColorControl = typename WaveformColorGroup::DefaultControl;


template<typename T>
struct WaveformFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::maximumValue, "maximumValue"),
        fields::Field(&T::levelCount, "levelCount"),
        fields::Field(&T::columnCount, "columnCount"),
        fields::Field(&T::verticalScale, "verticalScale"),
        fields::Field(&T::color, "color"));
};


template<template<typename> typename T>
struct WaveformSchema
{
    T<bool> enable;
    T<size_t> maximumValue;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<1024>>> levelCount;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<1920>>> columnCount;
    T<pex::MakeRange<double, pex::Limit<1>, pex::Limit<10>>> verticalScale;
    T<WaveformColorGroup> color;

    static constexpr auto fields = WaveformFields<WaveformSchema>::fields;
    static constexpr auto fieldsTypeName = "Waveform";
};


struct WaveformSettings: public WaveformSchema<pex::Identity>
{
    static constexpr size_t defaultMaximumValue = 255;
    static constexpr size_t defaultLevelCount = 256;
    static constexpr size_t defaultColumnCount = 400;
    static constexpr double defaultVerticalZoom = 1.0;

    WaveformSettings()
        :
        WaveformSchema<pex::Identity>{
            true,
            defaultMaximumValue,
            defaultLevelCount,
            defaultColumnCount,
            defaultVerticalZoom,
            WaveformColor{}}
    {

    }
};


DECLARE_OUTPUT_STREAM_OPERATOR(WaveformSettings)
DECLARE_EQUALITY_OPERATORS(WaveformSettings)


using WaveformGroup =
    pex::Group<WaveformSchema, pex::PlainT<WaveformSettings>>;

using WaveformModel = typename WaveformGroup::Model;
using WaveformControl = typename WaveformGroup::DefaultControl;


} // end namespace draw


extern template struct pex::Group
    <
        draw::WaveformColorSchema,
        pex::PlainT<draw::WaveformColor>
    >;


extern template struct pex::Group
    <
        draw::WaveformSchema,
        pex::PlainT<draw::WaveformSettings>
    >;
