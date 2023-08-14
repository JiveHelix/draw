#pragma once


#include <string>
#include <fields/fields.h>
#include <pex/linked_ranges.h>
#include <wxpex/color_picker.h>
#include "draw/size.h"


namespace draw
{


using LowBrightness = pex::Limit<0, 4, 10>;
using HighBrightness = pex::Limit<0, 7, 10>;

using BrightnessRanges =
    pex::LinkedRanges
    <
        double,
        pex::Limit<0>,
        pex::Limit<0, 4, 10>,
        pex::Limit<1>,
        pex::Limit<0, 7, 10>
    >;


template<typename T>
struct WaveformColorFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::range, "range"),
        fields::Field(&T::count, "count"),
        fields::Field(&T::color, "color"),
        fields::Field(&T::highlightColor, "highlightColor"));
};


template<template<typename> typename T>
struct WaveformColorTemplate
{
    T<BrightnessRanges::GroupMaker> range;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<256>>> count;
    T<pex::MakeGroup<wxpex::HsvGroup>> color;
    T<pex::MakeGroup<wxpex::HsvGroup>> highlightColor;

    static constexpr auto fields =
        WaveformColorFields<WaveformColorTemplate>::fields;

    static constexpr auto fieldsTypeName = "WaveformColor";
};


struct WaveformColor: public WaveformColorTemplate<pex::Identity>
{
    static constexpr size_t defaultCount = 20;

    static constexpr auto defaultHue = 138.0; // green
    static constexpr auto defaultHighlightHue = 292.0; // purple

    static WaveformColor Default()
    {
        return {{
            BrightnessRanges::Settings::Default(),
            defaultCount,
            {{defaultHue, 1.0, 1.0}},
            {{defaultHighlightHue, 1.0, 1.0}}}};
    }
};


DECLARE_OUTPUT_STREAM_OPERATOR(WaveformColor)
DECLARE_EQUALITY_OPERATORS(WaveformColor)


using WaveformColorGroup =
    pex::Group<WaveformColorFields, WaveformColorTemplate, WaveformColor>;

using WaveformColorControl = typename WaveformColorGroup::Control;


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
struct WaveformTemplate
{
    T<bool> enable;
    T<size_t> maximumValue;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<1024>>> levelCount;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<1920>>> columnCount;
    T<pex::MakeRange<double, pex::Limit<1>, pex::Limit<10>>> verticalScale;
    T<pex::MakeGroup<WaveformColorGroup>> color;

    static constexpr auto fields = WaveformFields<WaveformTemplate>::fields;
    static constexpr auto fieldsTypeName = "Waveform";
};


struct WaveformSettings: public WaveformTemplate<pex::Identity>
{
    static constexpr size_t defaultMaximumValue = 255;
    static constexpr size_t defaultLevelCount = 200;
    static constexpr size_t defaultColumnCount = 400;
    static constexpr double defaultVerticalZoom = 1.0;

    static WaveformSettings Default()
    {
        return {{
            true,
            defaultMaximumValue,
            defaultLevelCount,
            defaultColumnCount,
            defaultVerticalZoom,
            WaveformColor::Default()}};
    }
};


DECLARE_OUTPUT_STREAM_OPERATOR(WaveformSettings)
DECLARE_EQUALITY_OPERATORS(WaveformSettings)


using WaveformGroup =
    pex::Group<WaveformFields, WaveformTemplate, WaveformSettings>;

using WaveformModel = typename WaveformGroup::Model;
using WaveformControl = typename WaveformGroup::Control;

using WaveformGroupMaker = pex::MakeGroup<WaveformGroup>;


} // end namespace draw


extern template struct pex::Group
    <
        draw::WaveformColorFields,
        draw::WaveformColorTemplate,
        draw::WaveformColor
    >;


extern template struct pex::Group
    <
        draw::WaveformFields,
        draw::WaveformTemplate,
        draw::WaveformSettings
    >;
