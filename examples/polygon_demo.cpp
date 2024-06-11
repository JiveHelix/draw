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
#include <draw/views/pixel_view_settings.h>
#include <draw/views/pixel_view.h>
#include <draw/polygon_brain.h>
#include <draw/shapes.h>
#include <draw/shape_list.h>
#include <draw/views/shape_list_view.h>

#include "common/observer.h"
#include "common/about_window.h"
#include "common/brain.h"


using ShapeValue = pex::poly::Value<draw::Shape, draw::PolygonShapeTemplate>;
using PolygonShapePolyGroup = draw::PolygonShapePolyGroup<ShapeValue>;
using PolygonShapeValue = typename PolygonShapePolyGroup::PolyValue;

using ListMaker = pex::MakePolyList<ShapeValue, draw::ShapeTemplates>;

using DemoModel = typename draw::ShapeListGroup<ListMaker>::Model;
using DemoControl = typename draw::ShapeListGroup<ListMaker>::Control;
using ShapesControl = decltype(DemoControl::shapes);


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
        this->demoControl_.shapes.Append(PolygonShapeValue::Default());
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        this->userControl_.pixelView.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        return new draw::ShapeListView<ListMaker>(parent, this->demoControl_);
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

        for (auto &shapeControl: this->demoControl_.shapes)
        {
            const auto &shapeValue = shapeControl.Get();
            shapes.Append(*shapeValue.GetValueBase());
        }

        this->userControl_.pixelView.asyncShapes.Set(shapes);
    }

    void LoadPng(const draw::GrayPng<PngPixel> &)
    {

    }

private:
    void OnPolygons_(const typename ShapesControl::Type &)
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
            ShapesControl
        >;

    PolygonsEndpoint polygonsEndpoint_;

    using PolygonBrain = draw::ShapeBrain
    <
        ShapesControl,
        draw::DragCreatePolygon
        <
            ShapesControl,
            PolygonShapeValue
        >
    >;

    PolygonBrain polygonBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
