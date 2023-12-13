#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <wxpex/app.h>
#include <wxpex/check_box.h>
#include <wxpex/border_sizer.h>

#include <draw/pixels.h>
#include <draw/quad_shape.h>
#include <draw/quad_brain.h>
#include <draw/shapes.h>

#include <draw/views/quad_shape_view.h>
#include <draw/views/pixel_view_settings.h>
#include <draw/views/pixel_view.h>

#include "common/observer.h"
#include "common/about_window.h"
#include "common/brain.h"
#include "shapes_interface.h"


using ListMaker = draw::QuadListMaker;
using DemoModel = typename DemoGroup<ListMaker>::Model;
using DemoControl = typename DemoGroup<ListMaker>::Control;


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        Brain<DemoBrain>(),
        shapesId_(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),
        quadsEndpoint_(this, this->demoControl_.shapes, &DemoBrain::OnQuads_),
        quadBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView)
    {
        for (auto &quad: this->demoControl_.shapes)
        {
            quad.look.fillEnable.Set(true);
            quad.look.fillColor.saturation.Set(1);
        }
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        this->userControl_.pixelView.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        return new DemoInterface<draw::QuadShapeView, DemoControl>(
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
        return "Quad Demo";
    }

    void Display()
    {
        auto shapes = draw::Shapes(this->shapesId_.Get());

        for (auto &shape: this->demoControl_.shapes)
        {
            shapes.EmplaceBack<draw::QuadShape>(shape.Get());
        }

        this->userControl_.pixelView.asyncShapes.Set(shapes);
    }

    void LoadPng(const draw::GrayPng<PngPixel> &)
    {

    }

private:
    void OnQuads_(const std::vector<draw::QuadShape> &)
    {
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;

    using QuadsEndpoint =
        pex::Endpoint<
            DemoBrain,
            decltype(DemoControl::shapes)
        >;

    QuadsEndpoint quadsEndpoint_;
    draw::QuadBrain quadBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
