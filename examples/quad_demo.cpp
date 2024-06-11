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
#include <draw/shape_list.h>
#include <draw/views/shape_list_view.h>
#include <draw/views/pixel_view_settings.h>
#include <draw/views/pixel_view.h>

#include "common/observer.h"
#include "common/about_window.h"
#include "common/brain.h"


using ShapeValue = pex::poly::Value<draw::Shape, draw::QuadShapeTemplate>;
using QuadShapePolyGroup = draw::QuadShapePolyGroup<ShapeValue>;
using QuadShapeValue = typename QuadShapePolyGroup::PolyValue;
using QuadShapeModel = typename QuadShapePolyGroup::Model;
using QuadShapeControl = typename QuadShapePolyGroup::Control;

using ShapeControlUserBase = typename draw::Shape::ControlUserBase;
using QuadControlMembers = typename QuadShapePolyGroup::ControlMembers;

static_assert(std::is_base_of_v<ShapeControlUserBase, QuadShapeControl>);
static_assert(std::is_base_of_v<QuadControlMembers, QuadShapeControl>);

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

        quadsEndpoint_(this, this->demoControl_.shapes, &DemoBrain::OnQuads_),

        quadBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView)
    {
        this->demoControl_.shapes.Append(QuadShapeValue::Default());
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
        return "Quad Demo";
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
    void OnQuads_(const typename ShapesControl::Type &)
    {
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;

    using QuadsEndpoint =
        pex::Endpoint<DemoBrain, ShapesControl>;

    QuadsEndpoint quadsEndpoint_;

    using QuadBrain = draw::ShapeBrain
    <
        ShapesControl,
        draw::DragCreateQuad<ShapesControl, QuadShapeValue>
    >;

    QuadBrain quadBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
