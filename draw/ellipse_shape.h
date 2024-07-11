#pragma once


#include <pex/poly_group.h>
#include "draw/ellipse.h"
#include "draw/views/ellipse_view.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/shape_edit.h"


namespace draw
{


template<typename DerivedShape>
class DragEllipsePoint: public DragEditPoint<DerivedShape>
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
        auto end_ = end.template Cast<double>();
        RotatePoint(adjusted, this->points_[this->index_], end_);
        adjusted.shape.EditPoint(end_, this->index_);

        return std::make_shared<DerivedShape>(adjusted);
    }
};


template<typename DerivedShape>
class DragRotateEllipsePoint: public DragEditPoint<DerivedShape>
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


struct EllipseShapeTemplates: public ShapeCommon<EllipseGroup, EllipseView>
{
    template<typename Base>
    class Impl: public ShapeImpl<Base, Impl<Base>>
    {
    public:
        using ImplBase = ShapeImpl<Base, Impl<Base>>;
        using ImplBase::ImplBase;

        bool HandlesAltClick() const override { return false; }
        bool HandlesControlClick() const override { return false; }
        bool HandlesRotate() const override { return true; }
        bool HandlesEditPoint() const override { return true; }
        bool HandlesEditLine() const override { return false; }
        bool HandlesDrag() const override { return true; }

        void Draw(wxpex::GraphicsContext &context) override
        {
            wxpex::MaintainTransform maintainTransform(context);
            ConfigureLook(context, this->look);

            auto center = this->shape.center;
            auto transform = context->GetTransform();
            transform.Translate(center.x, center.y);
            context->SetTransform(transform);
            context->Rotate(tau::ToRadians(this->shape.rotation));
            double ellipseMajor = this->shape.scale * this->shape.major;
            double ellipseMinor = this->shape.scale * this->shape.minor;

            // wx draws the ellipse contained within this rectangle.
            context->DrawEllipse(
                -ellipseMajor / 2.0,
                -ellipseMinor / 2.0,
                ellipseMajor,
                ellipseMinor);
        }

        std::string GetName() const override
        {
            return fmt::format("Ellipse {}", this->id);
        }

        std::unique_ptr<Drag> ProcessMouseDown(
            std::shared_ptr<ShapeControl> control,
            const tau::Point2d<int> &click,
            const wxpex::Modifier &modifier,
            CursorControl cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    DragRotateEllipsePoint<Impl>,
                    DragEllipsePoint<Impl>,
                    IgnoreMouse,
                    DragShape<Impl>,
                    Impl
                >(control, *this, click, modifier, cursor);
        }
    };
};


using EllipseShapePolyGroup =
    pex::poly::PolyGroup<ShapeFields, EllipseShapeTemplates>;

using EllipseShapeValue = typename EllipseShapePolyGroup::PolyValue;
using EllipseShapeModel = typename EllipseShapePolyGroup::Model;
using EllipseShapeControl = typename EllipseShapePolyGroup::Control;


struct CreateEllipse
{
    std::optional<EllipseShapeValue> operator()(
        const Drag &drag,
        const tau::Point2d<int> position)
    {
        auto size = drag.GetSize(position);

        if (size.GetArea() < 1)
        {
            return {};
        }

        Ellipse ellipse{};
        ellipse.center = drag.GetDragCenter(position);
        ellipse.major = size.width;
        ellipse.minor = size.height;
        ellipse.rotation = 0.0;
        ellipse.scale = 1.0;

        return EllipseShapeValue{{
            0,
            {},
            ellipse,
            Look::Default(),
            NodeSettings::Default()}};
    }
};


using DragCreateEllipse = DragCreateShape<CreateEllipse>;
using EllipseBrain = draw::ShapeBrain<DragCreateEllipse>;


} // end namespace draw
