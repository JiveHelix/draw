#pragma once


#include <tau/vector2d.h>
#include "draw/quad.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/polygon.h"
#include "draw/shape_edit.h"


namespace draw
{


struct Bounds2d
{
    pex::Bounds<double> x;
    pex::Bounds<double> y;

    template<typename Control>
    static Bounds2d Make(Control control)
    {
        return {control.x.GetBounds(), control.y.GetBounds()};
    }
};


struct QuadBounds
{
    Bounds2d shear;
    Bounds2d perspective;
    pex::Bounds<double> rotation;

    QuadBounds(QuadControl control);
};


template<typename Value>
struct CreateQuad
{
    std::optional<Value> operator()(
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

        return Value{{0, {}, quad, Look::Default(), NodeSettings::Default()}};
    }
};


template<typename DerivedShape, typename QuadControlMembers>
class DragQuadPoint: public DragEditPoint<DerivedShape>
{
public:
    using DragEditPoint<DerivedShape>::DragEditPoint;

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<int> &end) const override
    {
        if (end == this->start_)
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        auto adjusted = this->startingShape_;

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

        auto quadControlMembers = dynamic_cast<QuadControlMembers *>(
            this->control_.get());

        if (!quadControlMembers)
        {
            throw std::logic_error("Cast failed");
        }

        auto quadBounds = QuadBounds(quadControlMembers->shape);

        adjusted.shape.perspective.x =
            quadBounds.perspective.x.Constrain(
                adjusted.shape.perspective.x + horizontalDifference * 2.0);

        adjusted.shape.perspective.y =
            quadBounds.perspective.y.Constrain(
                adjusted.shape.perspective.y + verticalDifference * 2.0);

        return std::make_shared<DerivedShape>(adjusted);

    }
};


template<typename DerivedShape>
class DragRotateQuadPoint: public DragEditPoint<DerivedShape>
{
public:
    using DragEditPoint<DerivedShape>::DragEditPoint;

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<int> &end) const override
    {
        if (end == this->start_)
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        auto adjusted = this->startingShape_;

        RotatePoint(
            adjusted,
            this->points_[this->index_],
            end.template Cast<double>());

        return std::make_shared<DerivedShape>(adjusted);
    }
};


template<typename DerivedShape, typename QuadControlMembers>
class DragQuadLine: public DragEditShape<DerivedShape>
{
public:
    DragQuadLine(
        size_t index,
        const tau::Point2d<int> &start,
        const QuadLines &quadLines,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        DragEditShape<DerivedShape>(
            index,
            start,
            start.template Cast<double>(),
            control,
            startingShape),
        lines_(quadLines)
    {

    }

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<int> &end) const override
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
            double adjustment = parallel
                / adjusted.shape.GetSideLength(this->index_);

            auto quadControlMembers = dynamic_cast<QuadControlMembers *>(
                this->control_.get());

            if (!quadControlMembers)
            {
                throw std::logic_error("Cast failed");
            }

            auto quadBounds = QuadBounds(quadControlMembers->shape);

            if (this->index_ == 0 || this->index_ == 2)
            {
                adjusted.shape.shear.x = quadBounds.shear.x.Constrain(
                    adjusted.shape.shear.x - adjustment);
            }
            else
            {
                adjusted.shape.shear.y = quadBounds.shear.y.Constrain(
                    adjusted.shape.shear.y + adjustment);
            }
        }
        else
        {
            static const double minimum = 1;
            double adjustment;
            tau::Vector2d<double> adjustmentVector;

            if (this->index_ == 0 || this->index_ == 2)
            {
                double minimumAdjustment = minimum - adjusted.shape.size.height;
                adjustment = std::max(perpendicular, minimumAdjustment);
                adjusted.shape.size.height +=
                    static_cast<int>(std::round(adjustment));
            }
            else
            {
                double minimumAdjustment = minimum - adjusted.shape.size.width;
                adjustment = std::max(perpendicular, minimumAdjustment);
                adjusted.shape.size.width +=
                    static_cast<int>(std::round(adjustment));
            }

            adjustmentVector = adjustment * line.vector.Rotate(-90) / 2.0;
            adjusted.shape.center += adjustmentVector;
        }

        return std::make_shared<DerivedShape>(adjusted);
    }

private:
    QuadLines lines_;
};


} // end namespace draw
