#include "draw/polygon_brain.h"


namespace draw
{


std::optional<PolygonShape> CreatePolygon::operator()(
    const Drag &drag,
    const tau::Point2d<int> position)
{
    auto size = drag.GetSize(position);

    if (size.GetArea() < 1)
    {
        return {};
    }

    auto lines = PolygonLines(size);
    auto polygon = Polygon(lines.GetPoints());
    polygon.center = drag.GetDragCenter(position);

    return PolygonShape{polygon, Look::Default()};
}


Polygon DragPolygonPoint::MakeShape_(const tau::Point2d<int> &end) const
{
    double magnitude = this->GetMagnitude(end);

    if (magnitude < 1.0)
    {
        return this->control_.Get();
    }

    auto points = this->points_;
    points[this->index_] = this->GetPosition(end);

    return Polygon(points);
}


Polygon DragRotatePolygonPoint::MakeShape_(
    const tau::Point2d<int> &end) const
{
    auto adjusted = this->startingShape_;
    double magnitude = this->GetMagnitude(end);

    if (magnitude < 1.0)
    {
        return adjusted;
    }

    const auto &point = this->points_[this->index_];

    auto center = adjusted.center;
    auto baseline = (center - point).GetAngle();
    auto change = (center - end).GetAngle();
    auto difference = DragAngleDifference(change, baseline);
    adjusted.rotation += difference;
    adjusted.rotation = std::fmod(adjusted.rotation + 180, 360.0) - 180.0;

    return adjusted;
}


DragPolygonLine::DragPolygonLine(
    size_t index,
    const tau::Point2d<int> &start,
    PolygonControl polygon,
    const PolygonLines &lines)
    :
    DragEditPolygon(index, start, start.template Convert<double>(), polygon),
    lines_(lines)
{
    this->SetOffset(this->lines_[index].point);
}


Polygon DragPolygonLine::MakeShape_(const tau::Point2d<int> &end) const
{
    auto lines = this->lines_;
    auto &line = lines[this->index_];
    double magnitude = this->GetMagnitude(end);

    if (magnitude < 1.0)
    {
        return this->control_.Get();
    }

    line.point = this->GetPosition(end);

    return Polygon(lines.GetPoints());
}


} // end namespace draw
