#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <tau/vector2d.h>
#include "draw/scale.h"


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
};


struct Cross: public CrossTemplate<pex::Identity>
{
    Cross()
        :
        CrossTemplate<pex::Identity>{
            {{0.0, 0.0}},
            25.0,
            0.0}
    {

    }

    static Cross Default()
    {
        return {};
    }
};



using CrossGroup = pex::Group<CrossFields, CrossTemplate, Cross>;
using CrossControl = typename CrossGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(Cross)
DECLARE_EQUALITY_OPERATORS(Cross)



} // end namespace draw



extern template struct pex::Group
    <
        draw::CrossFields,
        draw::CrossTemplate,
        draw::Cross
    >;
