#pragma once


#include <tau/vector2d.h>
#include "draw/polygon.h"
#include "draw/polygon_shape.h"
#include "draw/drag.h"
#include "draw/shape_edit.h"


namespace draw
{


using DragPolygon = DragShape<PolygonControl>;
using DragEditPolygon = DragEditShape<PolygonControl>;


struct CreatePolygon
{
    std::optional<PolygonShape> operator()(
        const Drag &drag,
        const tau::Point2d<int> position);
};


using DragCreatePolygon = DragCreateShape<PolygonListControl, CreatePolygon>;


class DragPolygonPoint: public DragEditPoint<PolygonControl>
{
public:
    using DragEditPoint<PolygonControl>::DragEditPoint;

protected:
    Polygon MakeShape_(const tau::Point2d<int> &end) const override;
};


class DragRotatePolygonPoint: public DragEditPoint<PolygonControl>
{
public:
    using DragEditPoint<PolygonControl>::DragEditPoint;

protected:
    Polygon MakeShape_(const tau::Point2d<int> &end) const override;
};


class DragPolygonLine: public DragEditPolygon
{
public:
    DragPolygonLine(
        size_t index,
        const tau::Point2d<int> &start,
        PolygonControl polygon,
        const PolygonLines &lines);

protected:
    Polygon MakeShape_(const tau::Point2d<int> &end) const override;

private:
    PolygonLines lines_;
};


using PolygonBrain = ShapeBrain
<
    PolygonListControl,
    DragCreatePolygon
>;


} // end namespace draw
