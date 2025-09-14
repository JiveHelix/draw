#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <wxpex/color_picker.h>
#include <wxpex/graphics.h>


namespace draw
{



template<typename T>
struct StrokeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::weight, "weight"),
        fields::Field(&T::color, "color"),
        fields::Field(&T::penStyle, "penStyle"),
        fields::Field(&T::penCap, "penCap"),
        fields::Field(&T::penJoin, "penJoin"),
        fields::Field(&T::antialias, "antialias"));
};


template<template<typename> typename T>
struct StrokeTemplate
{
    using WeightRange =
        pex::MakeRange<double, pex::Limit<0, 1, 10>, pex::Limit<100>>;

    T<bool> enable;
    T<WeightRange> weight;
    T<wxpex::HsvaGroup> color;
    T<wxpex::PenStyleSelect> penStyle;
    T<wxpex::PenCapSelect> penCap;
    T<wxpex::PenJoinSelect> penJoin;
    T<bool> antialias;

    static constexpr auto fields = StrokeFields<StrokeTemplate>::fields;
    static constexpr auto fieldsTypeName = "Stroke";
};


struct Stroke: public StrokeTemplate<pex::Identity>
{
    Stroke()
        :
        StrokeTemplate<pex::Identity>{
            true,
            1.0,
            {{0.0, 0.0, 1.0, 1.0}},
            wxpex::PenStyle::solid,
            wxpex::PenCap::round,
            wxpex::PenJoin::round,
            true}
    {

    }

    wxGraphicsPenInfo GetPenInfo() const
    {
        return wxGraphicsPenInfo(
            wxpex::ToWxColour(this->color),
            this->weight,
            wxPenStyle(this->penStyle))
                .Cap(wxPenCap(this->penCap))
                .Join(wxPenJoin(this->penJoin));
    }
};



using StrokeGroup = pex::Group<StrokeFields, StrokeTemplate, pex::PlainT<Stroke>>;

using StrokeModel = typename StrokeGroup::Model;
using StrokeControl = typename StrokeGroup::DefaultControl;

DECLARE_EQUALITY_OPERATORS(Stroke)


template<typename T>
struct FillFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::color, "color"),
        fields::Field(&T::brushStyle, "brushStyle"));
};


template<template<typename> typename T>
struct FillTemplate
{
    T<bool> enable;
    T<wxpex::HsvaGroup> color;
    T<wxpex::BrushStyleSelect> brushStyle;

    static constexpr auto fields = FillFields<FillTemplate>::fields;
    static constexpr auto fieldsTypeName = "Fill";
};


struct Fill: public FillTemplate<pex::Identity>
{
    Fill()
        :
        FillTemplate<pex::Identity>{
            false,
            {{0.0, 0.0, 0.5, 1.0}},
            wxpex::BrushStyle::solid}
    {

    }
};



using FillGroup = pex::Group<FillFields, FillTemplate, pex::PlainT<Fill>>;

using FillModel = typename FillGroup::Model;
using FillControl = typename FillGroup::DefaultControl;

DECLARE_EQUALITY_OPERATORS(Fill)



template<typename T>
struct LookFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::stroke, "stroke"),
        fields::Field(&T::fill, "fill"));
};


template<template<typename> typename T>
struct LookTemplate
{
    T<StrokeGroup> stroke;
    T<FillGroup> fill;

    static constexpr auto fields = LookFields<LookTemplate>::fields;
    static constexpr auto fieldsTypeName = "Look";
};


struct Look: public LookTemplate<pex::Identity>
{
    Look()
        :
        LookTemplate<pex::Identity>{
            Stroke{},
            Fill{}}
    {

    }
};



using LookGroup = pex::Group<LookFields, LookTemplate, pex::PlainT<Look>>;

using LookModel = typename LookGroup::Model;
using LookControl = typename LookGroup::DefaultControl;

DECLARE_EQUALITY_OPERATORS(Look)


} // end namespace draw


extern template struct pex::Group
    <
        draw::LookFields,
        draw::LookTemplate,
        pex::PlainT<draw::Look>
    >;
