#pragma once


#include <pex/group.h>
#include <pex/poly.h>
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
    class Derived: public ShapeDerived<Base, Derived<Base>>
    {
    public:
        using Super = ShapeDerived<Base, Derived<Base>>;
        using Super::Super;

        void Draw(wxpex::GraphicsContext &context) override
        {
            if (this->shape.size.GetArea() < 0.5)
            {
                return;
            }

            ConfigureLook(context, this->look);
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
            const tau::Point2d<int> &click,
            const wxpex::Modifier &modifier,
            CursorControl cursor) override
        {
            return ::draw::ProcessMouseDown
                <
                    DragRotateQuadPoint<Derived>,
                    DragQuadPoint<Derived, ControlMembers>,
                    DragQuadLine<Derived, ControlMembers>,
                    DragShape<Derived>,
                    Derived
                >(control, *this, click, modifier, cursor);
        }
    };
};


using QuadShapePoly =
    pex::poly::Poly<ShapeFields, QuadShapeTemplates>;

using QuadShapeValue = typename QuadShapePoly::PolyValue;
using QuadShapeModel = typename QuadShapePoly::Model;
using QuadShapeControl = typename QuadShapePoly::Control;

using DragCreateQuad = DragCreateShape<CreateQuad<QuadShapeValue>>;
using QuadBrain = draw::ShapeBrain<DragCreateQuad>;


} // end namespace draw
