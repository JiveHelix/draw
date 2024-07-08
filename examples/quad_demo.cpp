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



using ShapeControlBase = typename draw::Shape::ControlBase;

static_assert(std::is_base_of_v<ShapeControlBase, draw::QuadShapeControl>);

static_assert(
    std::is_base_of_v
    <
        typename draw::QuadShapeTemplates::ControlMembers,
        draw::QuadShapeControl
    >);


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        shapesId_(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),

        shapesEndpoint_(this, this->demoControl_.shapes, &DemoBrain::OnQuads_),

        quadBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView)
    {
        this->demoControl_.shapes.Append(draw::QuadShapeValue::Default());
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        this->userControl_.pixelView.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        return new draw::ShapeListView(parent, this->demoControl_);
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
    void OnQuads_(const typename draw::ShapesControl::Type &)
    {
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    draw::ShapeListModel demoModel_;
    draw::ShapeListControl demoControl_;

    using ShapesEndpoint = draw::ShapesEndpoint<DemoBrain>;
    ShapesEndpoint shapesEndpoint_;

    draw::QuadBrain quadBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
