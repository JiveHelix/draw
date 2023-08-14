#include "draw/quad_brain.h"
#include "draw/polygon_brain.h"


namespace draw
{


QuadBounds::QuadBounds(QuadControl control)
    :
    shear(Bounds2d::Make(control.shear)),
    perspective(Bounds2d::Make(control.perspective)),
    rotation(control.rotation.GetBounds())
{

}


DragQuad::DragQuad(
    size_t index,
    const tau::Point2d<int> &start,
    const tau::Point2d<double> &offset,
    const Quad &quad,
    const QuadBounds &quadBounds)
    :
    Drag(index, start, offset),
    quad_(quad),
    quadBounds_(quadBounds)
{

}


DragQuadPoint::DragQuadPoint(
    size_t index,
    const tau::Point2d<int> &start,
    const tau::Point2d<double> &offset,
    const Quad &quad,
    const QuadBounds &quadBounds)
    :
    DragQuad(index, start, offset, quad, quadBounds),
    points_(quad.GetPoints())
{

}


DragRotatePoint::DragRotatePoint(
    size_t index,
    const tau::Point2d<int> &start,
    const tau::Point2d<double> &offset,
    const Quad &quad,
    const QuadBounds &quadBounds)
    :
    DragQuad(index, start, offset, quad, quadBounds),
    points_(quad.GetPoints())
{

}


Quad DragQuadPoint::GetQuad(const tau::Point2d<int> &end) const
{
    auto adjusted = this->quad_;
    double magnitude = this->GetMagnitude(end);
    
    if (magnitude < 1.0)
    {
        return adjusted;
    }

    const auto &point = this->points_[this->index_];

    tau::Point2d<double> horizontalAnchor;
    tau::Point2d<double> verticalAnchor;

    // Perspective is applied symmetrically.
    // Adjustments made to the mirrored side of the quad have a mirrored
    // effect.
    double horizontalFlip = 1.0;
    double verticalFlip = 1.0;

    switch (this->index_)
    {
        case 0:
            // Dragging the top left corner
            horizontalAnchor = this->points_[1];
            verticalAnchor = this->points_[3];
            break;

        case 1:
            // Dragging the top right corner
            horizontalAnchor = this->points_[0];
            verticalAnchor = this->points_[2];
            verticalFlip = -1.0;
            break;

        case 2:
            // Dragging the bottom right corner
            horizontalAnchor = this->points_[3];
            verticalAnchor = this->points_[1];
            horizontalFlip = -1.0;
            verticalFlip = -1.0;
            break;

        case 3:
            // Dragging the bottom left corner
            horizontalAnchor = this->points_[2];
            verticalAnchor = this->points_[0];
            horizontalFlip = -1.0;
            break;

        default:
            throw std::logic_error("Quads only have 4 points");
    }

    double horizontalBaseline = (point - horizontalAnchor).GetAngle();
    double verticalBaseline = (point - verticalAnchor).GetAngle();

    double horizontalChange = (end - horizontalAnchor).GetAngle();
    double verticalChange = (end - verticalAnchor).GetAngle();

    double horizontalDifference =
        horizontalFlip
        * DragAngleDifference(horizontalChange, horizontalBaseline);

    double verticalDifference =
        verticalFlip
        * DragAngleDifference(verticalChange, verticalBaseline);

    adjusted.perspective.x =
        this->quadBounds_.perspective.x.Constrain(
            adjusted.perspective.x + horizontalDifference * 2.0);

    adjusted.perspective.y =
        this->quadBounds_.perspective.y.Constrain(
            adjusted.perspective.y + verticalDifference * 2.0);
    
    return adjusted;
}


Quad DragRotatePoint::GetQuad(const tau::Point2d<int> &end) const
{
    auto adjusted = this->quad_;
    double magnitude = this->GetMagnitude(end);
    
    if (magnitude < 1.0)
    {
        return adjusted;
    }

    const auto &point = this->points_[this->index_];

    auto center = this->quad_.center;
    auto baseline = (center - point).GetAngle();
    auto change = (center - end).GetAngle();
    auto difference = DragAngleDifference(change, baseline);

    adjusted.rotation += difference;
    adjusted.rotation = std::fmod(adjusted.rotation + 180, 360.0) - 180.0;
    
    return adjusted;
}


DragQuadLine::DragQuadLine(
    size_t index,
    const tau::Point2d<int> &start,
    const Quad &quad,
    const QuadBounds &quadBounds)
    :
    DragQuad(index, start, start.template Convert<double>(), quad, quadBounds),
    lines_(this->quad_.GetLines())
{

}


Quad DragQuadLine::GetQuad(const tau::Point2d<int> &end) const
{
    const auto &line = this->lines_[this->index_];
    double lineAngle = line.GetAngleDegrees();
    double dragAngle = this->GetAngle(end);
    double magnitude = this->GetMagnitude(end);
    double relativeAngle = tau::ToRadians(dragAngle - lineAngle);

    auto parallel = magnitude * std::cos(relativeAngle);
    auto perpendicular = -1 * magnitude * std::sin(relativeAngle);

    auto adjusted = this->quad_;

    if (std::abs(parallel) > std::abs(perpendicular))
    {
        double adjustment = parallel / adjusted.GetSideLength(this->index_);

        if (this->index_ == 0 || this->index_ == 2)
        {
            adjusted.shear.x = this->quadBounds_.shear.x.Constrain(
                adjusted.shear.x - adjustment);
        }
        else
        {
            adjusted.shear.y = this->quadBounds_.shear.y.Constrain(
                adjusted.shear.y + adjustment);
        }
    }
    else
    {
        static const double minimum = 1;
        double adjustment;
        tau::Vector2d<double> adjustmentVector;

        if (this->index_ == 0 || this->index_ == 2)
        {
            double minimumAdjustment = minimum - adjusted.size.height;
            adjustment = std::max(perpendicular, minimumAdjustment);
            adjusted.size.height += adjustment;
        }
        else
        {
            double minimumAdjustment = minimum - adjusted.size.width;
            adjustment = std::max(perpendicular, minimumAdjustment);
            adjusted.size.width += adjustment;
        }

        adjustmentVector = adjustment * line.vector.Rotate(-90) / 2.0;
        adjusted.center += adjustmentVector;
    }

    return adjusted;
}


QuadBrain::QuadBrain(
    QuadControl quadControl,
    PixelViewControl pixelViewControl)
    :
    quadControl_(quadControl),
    pixelViewControl_(pixelViewControl),
    mouseDownEndpoint_(
        this,
        pixelViewControl.mouseDown,
        &QuadBrain::OnMouseDown_),
    logicalPositionEndpoint_(
        this,
        pixelViewControl.logicalPosition,
        &QuadBrain::OnLogicalPosition_),
    modifierEndpoint_(
        this,
        pixelViewControl.modifier,
        &QuadBrain::OnModifier_),
    dragCenter_(),
    dragPoint_(),
    dragLine_(),
    dragCreate_()
{

}


void QuadBrain::OnMouseDown_(bool isDown)
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
    auto quad = this->quadControl_.Get();
    auto points = quad.GetPoints();
    auto pointIndex = GetPoint(click, points);

    if (pointIndex)
    {
        if (this->pixelViewControl_.modifier.Get().IsControl())
        {
            this->dragPoint_ =
                DragQuadPoint(
                    *pointIndex,
                    click,
                    points[*pointIndex],
                    quad,
                    QuadBounds(this->quadControl_));
        }
        else
        {
            this->dragRotatePoint_ =
                DragRotatePoint(
                    *pointIndex,
                    click,
                    points[*pointIndex],
                    quad,
                    QuadBounds(this->quadControl_));
        }

        return;
    }

    auto lineIndex = GetLine(click, points);

    if (lineIndex)
    {
        this->dragLine_ = DragQuadLine(
            *lineIndex,
            click,
            quad,
            QuadBounds(this->quadControl_));

        return;
    }

    if (quad.Contains(click.template Convert<double>()))
    {
        this->dragCenter_ = Drag(click, quad.center);

        this->pixelViewControl_.cursor.Set(
            wxpex::Cursor::closedHand);

        return;
    }

    this->dragCreate_ = Drag(click, click);
}


void QuadBrain::OnModifier_(const wxpex::Modifier &)
{
    this->UpdateCursor_();
}


void QuadBrain::OnLogicalPosition_(const tau::Point2d<int> &position)
{
    this->UpdateCursor_();

    if (this->dragCenter_)
    {
        this->quadControl_.center.Set(
            this->dragCenter_->GetPosition(position));

        return;
    }

    if (this->dragLine_)
    {
        this->quadControl_.Set(this->dragLine_->GetQuad(position));

        return;
    }

    if (this->dragPoint_)
    {
        this->quadControl_.Set(this->dragPoint_->GetQuad(position));

        return;
    }

    if (this->dragRotatePoint_)
    {
        this->quadControl_.Set(this->dragRotatePoint_->GetQuad(position));

        return;
    }

    if (this->dragCreate_)
    {
        auto size = this->dragCreate_->GetSize(position);
        
        if (size.GetArea() < 1)
        {
            return;
        }

        auto quad = Quad::Default();
        quad.center = this->dragCreate_->GetDragCenter(position);
        quad.size = this->dragCreate_->GetSize(position);
        this->quadControl_.Set(quad);

        return;
    }
}


bool QuadBrain::IsDragging_() const
{
    return (
        this->dragCenter_
        || this->dragPoint_
        || this->dragRotatePoint_
        || this->dragLine_
        || this->dragCreate_);
}


void QuadBrain::UpdateCursor_()
{
    if (this->IsDragging_())
    {
        return;
    }

    auto modifier = this->pixelViewControl_.modifier.Get();
    auto quad = this->quadControl_.Get();

    if (quad.size.GetArea() < 1.0)
    {
        return;
    }

    auto click = this->pixelViewControl_.logicalPosition.Get();
    auto points = quad.GetPoints();

    auto pointIndex = GetPoint(click, points);

    if (pointIndex)
    {
        // Hovering over a point, change cursor
        if (modifier.IsControl())
        {
            this->pixelViewControl_.cursor.Set(wxpex::Cursor::cross);
        }
        else
        {
            // TODO: Create a rotate cursor
            this->pixelViewControl_.cursor.Set(wxpex::Cursor::pointRight);
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

    if (quad.Contains(click.template Convert<double>()))
    {
        this->pixelViewControl_.cursor.Set(wxpex::Cursor::openHand);
    }
    else
    {
        this->pixelViewControl_.cursor.Set(wxpex::Cursor::arrow);
    }
}


} // end namespace draw
