

#include "common/shape_demo_brain.h"


// TODO: Move to unit tests
using ShapeControlBase = typename draw::Shape::ControlBase;

static_assert(std::is_base_of_v<ShapeControlBase, draw::QuadShapeControl>);

static_assert(
    std::is_base_of_v
    <
        typename draw::QuadShapeTemplates::ControlMembers,
        draw::QuadShapeControl
    >);


class DemoBrain: public ShapeDemoBrain<DemoBrain>
{
public:
    DemoBrain()
        :
        ShapeDemoBrain<DemoBrain>(),
        quadBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView.canvas)
    {
        this->demoControl_.shapes.Append(
            draw::ShapeValue::Default<draw::QuadShape>());
    }

    std::string GetAppName() const
    {
        return "Quad Demo";
    }

private:
    draw::QuadBrain quadBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
