#include "common/shape_demo_brain.h"


class DemoBrain: public ShapeDemoBrain<DemoBrain>
{
public:
    DemoBrain()
        :
        ShapeDemoBrain<DemoBrain>(),
        ellipseBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView.canvas)
    {
        this->demoControl_.shapes.Append(
            draw::ShapeValue::Default<draw::EllipseShape>());
    }

    std::string GetAppName() const
    {
        return "Ellipse Demo";
    }

private:
    draw::EllipseBrain ellipseBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
