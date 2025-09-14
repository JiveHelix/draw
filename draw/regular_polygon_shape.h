#pragma once


#include <pex/group.h>
#include <pex/derived_group.h>
#include <wxpex/modifier.h>

#include "draw/regular_polygon.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/oddeven.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/views/regular_polygon_view.h"
#include "draw/views/look_view.h"
#include "draw/detail/poly_shape_id.h"
#include "draw/node_settings.h"
#include "draw/draw_segments.h"


namespace draw
{


template<typename DerivedShape>
class DragRegularPolygonPoint: public DragEditPoint<DerivedShape>
{
public:
    using DragEditPoint<DerivedShape>::DragEditPoint;

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<double> &end) const override
    {
        if (end == this->start_)
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        auto adjusted = this->startingShape_;
        auto position = this->GetPosition(end);
        auto newRadius = adjusted.shape.center.Distance(position);

        auto oldVector =
            tau::Vector2d<double>(
                adjusted.shape.center,
                this->GetOffset());

        auto newVector =
            tau::Vector2d<double>(
                adjusted.shape.center,
                position);

        auto newShape = adjusted.shape.Rotate_deg(
            oldVector.GetAngle_deg(newVector));

        newShape.SetRadius(newRadius);
        adjusted.shape = newShape;

        return std::make_shared<DerivedShape>(adjusted);
    }
};


template<typename DerivedShape>
class DragRotateRegularPolygonPoint: public DragEditPoint<DerivedShape>
{
public:
    using DragEditPoint<DerivedShape>::DragEditPoint;

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<double> &end) const override
    {
        if (end == this->start_)
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        auto adjusted = this->startingShape_;
        auto position = this->GetPosition(end);

        auto oldVector =
            tau::Vector2d<double>(
                adjusted.shape.center,
                this->GetOffset());

        auto newVector =
            tau::Vector2d<double>(
                adjusted.shape.center,
                position);

        adjusted.shape = adjusted.shape.Rotate_deg(
            oldVector.GetAngle_deg(newVector));

        return std::make_shared<DerivedShape>(adjusted);
    }
};


template<typename DerivedShape>
class DragRegularPolygonLine: public DragEditShape<DerivedShape>
{
public:
    virtual ~DragRegularPolygonLine() {}

    DragRegularPolygonLine(
        size_t index,
        const tau::Point2d<double> &start,
        const PolygonLines &lines,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        DragEditShape<DerivedShape>(
            index,
            start,
            lines[index].point,
            control,
            startingShape),
        lines_(lines)
    {

    }

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<double> &end) const override
    {
        if (end == this->start_)
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        auto adjusted = this->startingShape_;
        auto position = this->GetPosition(end);

        auto radius = adjusted.shape.center.Distance(position);

        auto oldVector =
            tau::Vector2d<double>(
                adjusted.shape.center,
                this->start_);

        auto newVector =
            tau::Vector2d<double>(
                adjusted.shape.center,
                end);

        auto newShape = adjusted.shape.Rotate_deg(
            oldVector.GetAngle_deg(newVector));

        // The reference point of a line in this case is defined to be the
        // line's midpoint.
        newShape.SetMidpointRadius(radius);
        adjusted.shape = newShape;

        return std::make_shared<DerivedShape>(adjusted);
    }

private:
    PolygonLines lines_;
};


struct RegularPolygonShapeTemplates
    :
    public ShapeCommon<RegularPolygonGroup, RegularPolygonView>
{
    template<typename Base>
    class DerivedValue: public ShapeDerived<Base, DerivedValue<Base>>
    {
    public:
        using Super = ShapeDerived<Base, DerivedValue<Base>>;
        using Super::Super;

        bool HandlesAltClick() const override { return false; }
        bool HandlesControlClick() const override { return false; }
        bool HandlesRotate() const override { return true; }
        bool HandlesEditPoint() const override { return true; }
        bool HandlesEditLine() const override { return true; }
        bool HandlesDrag() const override { return true; }

        void Draw(DrawContext &context) override
        {
            auto points = this->shape.GetPoints();

            if (points.empty())
            {
                return;
            }

            context.ConfigureLook(this->look);
            DrawSegments(context, points);
        }

        std::string GetName() const override
        {
            return fmt::format("RegularPolygon {}", this->id);
        }

        std::unique_ptr<Drag> ProcessMouseDown(
            std::shared_ptr<ShapeControl> control,
            const tau::Point2d<double> &click,
            const wxpex::Modifier &modifier,
            CursorControl cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    DragRotateRegularPolygonPoint<DerivedValue>,
                    DragRegularPolygonPoint<DerivedValue>,
                    DragRegularPolygonLine<DerivedValue>,
                    DragShape<DerivedValue>,
                    DerivedValue
                >(control, *this, click, modifier, cursor);
        }
    };
};


using RegularPolygonShapeDerivedGroup =
    pex::poly::DerivedGroup<ShapeFields, RegularPolygonShapeTemplates>;

using RegularPolygonShape =
    typename RegularPolygonShapeDerivedGroup::DerivedValue;

using RegularPolygonShapeModel =
    typename RegularPolygonShapeDerivedGroup::Model;

using RegularPolygonShapeControl =
    typename RegularPolygonShapeDerivedGroup::Control;


struct CreateRegularPolygon
{
    std::optional<ShapeValueWrapper> operator()(
        const Drag &drag,
        const tau::Point2d<double> position)
    {
        auto magnitude = drag.GetMagnitude(position);

        if (magnitude < 1.)
        {
            return {};
        }

        auto angle = drag.GetAngle(position);

        RegularPolygon shape(
            drag.GetStart(),
            magnitude,
            3,
            angle);

        shape.SetRadius(magnitude);

        return ShapeValueWrapper::Create<RegularPolygonShape>(
            0,
            pex::Order{},
            shape,
            Look{},
            NodeSettings{});
    }
};


using DragCreateRegularPolygon = DragCreateShape<CreateRegularPolygon>;
using RegularPolygonBrain = draw::ShapeEditor<DragCreateRegularPolygon>;


} // end namespace draw
