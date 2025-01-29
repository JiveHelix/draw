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


template<typename T>
struct EllipseFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::center, "center"),
        fields::Field(&T::major, "major"),
        fields::Field(&T::minor, "minor"),
        fields::Field(&T::rotation, "rotation"),
        fields::Field(&T::scale, "scale"));
};


template<template<typename> typename T>
struct EllipseTemplate
{
    using AxisRange = pex::MakeRange<double, pex::Limit<0>, pex::Limit<1000>>;

    using AngleRange =
        pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>;

    T<tau::Point2dGroup<double>> center;
    T<AxisRange> major;
    T<AxisRange> minor;
    T<AngleRange> rotation;
    T<ScaleRange> scale;

    static constexpr auto fields = EllipseFields<EllipseTemplate>::fields;
    static constexpr auto fieldsTypeName = "Ellipse";
};


struct Ellipse: public EllipseTemplate<pex::Identity>
{
    using Point = tau::Point2d<double>;

    Ellipse();
    bool Contains(const Point &point) const;
    bool Contains(const Point &point, double margin) const;
    PointsDouble GetPoints() const;
    void EditPoint(const Point &point, size_t index);
    void Draw(DrawContext &context);
};


using EllipseGroup = pex::Group
<
    EllipseFields,
    EllipseTemplate,
    pex::PlainT<Ellipse>
>;

using EllipseModel = typename EllipseGroup::Model;

using EllipseControl = typename EllipseGroup::Control;



DECLARE_OUTPUT_STREAM_OPERATOR(Ellipse)


} // end namespace draw


extern template struct pex::Group
<
    draw::EllipseFields,
    draw::EllipseTemplate,
    pex::PlainT<draw::Ellipse>
>;
