#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <tau/vector2d.h>
#include "draw/scale.h"
#include "draw/points.h"


namespace draw
{


template<typename T>
struct CrossFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::center, "center"),
        fields::Field(&T::size, "size"),
        fields::Field(&T::rotation, "rotation"));
};


using RotationRange =
    pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>;

using SizeRange =
    pex::MakeRange<double, pex::Limit<0>, pex::Limit<100>>;


template<template<typename> typename T>
struct CrossTemplate
{
    T<tau::Point2dGroup<double>> center;
    T<SizeRange> size;
    T<RotationRange> rotation;

    static constexpr auto fields = CrossFields<CrossTemplate>::fields;
    static constexpr auto fieldsTypeName = "Cross";
};


template<typename Base>
struct Cross_: public Base
{
    Cross_()
        :
        Base{
            {0.0, 0.0},
            25.0,
            0.0}
    {

    }

    PointsDouble GetPoints() const
    {
        return {this->center};
    }

    bool Contains(const tau::Point2d<double> &point, double margin) const
    {
        return point.Distance(this->center) < margin;
    }
};


struct CrossCustom
{
    template<typename Base>
    using Plain = Cross_<Base>;
};



using CrossGroup = pex::Group<CrossFields, CrossTemplate, CrossCustom>;
using CrossControl = typename CrossGroup::DefaultControl;
using Cross = typename CrossGroup::Plain;


DECLARE_OUTPUT_STREAM_OPERATOR(Cross)
DECLARE_EQUALITY_OPERATORS(Cross)



} // end namespace draw



extern template struct pex::Group
    <
        draw::CrossFields,
        draw::CrossTemplate,
        draw::CrossCustom
    >;
