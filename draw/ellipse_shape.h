#pragma once


#include <pex/poly.h>
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
    class Derived: public ShapeDerived<Base, Derived<Base>>
    {
    public:
        using Super = ShapeDerived<Base, Derived<Base>>;
        using Super::Super;

        bool HandlesAltClick() const override { return false; }
        bool HandlesControlClick() const override { return false; }
        bool HandlesRotate() const override { return true; }
        bool HandlesEditPoint() const override { return true; }
        bool HandlesEditLine() const override { return false; }
        bool HandlesDrag() const override { return true; }

        void Draw(DrawContext &context) override
        {
            context.ConfigureLook(this->look);
            this->shape.Draw(context);
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
                    DragRotateEllipsePoint<Derived>,
                    DragEllipsePoint<Derived>,
                    IgnoreMouse,
                    DragShape<Derived>,
                    Derived
                >(control, *this, click, modifier, cursor);
        }
    };
};


using EllipseShapePoly =
    pex::poly::Poly<ShapeFields, EllipseShapeTemplates>;

using EllipseShape = typename EllipseShapePoly::Derived;
using EllipseShapeModel = typename EllipseShapePoly::Model;
using EllipseShapeControl = typename EllipseShapePoly::Control;


struct CreateEllipse
{
    std::optional<ShapeValue> operator()(
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

        return ShapeValue::Create<EllipseShape>(
            0,
            pex::Order{},
            ellipse,
            Look{},
            NodeSettings{});
    }
};


using DragCreateEllipse = DragCreateShape<CreateEllipse>;
using EllipseBrain = draw::ShapeBrain<DragCreateEllipse>;


} // end namespace draw
