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


std::optional<QuadShape> CreateQuad::operator()(
    const Drag &drag,
    const tau::Point2d<int> position)
{
    auto size = drag.GetSize(position);

    if (size.GetArea() < 1)
    {
        return {};
    }

    auto quad = Quad::Default();
    quad.center = drag.GetDragCenter(position);
    quad.size = drag.GetSize(position);

    return QuadShape{quad, Look::Default()};
}


Quad DragQuadPoint::MakeShape_(const tau::Point2d<int> &end) const
{
    auto adjusted = this->startingShape_;
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

    auto quadBounds = QuadBounds(this->control_);

    adjusted.perspective.x =
        quadBounds.perspective.x.Constrain(
            adjusted.perspective.x + horizontalDifference * 2.0);

    adjusted.perspective.y =
        quadBounds.perspective.y.Constrain(
            adjusted.perspective.y + verticalDifference * 2.0);
    
    return adjusted;
}


Quad DragRotateQuadPoint::MakeShape_(const tau::Point2d<int> &end) const
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


DragQuadLine::DragQuadLine(
    size_t index,
    const tau::Point2d<int> &start,
    QuadControl quad,
    const QuadLines &quadLines)
    :
    DragEditQuad(index, start, start.template Convert<double>(), quad),
    lines_(quadLines)
{

}


Quad DragQuadLine::MakeShape_(const tau::Point2d<int> &end) const
{
    const auto &line = this->lines_[this->index_];
    double lineAngle = line.GetAngleDegrees();
    double dragAngle = this->GetAngle(end);
    double magnitude = this->GetMagnitude(end);
    double relativeAngle = tau::ToRadians(dragAngle - lineAngle);

    auto parallel = magnitude * std::cos(relativeAngle);
    auto perpendicular = -1 * magnitude * std::sin(relativeAngle);

    auto adjusted = this->startingShape_;

    if (std::abs(parallel) > std::abs(perpendicular))
    {
        double adjustment = parallel / adjusted.GetSideLength(this->index_);

        auto quadBounds = QuadBounds(this->control_);

        if (this->index_ == 0 || this->index_ == 2)
        {
            adjusted.shear.x = quadBounds.shear.x.Constrain(
                adjusted.shear.x - adjustment);
        }
        else
        {
            adjusted.shear.y = quadBounds.shear.y.Constrain(
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


} // end namespace draw
