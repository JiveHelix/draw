#pragma once


#include <tau/vector2d.h>
#include "draw/polygon.h"
#include "draw/drag.h"
#include "draw/shape_editor.h"
#include "draw/shape_list.h"


namespace draw
{


template<typename DerivedShape>
class DragPolygonPoint: public DragEditPoint<DerivedShape>
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


template<typename DerivedShape>
class DragPolygonLine: public DragEditShape<DerivedShape>
{
public:
    virtual ~DragPolygonLine() {}

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
        if (end == this->start_)
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        auto lines = this->lines_;
        auto &line = lines[this->index_];
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
