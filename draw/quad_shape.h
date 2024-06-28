#pragma once


#include <pex/group.h>
#include <pex/poly_group.h>
#include <wxpex/modifier.h>

#include "draw/quad.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/oddeven.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/views/quad_view.h"
#include "draw/views/look_view.h"
#include "draw/detail/poly_shape_id.h"
#include "draw/node_settings.h"
#include "draw/quad_brain.h"
#include "draw/draw_segments.h"


namespace draw
{


template<template<typename> typename T>
class QuadShapeTemplate
{
public:
    // id is read-only to a control
    T<pex::ReadOnly<ssize_t>> id;
    T<DepthOrderGroup> depthOrder;
    T<QuadGroup> shape;
    T<LookGroup> look;
    T<NodeSettingsGroup> node;

    static constexpr auto fields =
        ShapeFields<QuadShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "QuadShape";

    // pex::poly::Value generates a variadic pack of PolyDerived types using
    // ValueBase_ and a variadic pack of templates (like this one).
    // We need to override some of the virtual methods in draw::Shape.
    // The template argument Base should be
    // pex::poly::PolyDerived_<draw::Shape, QuadShapeTemplate>
    //
    // TODO: There must be a less labyrinthine architecture.
    template<typename Base>
    class Impl: public Base
    {
        static_assert(
            std::is_base_of_v<Shape, Base>,
            "Expected Base to be derived from Shape.");

    public:
        using Base::Base;

        static Impl Default()
        {
            return {{
                0,
                {},
                Quad::Default(),
                Look::Default(),
                NodeSettings::Default()}};
        }

        void Draw(wxpex::GraphicsContext &context) override
        {
            if (this->shape.size.GetArea() < 0.5)
            {
                return;
            }

            ConfigureLook(context, this->look);
            DrawSegments(context, this->shape.GetPoints());
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

        bool HandlesAltClick() const override { return false; }
        bool HandlesControlClick() const override { return false; }
        bool HandlesRotate() const override { return true; }
        bool HandlesEditPoint() const override { return true; }
        bool HandlesEditLine() const override { return true; }

        std::string GetName() const override
        {
            return fmt::format("Quad {}", this->id);
        }

        std::unique_ptr<Drag> ProcessMouseDown(
            std::shared_ptr<ShapeControl> control,
            const tau::Point2d<int> &click,
            const wxpex::Modifier &modifier,
            CursorControl cursor) override
        {
            using QuadControlMembers = pex::ControlMembers_<QuadShapeTemplate>;

            return ::draw::ProcessMouseDown
                <
                    DragRotateQuadPoint<Impl>,
                    DragQuadPoint<Impl, QuadControlMembers>,
                    DragQuadLine<Impl, QuadControlMembers>,
                    DragShape<Impl>,
                    Impl
                >(control, *this, click, modifier, cursor);
        }
    };
};


using QuadShapePolyGroup = pex::poly::PolyGroup
<
    ShapeFields,
    QuadShapeTemplate,
    Shape,
    ShapeCustom<QuadView>
>;


using QuadShapeValue = typename QuadShapePolyGroup::PolyValue;
using QuadShapeModel = typename QuadShapePolyGroup::Model;
using QuadShapeControl = typename QuadShapePolyGroup::Control;


using DragCreateQuad =
    DragCreateShape<ShapesControl, CreateQuad<QuadShapeValue>>;


using QuadBrain = draw::ShapeBrain<DragCreateQuad>;


} // end namespace draw
