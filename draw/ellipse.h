#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <tau/vector2d.h>
#include "draw/scale.h"


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
};


struct Ellipse: public EllipseTemplate<pex::Identity>
{
    static Ellipse Default();
    bool Contains(const tau::Point2d<double> &point) const;
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
