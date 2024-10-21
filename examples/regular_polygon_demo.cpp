

#include "common/shape_demo_brain.h"
#include <draw/regular_polygon_shape.h>


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
            draw::ShapeValue::Default<draw::RegularPolygonShape>());
    }

    std::string GetAppName() const
    {
        return "Regular Polygon Demo";
    }

private:
    draw::RegularPolygonBrain polygonBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
