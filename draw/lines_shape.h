#pragma once


#include <pex/group.h>
#include <tau/line2d.h>
#include "draw/look.h"
#include "draw/shapes.h"


namespace draw
{


template<template<typename> typename T>
struct LinesShapeSchema
{
    T<bool> infinite;
    T<pex::MakeRange<double, pex::Limit<0>, pex::Limit<2000>>> length;
    T<LookGroup> look;

    static constexpr auto fieldsTypeName = "LinesShape";
};


struct LinesShapeSettings: public LinesShapeSchema<pex::Identity>
{
    LinesShapeSettings();
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
        const Lines &lines);

    void Draw(DrawContext &context) override;

    LinesShapeSettings settings_;
    Lines lines_;
};


using LinesShapeGroup = pex::Group
<
    LinesShapeSchema,
    pex::PlainT<LinesShapeSettings>
>;

using LinesShapeModel = typename LinesShapeGroup::Model;
using LinesShapeControl = typename LinesShapeGroup::DefaultControl;


} // end namespace draw


extern template struct pex::Group
<
    draw::LinesShapeSchema,
    pex::PlainT<draw::LinesShapeSettings>
>;
