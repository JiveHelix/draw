#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
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


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::polygon, "polygon"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<pex::MakeGroup<draw::PolygonShapeGroup>> polygon;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;


class DemoMainFrame: public wxFrame
{
public:
    DemoMainFrame(
        UserControl userControl,
        DemoControl control)
        :
        wxFrame(nullptr, wxID_ANY, "Shapes Demo"),
        shortcuts_(
            std::make_unique<wxpex::MenuShortcuts>(
                wxpex::Window(this),
                MakeShortcuts(userControl)))
    {
        this->SetMenuBar(this->shortcuts_->GetMenuBar());

        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        auto polygonShapeView =
            new draw::PolygonShapeView(
                this,
                "Polygon Shape",
                control.polygon,
                layoutOptions);

        polygonShapeView->Expand();
        auto topSizer = wxpex::BorderSizer(polygonShapeView, 5);
        this->SetSizerAndFit(topSizer.release());
    }

private:
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        shapesId_(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),

        polygonEndpoint_(
            this,
            this->demoControl_.polygon,
            &DemoBrain::OnPolygon_),

        polygonBrain_(
            this->demoControl_.polygon.polygon,
            this->userControl_.pixelView)
    {
        this->demoControl_.polygon.look.fillEnable.Set(true);
        this->demoControl_.polygon.look.fillColor.saturation.Set(1);
    }

    wxpex::Window CreateControlFrame()
    {
        this->userControl_.pixelView.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        auto window = wxpex::Window(new DemoMainFrame(
            this->GetUserControls(),
            this->demoControl_));

        return window;
    }

    void SaveSettings() const
    {
        std::cout << "TODO: Persist the processing settings." << std::endl;
    }

    void LoadSettings()
    {
        std::cout << "TODO: Restore the processing settings." << std::endl;
    }

    void ShowAbout()
    {
        wxAboutBox(MakeAboutDialogInfo("Polygon Demo"));
    }

    void Display()
    {
        auto shapes = draw::Shapes(this->shapesId_.Get());
        shapes.EmplaceBack<draw::PolygonShape>(this->demoModel_.polygon.Get());
        this->userControl_.pixelView.asyncShapes.Set(shapes);
    }

    void Shutdown()
    {
        Brain<DemoBrain>::Shutdown();
    }

    void LoadPng(const draw::Png<int32_t> &)
    {

    }

    void CreatePixelView_()
    {
        this->pixelView_ = {
            new draw::PixelView(
                nullptr,
                this->userControl_.pixelView,
                "Polygon View"),
            MakeShortcuts(this->GetUserControls())};

        this->pixelView_.Get()->Show();
    }

private:
    void OnPolygon_(const draw::PolygonShape &)
    {
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;
    pex::Endpoint<DemoBrain, draw::PolygonShapeControl> polygonEndpoint_;
    draw::PolygonBrain polygonBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
