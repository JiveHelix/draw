#pragma once


#include <pex/group.h>
#include <tau/vector2d.h>
#include "draw/point.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/oddeven.h"


namespace draw
{


template<template<typename> typename T>
struct PointsShapeSchema
{
    T<pex::MakeRange<double, pex::Limit<0>, pex::Limit<20>>> radius;
    T<LookGroup> look;

    static constexpr auto fieldsTypeName = "PointsShape";
};


struct PointsShapeSettings
    :
    public PointsShapeSchema<pex::Identity>
{
    PointsShapeSettings();
};


DECLARE_EQUALITY_OPERATORS(PointsShapeSettings)


class PointsShape: public DrawnShape
{
public:
    PointsShape(
        const PointsShapeSettings &settings,
        const PointsDouble &points);

    void Draw(DrawContext &context) override;

    PointsShapeSettings settings_;
    PointsDouble points_;
};


using PointsShapeGroup = pex::Group
<
    PointsShapeSchema,
    pex::PlainT<PointsShapeSettings>
>;

using PointsShapeModel = typename PointsShapeGroup::Model;

using PointsShapeControl =
    typename PointsShapeGroup::DefaultControl;


class ValuePointsShape: public DrawnShape
{
public:
    using ValuePoints = std::vector<ValuePoint<double>>;

    ValuePointsShape(
        const PointsShapeSettings &settings,
        const ValuePoints &points);

    void Draw(DrawContext &context) override;

    PointsShapeSettings settings_;
    ValuePoints points_;
};


} // end namespace draw



extern template struct pex::Group
<
    draw::PointsShapeSchema,
    pex::PlainT<draw::PointsShapeSettings>
>;
