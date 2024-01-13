#include <fmt/core.h>
#include "draw/polygon_shape.h"
#include "draw/polygon_brain.h"
#include "draw/views/polygon_view.h"
#include "draw/views/look_view.h"


namespace draw
{


void DrawPolygon(
    wxpex::GraphicsContext &context,
    const Points &points)
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
