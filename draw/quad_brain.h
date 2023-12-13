#pragma once


#include <tau/vector2d.h>
#include "draw/quad.h"
#include "draw/quad_shape.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/polygon.h"
#include "draw/shape_edit.h"


namespace draw
{


struct Bounds2d
{
    pex::Bounds<double> x;
    pex::Bounds<double> y;

    template<typename Control>
    static Bounds2d Make(Control control)
    {
        return {control.x.GetBounds(), control.y.GetBounds()};
    }
};


struct QuadBounds
{
    Bounds2d shear;
    Bounds2d perspective;
    pex::Bounds<double> rotation;

    QuadBounds(QuadControl control);
};


using DragQuad = DragShape<QuadControl>;
using DragEditQuad = DragEditShape<QuadControl>;


struct CreateQuad
{
    std::optional<QuadShape> operator()(
        const Drag &drag,
        const tau::Point2d<int> position);
};


using DragCreateQuad = DragCreateShape<QuadListControl, CreateQuad>;


class DragQuadPoint: public DragEditPoint<QuadControl>
{
public:
    using DragEditPoint<QuadControl>::DragEditPoint;

protected:
    Quad MakeShape_(const tau::Point2d<int> &end) const override;
};


class DragRotateQuadPoint: public DragEditPoint<QuadControl>
{
public:
    using DragEditPoint<QuadControl>::DragEditPoint;

protected:
    Quad MakeShape_(const tau::Point2d<int> &end) const override;
};


class DragQuadLine: public DragEditQuad
{
public:
    DragQuadLine(
        size_t index,
        const tau::Point2d<int> &start,
        QuadControl quad,
        const QuadLines &lines);

protected:
    Quad MakeShape_(const tau::Point2d<int> &end) const override;

private:
    QuadLines lines_;
};


using QuadBrain = ShapeBrain
<
    QuadListControl,
    DragCreateQuad,
    DragRotateQuadPoint,
    DragQuadPoint,
    DragQuadLine,
    DragQuad
>;


} // end namespace draw
