#include "draw/segments_shape.h"


namespace draw
{


SegmentsShape::SegmentsShape(
    const Look &look,
    const PointsDouble &points)
    :
    look_(look),
    points_(points)
{

}


void SegmentsShape::Draw(DrawContext &context)
{
    if (this->points_.empty())
    {
        return;
    }

    context.ConfigureLook(this->look_);
    auto path = context->CreatePath();

    auto point = std::begin(this->points_);
    auto end = std::end(this->points_);

    path.MoveToPoint(point->x, point->y);

    while (++point != end)
    {
        path.AddLineToPoint(point->x, point->y);
    }

    context->DrawPath(path);
}


} // end namespace draw
