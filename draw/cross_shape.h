#pragma once


#include <pex/group.h>
#include "draw/cross.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/shape_editor.h"
#include "draw/views/cross_view.h"


namespace draw
{

void DrawCross(
    DrawContext &context,
    const Cross &cross);


struct CrossShapeTemplates: public ShapeCommon<CrossGroup, CrossView>
{
    template<typename Base>
    class DerivedValue: public ShapeDerived<Base, DerivedValue<Base>>
    {
    public:
        using Super = ShapeDerived<Base, DerivedValue<Base>>;
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
            const tau::Point2d<double> &click,
            const wxpex::Modifier &modifier,
            CursorControl cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    IgnoreMouse,
                    IgnoreMouse,
                    IgnoreMouse,
                    DragShape<DerivedValue>,
                    DerivedValue
                >(control, *this, click, modifier, cursor);
        }
    };
};


using CrossShapeDerivedGroup =
    pex::poly::DerivedGroup<ShapeFields, CrossShapeTemplates>;

using CrossShape = typename CrossShapeDerivedGroup::DerivedValue;
using CrossShapeModel = typename CrossShapeDerivedGroup::Model;
using CrossShapeControl = typename CrossShapeDerivedGroup::Control;


struct CreateCross
{
    std::optional<ShapeValueWrapper> operator()(
        const Drag &,
        const tau::Point2d<double> position)
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

        return ShapeValueWrapper::Create<CrossShape>(
            0,
            pex::Order{},
            cross,
            look,
            NodeSettings{});
    }
};


using DragCreateCross = DragCreateShape<CreateCross>;
using CrossBrain = draw::ShapeEditor<DragCreateCross>;



} // end namespace draw
