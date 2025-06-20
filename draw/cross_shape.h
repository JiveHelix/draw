#pragma once


#include <pex/group.h>
#include "draw/cross.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/shape_edit.h"
#include "draw/views/cross_view.h"


namespace draw
{

void DrawCross(
    DrawContext &context,
    const Cross &cross);


struct CrossShapeTemplates: public ShapeCommon<CrossGroup, CrossView>
{
    template<typename Base>
    class Derived: public ShapeDerived<Base, Derived<Base>>
    {
    public:
        using Super = ShapeDerived<Base, Derived<Base>>;
        using Super::Super;

        void Draw(DrawContext &context) override
        {
            context.ConfigureLook(this->look);
            DrawCross(context, this->shape);
        }

        bool HandlesAltClick() const override { return false; }
        bool HandlesControlClick() const override { return false; }
        bool HandlesRotate() const override { return false; }
        bool HandlesEditPoint() const override { return false; }
        bool HandlesEditLine() const override { return false; }
        bool HandlesDrag() const override { return true; }

        std::string GetName() const override
        {
            return fmt::format("Cross {}", this->id);
        }

        std::unique_ptr<Drag> ProcessMouseDown(
            std::shared_ptr<ShapeControl> control,
            const tau::Point2d<int> &click,
            const wxpex::Modifier &modifier,
            CursorControl cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    IgnoreMouse,
                    IgnoreMouse,
                    IgnoreMouse,
                    DragShape<Derived>,
                    Derived
                >(control, *this, click, modifier, cursor);
        }
    };
};


using CrossShapePoly =
    pex::poly::Poly<ShapeFields, CrossShapeTemplates>;

using CrossShape = typename CrossShapePoly::Derived;
using CrossShapeModel = typename CrossShapePoly::Model;
using CrossShapeControl = typename CrossShapePoly::Control;


struct CreateCross
{
    std::optional<ShapeValue> operator()(
        const Drag &,
        const tau::Point2d<int> position)
    {
        auto cross = Cross{};
        cross.center = position;
        auto look = Look{};

        // ShapeCommon Template:
        /*
        T<pex::ReadOnly<ssize_t>> id;
        T<pex::OrderGroup> order;
        T<ShapeGroup> shape;
        T<LookGroup> look;
        T<NodeSettingsGroup> node;
        */

        return ShapeValue::Create<CrossShape>(
            0,
            pex::Order{},
            cross,
            look,
            NodeSettings{});
    }
};


using DragCreateCross = DragCreateShape<CreateCross>;
using CrossBrain = draw::ShapeBrain<DragCreateCross>;



} // end namespace draw
