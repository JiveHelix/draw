#include "draw/polygon_brain.h"


namespace draw
{


std::optional<size_t> GetPoint(
    const tau::Point2d<int> &click,
    const std::vector<tau::Point2d<double>> &points)
{
    for (size_t index = 0; index < points.size(); ++index)
    {
        auto &point = points[index];

        if (click.Distance(point) < 10.0)
        {
            return index;
        }
    }

    return {};
}


std::optional<size_t> GetLine(
    const tau::Point2d<int> &click,
    const std::vector<tau::Point2d<double>> &points)
{
    auto lines = PolygonLines(points);

    for (size_t index = 0; index < lines.lines.size(); ++index)
    {
        auto &line = lines[index];

        if (line.DistanceToPoint(click) < 10.0)
        {
            return index;
        }
    }

    return {};
}


DragPolygon::DragPolygon(
    size_t index,
    const tau::Point2d<int> &start,
    const tau::Point2d<double> &offset,
    const Polygon &polygon)
    :
    Drag(index, start, offset),
    polygon_(polygon)
{
    if (index >= polygon.GetPoints().size())
    {
        throw std::out_of_range("Selected indexed exceeds point count.");
    }
}


DragPolygonPoint::DragPolygonPoint(
    size_t index,
    const tau::Point2d<int> &start,
    const tau::Point2d<double> &offset,
    const Polygon &polygon)
    :
    DragPolygon(index, start, offset, polygon),
    points_(polygon.GetPoints())
{
    if (index >= this->points_.size())
    {
        throw std::out_of_range("Selected indexed exceeds point count.");
    }
}


DragRotatePolygonPoint::DragRotatePolygonPoint(
    size_t index,
    const tau::Point2d<int> &start,
    const tau::Point2d<double> &offset,
    const Polygon &polygon)
    :
    DragPolygon(index, start, offset, polygon),
    points_(polygon.GetPoints())
{
    if (index >= this->points_.size())
    {
        throw std::out_of_range("Selected indexed exceeds point count.");
    }
}


double DragAngleDifference(double first, double second)
{
    if (first < -90 || second < -90)
    {
        first = std::fmod(first + 360, 360);
        second = std::fmod(second + 360, 360);
    }

    return first - second;
}


Polygon DragPolygonPoint::GetPolygon(const tau::Point2d<int> &end) const
{
    double magnitude = this->GetMagnitude(end);
    
    if (magnitude < 1.0)
    {
        return this->polygon_;
    }

    auto points = this->points_;
    points[this->index_] = this->GetPosition(end);

    return Polygon(points);
}


Polygon DragRotatePolygonPoint::GetPolygon(
    const tau::Point2d<int> &end) const
{
    auto adjusted = this->polygon_;
    double magnitude = this->GetMagnitude(end);
    
    if (magnitude < 1.0)
    {
        return adjusted;
    }

    const auto &point = this->points_[this->index_];

    auto center = this->polygon_.center;
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
    const Polygon &polygon)
    :
    DragPolygon(index, start, start.template Convert<double>(), polygon),
    lines_(PolygonLines(this->polygon_.GetPoints()))
{
    this->SetOffset(this->lines_[index].point);
}


Polygon DragPolygonLine::GetPolygon(const tau::Point2d<int> &end) const
{
    auto lines = this->lines_;
    auto &line = lines[this->index_];
    double magnitude = this->GetMagnitude(end);

    if (magnitude < 1.0)
    {
        return this->polygon_;
    }

    line.point = this->GetPosition(end);

    return Polygon(lines.GetPoints());
}


PolygonBrain::PolygonBrain(
    PolygonControl polygonControl,
    PixelViewControl pixelViewControl)
    :
    polygonControl_(polygonControl),
    pixelViewControl_(pixelViewControl),
    mouseDownEndpoint_(
        this,
        pixelViewControl.mouseDown,
        &PolygonBrain::OnMouseDown_),
    logicalPositionEndpoint_(
        this,
        pixelViewControl.logicalPosition,
        &PolygonBrain::OnLogicalPosition_),
    modifierEndpoint_(
        this,
        pixelViewControl.modifier,
        &PolygonBrain::OnModifier_),
    dragCenter_(),
    dragPoint_(),
    dragLine_(),
    dragCreate_()
{

}


void PolygonBrain::OnMouseDown_(bool isDown)
{
    if (!isDown)
    {
        // Mouse has been released.
        this->dragCenter_.reset();
        this->dragLine_.reset();
        this->dragPoint_.reset();
        this->dragRotatePoint_.reset();
        this->dragCreate_.reset();
        this->UpdateCursor_();
        return;
    }

    auto click = this->pixelViewControl_.logicalPosition.Get();
    auto polygon = this->polygonControl_.Get();
    auto points = polygon.GetPoints();

    if (this->pixelViewControl_.modifier.Get().IsAlt())
    {
        // Subtract a point
        auto found = std::find_if(
            std::begin(points),
            std::end(points),
            [&click](const auto &p) -> bool
            {
                return click.Distance(p) < 10.0;
            });

        if (found != std::end(points))
        {
            points.erase(found);
        }

        this->polygonControl_.Set(Polygon(points));

        return;
    }

    auto pointIndex = GetPoint(click, points);

    if (pointIndex)
    {
        if (this->pixelViewControl_.modifier.Get().IsControl())
        {
            this->dragRotatePoint_ =
                DragRotatePolygonPoint(
                    *pointIndex,
                    click,
                    points[*pointIndex],
                    polygon);
        }
        else
        {
            this->dragPoint_ =
                DragPolygonPoint(
                    *pointIndex,
                    click,
                    points[*pointIndex],
                    polygon);
        }

        return;
    }

    auto lineIndex = GetLine(click, points);

    if (lineIndex)
    {
        this->dragLine_ = DragPolygonLine(*lineIndex, click, polygon);

        return;
    }

    if (polygon.Contains(click.template Convert<double>()))
    {
        this->dragCenter_ = Drag(click, polygon.center);

        this->pixelViewControl_.cursor.Set(
            wxpex::Cursor::closedHand);

        return;
    }

    if (this->pixelViewControl_.modifier.Get().IsControl())
    {
        // Add a point
        auto polygonPoints =
            this->polygonControl_.Get().GetPoints();

        polygonPoints.push_back(click.Convert<double>());
        this->polygonControl_.Set(Polygon(polygonPoints));

        return;
    }

    this->dragCreate_ = Drag(click, click);
}


void PolygonBrain::OnModifier_(const wxpex::Modifier &)
{
    this->UpdateCursor_();
}


void PolygonBrain::OnLogicalPosition_(const tau::Point2d<int> &position)
{
    this->UpdateCursor_();

    if (this->dragCenter_)
    {
        this->polygonControl_.center.Set(
            this->dragCenter_->GetPosition(position));

        return;
    }

    if (this->dragLine_)
    {
        this->polygonControl_.Set(this->dragLine_->GetPolygon(position));

        return;
    }

    if (this->dragPoint_)
    {
        this->polygonControl_.Set(this->dragPoint_->GetPolygon(position));

        return;
    }

    if (this->dragRotatePoint_)
    {
        this->polygonControl_.Set(this->dragRotatePoint_->GetPolygon(position));

        return;
    }

    if (this->dragCreate_)
    {
        auto size = this->dragCreate_->GetSize(position);
        
        if (size.GetArea() < 1)
        {
            return;
        }

        auto lines = PolygonLines(this->dragCreate_->GetSize(position));
        auto polygon = Polygon(lines.GetPoints());
        polygon.center = this->dragCreate_->GetDragCenter(position);
        this->polygonControl_.Set(polygon);
        
        return;
    }
}


void PolygonBrain::UpdateCursor_()
{
    if (this->IsDragging_())
    {
        return;
    }

    auto modifier = this->pixelViewControl_.modifier.Get();
    auto polygon = this->polygonControl_.Get();

    if (polygon.points.size() < 3)
    {
        return;
    }

    auto click = this->pixelViewControl_.logicalPosition.Get();
    auto points = polygon.GetPoints();
    auto pointIndex = GetPoint(click, points);

    if (pointIndex)
    {
        // Hovering over a point, change cursor
        if (modifier.IsControl())
        {
            // TODO: Create a rotate cursor
            this->pixelViewControl_.cursor.Set(wxpex::Cursor::pointRight);
        }
        else if (modifier.IsAlt())
        {
            this->pixelViewControl_.cursor.Set(wxpex::Cursor::bullseye);
        }
        else
        {
            this->pixelViewControl_.cursor.Set(wxpex::Cursor::cross);
        }

        return;
    }

    auto lineIndex = GetLine(click, points);

    if (lineIndex)
    {
        // Hovering over a line, change cursor
        this->pixelViewControl_.cursor.Set(wxpex::Cursor::sizing);
        return;
    }

    if (polygon.Contains(click.template Convert<double>()))
    {
        this->pixelViewControl_.cursor.Set(wxpex::Cursor::openHand);

        return;
    }

    if (modifier.IsControl())
    {
        this->pixelViewControl_.cursor.Set(wxpex::Cursor::pencil);
        return;
    }

    if (this->pixelViewControl_.cursor.Get() != wxpex::Cursor::arrow)
    {
        this->pixelViewControl_.cursor.Set(wxpex::Cursor::arrow);
    }
}


bool PolygonBrain::IsDragging_() const
{
    return (
        this->dragCenter_
        || this->dragPoint_
        || this->dragRotatePoint_
        || this->dragLine_
        || this->dragCreate_);
}


} // end namespace draw
