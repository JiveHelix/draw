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


PolygonShape::PolygonShape(
    size_t id_,
    const Polygon &polygon_,
    const Look &look_)
    :
    PolygonShapeTemplate<pex::Identity>({id_, polygon_, look_, {}})
{

}


PolygonShape::PolygonShape(const Polygon &polygon_, const Look &look_)
    :
    PolygonShapeTemplate<pex::Identity>({0, polygon_, look_, {}})
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
    PolygonShapeGroup::Model(),
    polyShapeId_()
{
    this->id.Set(this->polyShapeId_.Get());
}


void PolygonShapeModel::Set(const PolygonShape &other)
{
    // Do not change the id.
    this->shape.Set(other.shape);
    this->look.Set(other.look);
}


bool PolygonShapeControl::ProcessControlClick(const tau::Point2d<int> &click)
{
    auto points = this->shape.Get().GetPoints();
    points.push_back(click.Convert<double>());
    this->shape.Set(Polygon(points));

    return true;
}

bool PolygonShapeControl::ProcessAltClick(
    PointsIterator foundPoint,
    Points &points)
{
    // Subtract a point
    points.erase(foundPoint);
    this->shape.Set(Polygon(points));

    return true;
}


std::unique_ptr<Drag> PolygonShapeControl::ProcessMouseDown(
    const tau::Point2d<int> &click,
    const wxpex::Modifier &modifier,
    CursorControl cursor)
{
    return ::draw::ProcessMouseDown
        <
            DragRotatePolygonPoint,
            DragPolygonPoint,
            DragPolygonLine,
            DragPolygon
        >(*this, click, modifier, cursor);
}


std::string PolygonShapeControl::GetName() const
{
    return fmt::format("Polygon {}", this->id.Get());
}


wxWindow * PolygonShapeControl::CreateShapeView(wxWindow *parent) const
{
    return new PolygonView(
        parent,
        "Polygon",
        this->shape,
        wxpex::LayoutOptions{});
}


wxWindow * PolygonShapeControl::CreateLookView(wxWindow *parent) const
{
    return new LookView(parent, "Look", this->look, wxpex::LayoutOptions{});
}


} // end namespace draw
