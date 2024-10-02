

#include "common/shape_demo_brain.h"
#include <draw/hue_generator.h>


class DemoBrain: public ShapeDemoBrain<DemoBrain>
{
public:
    DemoBrain()
        :
        ShapeDemoBrain<DemoBrain>(),
        hueGenerator_(),

        polygonBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView),

        countEndpoint_(
            this,
            this->demoControl_.shapes.count,
            &DemoBrain::OnShapeAdded_)
    {
        this->demoControl_.shapes.Append(
            draw::ShapeValue::Default<draw::QuadShape>());

        this->demoControl_.shapes.Append(
            draw::ShapeValue::Default<draw::EllipseShape>());
    }


    std::string GetAppName() const
    {
        return "Polygon Demo";
    }

    // Assign a random color to each shape as it is created.
    void OnShapeAdded_(size_t count)
    {
        if (count == 0)
        {
            // List is empty.
            return;
        }

        auto &shapeControl = this->demoControl_.shapes.at(count - 1);

        auto &lookControl = shapeControl.GetVirtual()->GetLook();

        auto defer = pex::MakeDefer(lookControl);

        auto hue = this->hueGenerator_.MakeHue();

        defer.strokeEnable.Set(true);
        defer.strokeColor.hue.Set(hue);
        defer.strokeColor.saturation.Set(1.0);
        defer.strokeColor.value.Set(1.0);
        defer.strokeColor.alpha.Set(1.0);

        defer.fillEnable.Set(true);
        defer.fillColor.hue.Set(hue);
        defer.fillColor.saturation.Set(0.75);
        defer.fillColor.value.Set(0.50);
        defer.fillColor.alpha.Set(1.0);
    }

private:
    HueGenerator hueGenerator_;
    draw::PolygonBrain polygonBrain_;

    using CountEndpoint = pex::Endpoint<DemoBrain, pex::control::ListCount>;
    CountEndpoint countEndpoint_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
