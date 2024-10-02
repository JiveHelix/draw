#include "draw/draw_segments.h"


namespace draw
{


void DrawSegments(
    DrawContext &context,
    const PointsDouble &points)
{
    auto path = context->CreatePath();
    auto point = std::begin(points);
    path.MoveToPoint(point->x, point->y);

    while (++point != std::end(points))
    {
        path.AddLineToPoint(point->x, point->y);
    }

    path.CloseSubpath();
    context->DrawPath(path);
}


} // end namespace draw
