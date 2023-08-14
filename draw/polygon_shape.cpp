#include "draw/polygon_shape.h"


namespace draw
{


void DrawPolygon(
    wxpex::GraphicsContext &context,
    const PolygonPoints &points)
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


PolygonShape::PolygonShape(const Polygon &polygon_, const Look &look_)
    :
    PolygonShapeTemplate<pex::Identity>({polygon_, look_})
{

}

void PolygonShape::Draw(wxpex::GraphicsContext &context)
{
    if (this->polygon.points.empty())
    {
        return;
    }

    ConfigureLook(context, this->look);
    DrawPolygon(context, this->polygon.GetPoints());
}


} // end namespace draw
