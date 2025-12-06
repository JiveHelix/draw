#pragma once


#include <pex/derived_group.h>
#include "draw/ellipse.h"
#include "draw/views/ellipse_view.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/shape_editor.h"


namespace draw
{


template<typename DerivedShape>
class DragEllipsePoint: public DragEditPoint<DerivedShape>
{
public:
    using DragEditPoint<DerivedShape>::DragEditPoint;

protected:
    std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<double> &end) const override
    {
        if (IsSamePoint(end, this->start_))
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
        const tau::Point2d<double> &end) const override
    {
        if (IsSamePoint(end, this->start_))
        {
            return std::make_shared<DerivedShape>(this->startingShape_);
        }

        auto adjusted = this->startingShape_;

        RotatePoint(
            adjusted,
            this->points_[this->index_],
            end);

        return std::make_shared<DerivedShape>(adjusted);
    }
};


struct EllipseShapeTemplates: public ShapeCommon<EllipseGroup, EllipseView>
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
            const tau::Point2d<double> &click,
            const wxpex::Modifier &modifier,
            const CursorControl &cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    DragRotateEllipsePoint<DerivedValue>,
                    DragEllipsePoint<DerivedValue>,
                    IgnoreMouse,
                    DragShape<DerivedValue>,
                    DerivedValue
                >(control, *this, click, modifier, cursor);
        }
    };
};


using EllipseShapeDerivedGroup =
    pex::poly::DerivedGroup<ShapeFields, EllipseShapeTemplates>;

using EllipseShape = typename EllipseShapeDerivedGroup::DerivedValue;
using EllipseShapeModel = typename EllipseShapeDerivedGroup::Model;
using EllipseShapeControl = typename EllipseShapeDerivedGroup::Control;


struct CreateEllipse
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

        Ellipse ellipse{};
        ellipse.center = drag.GetDragCenter(position);
        ellipse.major = size.width;
        ellipse.minor = size.height;
        ellipse.rotation = 0.0;
        ellipse.scale = 1.0;

        return ShapeValueWrapper::Create<EllipseShape>(
            0,
            pex::Order{},
            ellipse,
            Look{},
            NodeSettings{});
    }
};


using DragCreateEllipse = DragCreateShape<CreateEllipse>;
using EllipseBrain = draw::ShapeEditor<DragCreateEllipse>;


} // end namespace draw
