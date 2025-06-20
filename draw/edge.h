#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <tau/vector2d.h>
#include "draw/points.h"


namespace draw
{


template<typename T>
struct EdgeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::start, "start"),
        fields::Field(&T::end, "end"));
};


template<template<typename> typename T>
struct EdgeTemplate
{
    T<tau::Point2dGroup<double>> start;
    T<tau::Point2dGroup<double>> end;

    static constexpr auto fields = EdgeFields<EdgeTemplate>::fields;
    static constexpr auto fieldsTypeName = "Edge";
};


struct EdgeCustom
{
    template<typename Base>
    struct Plain: public Base
    {
        Plain()
            :
            Base{
                {0.0, 0.0},
                {1.0, 0.0}}
        {

        }

        Plain(
            const tau::Point2d<double> &start,
            const tau::Point2d<double> &end)
            :
            Base{start, end}
        {

        }

        PointsDouble Interpolate(size_t count)
        {
            auto direction =
                tau::Vector2d<double>(this->start, this->end).Normalize();

            auto size = this->start.Distance(this->end);
            auto stepSize = size / (static_cast<double>(count - 1));

            PointsDouble result;
            result.reserve(count);

            for (size_t i = 0; i < count; ++i)
            {
                result.emplace_back(
                    this->start
                    + (direction * stepSize * static_cast<double>(i)));
            }

            if (result.back().Distance(this->end) > 0.1)
            {
                throw std::runtime_error("Whoops");
            }

            return result;
        }
    };
};



using EdgeGroup = pex::Group<EdgeFields, EdgeTemplate, EdgeCustom>;
using EdgeControl = typename EdgeGroup::Control;
using Edge = typename EdgeGroup::Plain;


DECLARE_OUTPUT_STREAM_OPERATOR(Edge)
DECLARE_EQUALITY_OPERATORS(Edge)



} // end namespace draw



extern template struct pex::Group
    <
        draw::EdgeFields,
        draw::EdgeTemplate,
        draw::EdgeCustom
    >;
