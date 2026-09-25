#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <wxpex/color_picker.h>
#include <wxpex/graphics.h>


namespace draw
{


template<template<typename> typename T>
struct StrokeSchema
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

    static constexpr auto fieldsTypeName = "Stroke";
};


struct Stroke: public StrokeSchema<pex::Identity>
{
    Stroke()
        :
        StrokeSchema<pex::Identity>{
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



using StrokeGroup = pex::Group<StrokeSchema, pex::PlainT<Stroke>>;

using StrokeModel = typename StrokeGroup::Model;
using StrokeControl = typename StrokeGroup::DefaultControl;

DECLARE_EQUALITY_OPERATORS(Stroke)


template<template<typename> typename T>
struct FillSchema
{
    T<bool> enable;
    T<wxpex::HsvaGroup> color;
    T<wxpex::BrushStyleSelect> brushStyle;

    static constexpr auto fieldsTypeName = "Fill";
};


struct Fill: public FillSchema<pex::Identity>
{
    Fill()
        :
        FillSchema<pex::Identity>{
            false,
            {{0.0, 0.0, 0.5, 1.0}},
            wxpex::BrushStyle::solid}
    {

    }
};



using FillGroup = pex::Group<FillSchema, pex::PlainT<Fill>>;

using FillModel = typename FillGroup::Model;
using FillControl = typename FillGroup::DefaultControl;

DECLARE_EQUALITY_OPERATORS(Fill)


template<template<typename> typename T>
struct LookSchema
{
    T<StrokeGroup> stroke;
    T<FillGroup> fill;

    static constexpr auto fieldsTypeName = "Look";
};


struct Look: public LookSchema<pex::Identity>
{
    Look()
        :
        LookSchema<pex::Identity>{
            Stroke{},
            Fill{}}
    {

    }
};



using LookGroup = pex::Group<LookSchema, pex::PlainT<Look>>;

using LookModel = typename LookGroup::Model;
using LookControl = typename LookGroup::DefaultControl;

DECLARE_EQUALITY_OPERATORS(Look)


} // end namespace draw


extern template struct pex::Group
    <
        draw::LookSchema,
        pex::PlainT<draw::Look>
    >;
