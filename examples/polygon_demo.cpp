

#include "common/shape_demo_brain.h"


class DemoBrain: public ShapeDemoBrain<DemoBrain>
{
public:
    DemoBrain()
        :
        ShapeDemoBrain<DemoBrain>(),
        polygonBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView.canvas)
    {
        this->demoControl_.shapes.Append(
            draw::ShapeValueWrapper::Default<draw::PolygonShape>());
    }

    std::string GetAppName() const
    {
        return "Polygon Demo";
    }

private:
    draw::PolygonBrain polygonBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
