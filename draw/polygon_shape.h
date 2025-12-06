#pragma once


#include <pex/group.h>
#include <pex/derived_group.h>
#include <wxpex/modifier.h>

#include "draw/polygon.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/oddeven.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/views/polygon_view.h"
#include "draw/views/look_view.h"
#include "draw/detail/poly_shape_id.h"
#include "draw/node_settings.h"
#include "draw/polygon_brain.h"
#include "draw/draw_segments.h"


namespace draw
{


struct PolygonShapeTemplates: public ShapeCommon<PolygonGroup, PolygonView>
{
    template<typename Base>
    class DerivedValue: public ShapeDerived<Base, DerivedValue<Base>>
    {
    public:
        using Super = ShapeDerived<Base, DerivedValue<Base>>;
        using Super::Super;

        bool HandlesAltClick() const override { return true; }
        bool HandlesControlClick() const override { return true; }
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
            return fmt::format("Polygon {}", this->id);
        }

        std::unique_ptr<Drag> ProcessMouseDown(
            std::shared_ptr<ShapeControl> control,
            const tau::Point2d<double> &click,
            const wxpex::Modifier &modifier,
            const CursorControl &cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    DragRotatePolygonPoint<DerivedValue>,
                    DragPolygonPoint<DerivedValue>,
                    DragPolygonLine<DerivedValue>,
                    DragShape<DerivedValue>,
                    DerivedValue
                >(control, *this, click, modifier, cursor);
        }

        bool ProcessControlClick(
            ShapeControl &control,
            const tau::Point2d<double> &click) override
        {
            auto points = this->shape.GetPoints();
            points.push_back(click.template Cast<double>());
            this->shape = Polygon(points);
            control.SetValue(*this);

            return true;
        }

        bool ProcessAltClick(
            ShapeControl &control,
            PointsIterator iterator,
            PointsDouble &points) override
        {
            // Subtract a point
            points.erase(iterator);
            this->shape = Polygon(points);
            control.SetValue(*this);

            return true;
        }
    };
};


using PolygonShapeDerivedGroup =
    pex::poly::DerivedGroup<ShapeFields, PolygonShapeTemplates>;

using PolygonShape = typename PolygonShapeDerivedGroup::DerivedValue;
using PolygonShapeModel = typename PolygonShapeDerivedGroup::Model;
using PolygonShapeControl = typename PolygonShapeDerivedGroup::Control;


struct CreatePolygon
{
    std::optional<ShapeValueWrapper> operator()(
        const Drag &drag,
        const tau::Point2d<double> position)
    {
        auto size = drag.GetSize(position);

        if (size.GetArea() < 1)
        {
            return {};
        }

        auto lines = PolygonLines(size);
        auto polygon = Polygon(lines.GetPoints());
        polygon.center = drag.GetDragCenter(position);

        return ShapeValueWrapper::Create<PolygonShape>(
            0,
            pex::Order{},
            polygon,
            Look{},
            NodeSettings{});
    }
};


using DragCreatePolygon = DragCreateShape<CreatePolygon>;
using DragReplacePolygon = DragReplaceShape<CreatePolygon>;

using PolygonBrain = draw::ShapeEditor<DragCreatePolygon>;


} // end namespace draw
