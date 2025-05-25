#include "draw/segments_shape.h"
#include "draw/draw_spline.h"


namespace draw
{


SegmentsShape::SegmentsShape(
    const SegmentsSettings &settings,
    const PointsDouble &points)
    :
    segmentsSettings_(settings),
    points_(points)
{
    if (points.size() < 2)
    {
        throw std::logic_error("Segments requires at least 2 points");
    }
}


void SegmentsShape::Draw(DrawContext &context)
{
    if (this->points_.empty())
    {
        return;
    }

    context.ConfigureLook(this->segmentsSettings_.look);
    auto path = context->CreatePath();

    if (this->segmentsSettings_.isSpline)
    {
        DrawSpline(path, this->points_);
    }
    else
    {
        auto point = std::begin(this->points_);
        auto end = std::end(this->points_);

        path.MoveToPoint(point->x, point->y);

        while (++point != end)
        {
            path.AddLineToPoint(point->x, point->y);
        }
    }

    context->DrawPath(path);
}


} // end namespace draw
