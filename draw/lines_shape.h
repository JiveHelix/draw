#pragma once


#include <pex/group.h>
#include <tau/line2d.h>
#include "draw/look.h"
#include "draw/shapes.h"


namespace draw
{

template<typename T>
struct LinesShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::infinite, "infinite"),
        fields::Field(&T::length, "length"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
struct LinesShapeTemplate
{
    T<bool> infinite;
    T<pex::MakeRange<double, pex::Limit<0>, pex::Limit<2000>>> length;
    T<draw::LookGroup> look;

    static constexpr auto fields =
        LinesShapeFields<LinesShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "LinesShape";
};


struct LinesShapeSettings: public LinesShapeTemplate<pex::Identity>
{
    static LinesShapeSettings Default();
};


DECLARE_EQUALITY_OPERATORS(LinesShapeSettings)


class LinesShape
    :
    public DrawnShape
{
public:
    using Line = tau::Line2d<double>;
    using Lines = std::vector<Line>;

    LinesShape() = default;

    LinesShape(
        const LinesShapeSettings &settings,
        const Lines &points);

    void Draw(wxpex::GraphicsContext &context) override;

    LinesShapeSettings settings_;
    Lines lines_;
};


using LinesShapeGroup = pex::Group
<
    LinesShapeFields,
    LinesShapeTemplate,
    pex::PlainT<LinesShapeSettings>
>;

using LinesShapeModel = typename LinesShapeGroup::Model;
using LinesShapeControl = typename LinesShapeGroup::Control;


} // end namespace draw


extern template struct pex::Group
<
    draw::LinesShapeFields,
    draw::LinesShapeTemplate,
    pex::PlainT<draw::LinesShapeSettings>
>;
