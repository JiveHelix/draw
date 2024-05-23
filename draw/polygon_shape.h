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


template<template<typename> typename T>
class PolygonShapeTemplate
{
public:
    // id is read-only to a control
    T<pex::ReadOnly<ssize_t>> id;
    T<PolygonGroup> shape;
    T<LookGroup> look;
    T<NodeSettingsGroup> node;

    static constexpr auto fields =
        ShapeFields<PolygonShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "PolygonShape";

    template<typename Base>
    class Impl: public Base
    {
    public:
        using Base::Base;

        static Impl Default()
        {
            return {{
                0,
                Polygon::Default(),
                Look::Default(),
                NodeSettings::Default()}};
        }

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

        ssize_t GetId() const override
        {
            return this->id;
        }

        PointsDouble GetPoints() const override
        {
            return this->shape.GetPoints();
        }

        bool Contains(
            const tau::Point2d<int> &point,
            double margin) const override
        {
            return this->shape.Contains(point, margin);
        }

        std::shared_ptr<Shape> Copy() const override
        {
            return std::make_shared<Impl>(*this);
        }

        bool HandlesAltClick() const override { return true; }
        bool HandlesControlClick() const override { return true; }

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
            control.SetValueBase(*this);

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
            control.SetValueBase(*this);

            return true;
        }
    };
};


template<typename Value>
using PolygonShapePolyGroup = pex::poly::PolyGroup
<
    ShapeFields,
    PolygonShapeTemplate,
    Value,
    ShapeTemplates<PolygonView>
>;


} // end namespace draw
