#include "draw/polygon_shape.h"


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


PolygonShape::PolygonShape(
    size_t id_,
    const Polygon &polygon_,
    const Look &look_)
    :
    PolygonShapeTemplate<pex::Identity>({id_, polygon_, look_})
{

}


PolygonShape::PolygonShape(const Polygon &polygon_, const Look &look_)
    :
    PolygonShapeTemplate<pex::Identity>({0, polygon_, look_})
{

}


void PolygonShape::Draw(wxpex::GraphicsContext &context)
{
    auto points = this->shape.GetPoints();

    if (points.empty())
    {
        return;
    }

    ConfigureLook(context, this->look);
    DrawPolygon(context, points);
}


PolygonShapeModel::PolygonShapeModel()
    :
    PolygonShapeGroup::Model()
{
    static size_t nextId = 0;
    this->id.Set(nextId++);
}


void PolygonShapeModel::Set(const PolygonShape &other)
{
    // Do not change the id.
    this->shape.Set(other.shape);
    this->look.Set(other.look);
}


} // end namespace draw
