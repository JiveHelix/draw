#pragma once


#include <pex/group.h>
#include <tau/vector2d.h>
#include "draw/points.h"
#include "draw/look.h"
#include "draw/shapes.h"


namespace draw
{


class SegmentsShape
    :
    public DrawnShape
{
public:
    SegmentsShape() = default;

    SegmentsShape(
        const Look &look,
        const PointsDouble &points);

    void Draw(wxpex::GraphicsContext &context) override;

private:
    Look look_;
    PointsDouble points_;
};


} // end namespace draw
