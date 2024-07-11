#pragma once


#include <pex/group.h>
#include <pex/poly_group.h>
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
    class Impl: public ShapeImpl<Base, Impl<Base>>
    {
    public:
        using ImplBase = ShapeImpl<Base, Impl<Base>>;
        using ImplBase::ImplBase;

        bool HandlesAltClick() const override { return true; }
        bool HandlesControlClick() const override { return true; }
        bool HandlesRotate() const override { return true; }
        bool HandlesEditPoint() const override { return true; }
        bool HandlesEditLine() const override { return true; }
        bool HandlesDrag() const override { return true; }

        void Draw(wxpex::GraphicsContext &context) override
        {
            auto points = this->shape.GetPoints();

            if (points.empty())
            {
                return;
            }

            ConfigureLook(context, this->look);
            DrawSegments(context, points);
        }

        std::string GetName() const override
        {
            return fmt::format("Polygon {}", this->id);
        }

        std::unique_ptr<Drag> ProcessMouseDown(
            std::shared_ptr<ShapeControl> control,
            const tau::Point2d<int> &click,
            const wxpex::Modifier &modifier,
            CursorControl cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    DragRotatePolygonPoint<Impl>,
                    DragPolygonPoint<Impl>,
                    DragPolygonLine<Impl>,
                    DragShape<Impl>,
                    Impl
                >(control, *this, click, modifier, cursor);
        }

        bool ProcessControlClick(
            ShapeControl &control,
            const tau::Point2d<int> &click) override
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


using PolygonShapePolyGroup =
    pex::poly::PolyGroup<ShapeFields, PolygonShapeTemplates>;

using PolygonShapeValue = typename PolygonShapePolyGroup::PolyValue;
using PolygonShapeModel = typename PolygonShapePolyGroup::Model;
using PolygonShapeControl = typename PolygonShapePolyGroup::Control;

using DragCreatePolygon =
    DragCreateShape<CreatePolygon<PolygonShapeValue>>;

using PolygonBrain = draw::ShapeBrain<DragCreatePolygon>;


using DragReplacePolygon =
    DragReplaceShape<CreatePolygon<PolygonShapeValue>>;



} // end namespace draw
