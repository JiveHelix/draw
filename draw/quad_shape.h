#pragma once


#include <pex/group.h>
#include <pex/derived_group.h>
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


struct QuadShapeTemplates: public ShapeCommon<QuadGroup, QuadView>
{
    template<typename Base>
    class DerivedValue: public ShapeDerived<Base, DerivedValue<Base>>
    {
    public:
        using Super = ShapeDerived<Base, DerivedValue<Base>>;
        using Super::Super;

        void Draw(DrawContext &context) override
        {
            if (this->shape.size.GetArea() < 0.5)
            {
                return;
            }

            context.ConfigureLook(this->look);
            DrawSegments(context, this->shape.GetPoints());
        }

        bool HandlesAltClick() const override { return false; }
        bool HandlesControlClick() const override { return false; }
        bool HandlesRotate() const override { return true; }
        bool HandlesEditPoint() const override { return true; }
        bool HandlesEditLine() const override { return true; }
        bool HandlesDrag() const override { return true; }

        std::string GetName() const override
        {
            return fmt::format("Quad {}", this->id);
        }

        std::unique_ptr<Drag> ProcessMouseDown(
            std::shared_ptr<ShapeControl> control,
            const tau::Point2d<double> &click,
            const wxpex::Modifier &modifier,
            const CursorControl &cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    DragRotateQuadPoint<DerivedValue>,
                    DragQuadPoint<DerivedValue, ControlMembers>,
                    DragQuadLine<DerivedValue, ControlMembers>,
                    DragShape<DerivedValue>,
                    DerivedValue
                >(control, *this, click, modifier, cursor);
        }
    };
};


using QuadShapeDerivedGroup =
    pex::poly::DerivedGroup<ShapeFields, QuadShapeTemplates>;

using QuadShape = typename QuadShapeDerivedGroup::DerivedValue;
using QuadShapeModel = typename QuadShapeDerivedGroup::Model;
using QuadShapeControl = typename QuadShapeDerivedGroup::Control;


struct CreateQuad
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

        auto quad = Quad{};
        quad.center = drag.GetDragCenter(position);
        quad.size = drag.GetSize(position);

        return ShapeValueWrapper::Create<QuadShape>(
            0,
            pex::Order{},
            quad,
            Look{},
            NodeSettings{});
    }
};


using DragCreateQuad = DragCreateShape<CreateQuad>;
using QuadBrain = draw::ShapeEditor<DragCreateQuad>;


} // end namespace draw
