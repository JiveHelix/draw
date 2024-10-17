#include "draw/draw_segments.h"


namespace draw
{


void AddSegmentsToPoint(
    wxGraphicsPath &path,
    const PointsDouble &points)
{
    for (const auto &point: points)
    {
        path.AddLineToPoint(point.x, point.y);
    }
}


void DrawSegments(
    DrawContext &context,
    const PointsDouble &points)
{
    auto path = context->CreatePath();
    AddSegmentsToPoint(path, points);
    path.CloseSubpath();
    context->DrawPath(path);
}


} // end namespace draw
