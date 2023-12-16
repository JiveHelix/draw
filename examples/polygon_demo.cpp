// #define USE_OBSERVER_NAME
// #define ENABLE_PEX_LOG


#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include <fmt/core.h>
#include <pex/list.h>
#include <wxpex/app.h>
#include <wxpex/check_box.h>
#include <wxpex/border_sizer.h>

#include <draw/pixels.h>
#include <draw/polygon_shape.h>
#include <draw/views/polygon_shape_view.h>
#include <draw/views/pixel_view_settings.h>
#include <draw/views/pixel_view.h>
#include <draw/polygon_brain.h>
#include <draw/shapes.h>

#include "common/observer.h"
#include "common/about_window.h"
#include "common/brain.h"
#include "shapes_interface.h"


using ListMaker = draw::PolygonListMaker;
using DemoModel = typename DemoGroup<ListMaker>::Model;
using DemoControl = typename DemoGroup<ListMaker>::Control;


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        shapesId_(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),

        polygonsEndpoint_(
            this,
            this->demoControl_.shapes,
            &DemoBrain::OnPolygons_),

        polygonBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView)
    {
        for (auto &polygon: this->demoControl_.shapes)
        {
            polygon.look.fillEnable.Set(true);
            polygon.look.fillColor.saturation.Set(1);
        }
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        this->userControl_.pixelView.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        return new DemoInterface<draw::PolygonShapeView, DemoControl>(
            parent,
            this->demoControl_);
    }

    void SaveSettings() const
    {
        std::cout << "TODO: Persist the settings." << std::endl;
    }

    void LoadSettings()
    {
        std::cout << "TODO: Restore the settings." << std::endl;
    }

    std::string GetAppName() const
    {
        return "Polygon Demo";
    }

    void Display()
    {
        auto shapes = draw::Shapes(this->shapesId_.Get());

        for (auto &shape: this->demoControl_.shapes)
        {
            shapes.EmplaceBack<draw::PolygonShape>(shape.Get());
        }

        this->userControl_.pixelView.asyncShapes.Set(shapes);
    }

    void LoadPng(const draw::GrayPng<PngPixel> &)
    {

    }

private:
    void OnPolygons_(const std::vector<draw::PolygonShape> &)
    {
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;

    using PolygonsEndpoint =
        pex::Endpoint<
            DemoBrain,
            decltype(DemoControl::shapes)
        >;

    PolygonsEndpoint polygonsEndpoint_;
    draw::PolygonBrain polygonBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
