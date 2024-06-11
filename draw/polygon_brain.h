#pragma once


#include <tau/vector2d.h>
#include "draw/polygon.h"
#include "draw/drag.h"
#include "draw/shape_edit.h"


namespace draw
{


template<typename Value>
struct CreatePolygon
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

        auto lines = PolygonLines(size);
        auto polygon = Polygon(lines.GetPoints());
        polygon.center = drag.GetDragCenter(position);

        return Value{{0, {}, polygon, Look::Default(), NodeSettings::Default()}};
    }
};


template<typename ListControl, typename Value>
using DragCreatePolygon =
    DragCreateShape<ListControl, CreatePolygon<Value>>;


template<typename ListControl, typename Value>
using DragReplacePolygon =
    DragReplaceShape<ListControl, CreatePolygon<Value>>;


template<typename DerivedShape>
class DragPolygonPoint: public DragEditPoint<DerivedShape>
{
public:
    using DragEditPoint<DerivedShape>::DragEditPoint;

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<int> &end) const override
    {
        double magnitude = this->GetMagnitude(end);

        if (magnitude < 1.0)
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        auto points = this->points_;
        points[this->index_] = this->GetPosition(end);

        return std::make_shared<DerivedShape>(
            typename DerivedShape::TemplateBase{
                this->startingShape_.id,
                {},
                Polygon(points),
                this->startingShape_.look,
                this->startingShape_.node});
    }
};


template<typename DerivedShape>
class DragRotatePolygonPoint: public DragEditPoint<DerivedShape>
{
public:
    using DragEditPoint<DerivedShape>::DragEditPoint;

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<int> &end) const override
    {
        auto adjusted = this->startingShape_;
        double magnitude = this->GetMagnitude(end);

        if (magnitude < 1.0)
        {
            return std::make_shared<DerivedShape>(adjusted);
        }

        const auto &point = this->points_[this->index_];

        auto center = adjusted.shape.center;
        auto baseline = (center - point).GetAngle();
        auto change = (center - end).GetAngle();
        auto difference = DragAngleDifference(change, baseline);
        adjusted.shape.rotation += difference;

        adjusted.shape.rotation =
            std::fmod(adjusted.shape.rotation + 180, 360.0) - 180.0;

        return std::make_shared<DerivedShape>(adjusted);
    }
};


template<typename DerivedShape>
class DragPolygonLine: public DragEditShape<DerivedShape>
{
public:
    DragPolygonLine(
        size_t index,
        const tau::Point2d<int> &start,
        const PolygonLines &lines,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        DragEditShape<DerivedShape>(
            index,
            start,
            start.template Cast<double>(),
            control,
            startingShape),
        lines_(lines)
    {
        this->SetOffset(this->lines_[index].point);
    }

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<int> &end) const override
    {
        auto lines = this->lines_;
        auto &line = lines[this->index_];
        double magnitude = this->GetMagnitude(end);

        if (magnitude < 1.0)
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        line.point = this->GetPosition(end);

        return std::make_shared<DerivedShape>(
            typename DerivedShape::TemplateBase{
                this->startingShape_.id,
                {},
                Polygon(lines.GetPoints()),
                this->startingShape_.look,
                this->startingShape_.node});
    }

private:
    PolygonLines lines_;
};


} // end namespace draw
