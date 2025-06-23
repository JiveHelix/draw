#pragma once


#include <fields/fields.h>
#include <fields/compare.h>
#include <pex/endpoint.h>
#include <pex/linked_ranges.h>


namespace draw
{


namespace colors
{


struct SignedGradient
{
    static constexpr auto name = "Signed Gradient";
};


struct Turbo
{
    static constexpr auto name = "Turbo";
};


struct Grayscale
{
    static constexpr auto name = "Gray";
};


} // end namespace colors


template<typename T>
struct ColorMapSettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::turbo, "turbo"),
        fields::Field(&T::range, "range"),
        fields::Field(&T::maximum, "maximum"));
};


using DefaultLowColor = pex::Limit<0>;
using DefaultHighColor = pex::Limit<255>;

template<typename Value>
using ColorRange =
    pex::LinkedRanges
    <
        Value,
        DefaultLowColor,
        DefaultLowColor,
        DefaultHighColor,
        DefaultHighColor
    >;


template<typename Value>
struct ColorMapSettingsTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> turbo;
        T<typename ColorRange<Value>::Group> range;
        T<Value> maximum;

        static constexpr auto fields = ColorMapSettingsFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Color";
    };
};


template<typename Value>
struct ColorMapSettings:
    public ColorMapSettingsTemplate<Value>::template Template<pex::Identity>
{
    ColorMapSettings()
        :
        ColorMapSettingsTemplate<Value>::template Template<pex::Identity>{
            true,
            typename ColorRange<Value>::Settings{},
            DefaultHighColor::Get<Value>()}
    {

    }
};


template<typename Value>
struct ColorMapSettingsCustom
{
    using Plain = ColorMapSettings<Value>;

    template<typename Base>
    struct Model: public Base
    {
    public:
        Model()
            :
            Base(),

            maximumEndpoint_(
                this,
                this->maximum,
                &Model::OnMaximum_)
        {

        }

    private:
        void OnMaximum_(Value maximum_)
        {
            this->range.SetMaximumValue(maximum_);
        }

    private:
        using MaximumEndpoint = pex::Endpoint<Model, decltype(Model::maximum)>;
        MaximumEndpoint maximumEndpoint_;
    };
};


TEMPLATE_EQUALITY_OPERATORS(ColorMapSettings)
TEMPLATE_OUTPUT_STREAM(ColorMapSettings)


template<typename Value>
using ColorMapSettingsGroup =
    pex::Group
    <
        ColorMapSettingsFields,
        ColorMapSettingsTemplate<Value>::template Template,
        ColorMapSettingsCustom<Value>
    >;

template<typename Value>
using ColorMapSettingsModel = typename ColorMapSettingsGroup<Value>::Model;

template<typename Value>
using ColorMapSettingsControl = typename ColorMapSettingsGroup<Value>::Control;


} // end namespace draw


extern template struct pex::Group
    <
        draw::ColorMapSettingsFields,
        draw::ColorMapSettingsTemplate<int32_t>::template Template,
        draw::ColorMapSettingsCustom<int32_t>
    >;
