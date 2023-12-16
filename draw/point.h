#pragma once


#include <tau/vector2d.h>
#include <pex/interface.h>
#include <wxpex/point.h>


namespace draw
{


using PointGroup = tau::Point2dGroup<int>;
using Point = typename PointGroup::Plain;
using PointControl = typename PointGroup::Control;


template<typename T>
struct ValuePoint: public tau::Point2d<T>
{
    ValuePoint()
        :
        tau::Point2d<T>(),
        value()
    {

    }

    ValuePoint(T column, T row, T value_)
        :
        tau::Point2d<T>(column, row),
        value(value_)
    {

    }

    T value;

    static constexpr auto fields = std::tuple_cat(
        tau::Point2d<T>::fields,
        std::make_tuple(
            fields::Field(&ValuePoint::value, "value")));
};


TEMPLATE_OUTPUT_STREAM(ValuePoint)


} // end namespace draw
