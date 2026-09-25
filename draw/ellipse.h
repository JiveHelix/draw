#pragma once

#include <fields/fields.h>
#include "draw/draw_context.h"
#include <pex/group.h>
#include <pex/range.h>
#include <tau/vector2d.h>
#include "draw/scale.h"
#include "draw/points.h"


namespace draw
{


template<template<typename> typename T>
struct EllipseSchema
{
    using AxisRange = pex::MakeRange<double, pex::Limit<0>, pex::Limit<1000>>;

    using AngleRange =
        pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>;

    T<tau::Point2dGroup<double>> center;
    T<AxisRange> major;
    T<AxisRange> minor;
    T<AngleRange> rotation;
    T<ScaleRange> scale;

    static constexpr auto fieldsTypeName = "Ellipse";
};


struct Ellipse: public EllipseSchema<pex::Identity>
{
    using Point = tau::Point2d<double>;

    Ellipse();
    bool Contains(const Point &point) const;
    bool Contains(const Point &point, double margin) const;
    PointsDouble GetPoints() const;
    void EditPoint(const Point &point, size_t index);
    void Draw(DrawContext &context);
};


struct EllipseFinisher
{
    using Plain = Ellipse;
};


using EllipseGroup = pex::Group
<
    EllipseSchema,
    EllipseFinisher
>;

using EllipseModel = typename EllipseGroup::Model;

using EllipseControl = typename EllipseGroup::DefaultControl;



DECLARE_OUTPUT_STREAM_OPERATOR(Ellipse)


} // end namespace draw


extern template struct pex::Group
<
    draw::EllipseSchema,
    draw::EllipseFinisher
>;
