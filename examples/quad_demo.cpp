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


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::quad, "quad"),
        fields::Field(&T::pixelView, "pixelView"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<pex::MakeGroup<draw::QuadShapeGroup>> quad;
    T<draw::PixelViewGroupMaker> pixelView;
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
                wxpex::UnclosedWindow(this),
                MakeShortcuts(userControl)))
    {
        this->SetMenuBar(this->shortcuts_->GetMenuBar());

        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        auto quadShapeView =
            new draw::QuadShapeView(
                this,
                "Quad Shape",
                control.quad,
                layoutOptions);

        quadShapeView->Expand();
        auto topSizer = wxpex::BorderSizer(quadShapeView, 5);
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
        Brain<DemoBrain>(),
        shapesId_(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),
        quadEndpoint_(this, this->demoControl_.quad, &DemoBrain::OnQuad_),
        quadBrain_(
            this->demoControl_.quad.quad,
            this->demoControl_.pixelView)
    {
        this->demoControl_.quad.look.fillEnable.Set(true);
        this->demoControl_.quad.look.fillColor.saturation.Set(1);
    }

    wxpex::Window CreateControlFrame()
    {
        this->userControl_.pixelView.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        auto window = wxpex::Window(new DemoMainFrame(
            this->GetUserControls(),
            DemoControl(this->demoModel_)));

        return window;
    }

    void SaveSettings() const
    {
        std::cout << "TODO: Persist the settings." << std::endl;
    }

    void LoadSettings()
    {
        std::cout << "TODO: Restore the settings." << std::endl;
    }

    void ShowAbout()
    {
        wxAboutBox(MakeAboutDialogInfo("Quad Demo"));
    }

    void Display()
    {
        auto shapes = draw::Shapes(this->shapesId_.Get());
        shapes.EmplaceBack<draw::QuadShape>(this->demoModel_.quad.Get());
        this->demoControl_.pixelView.asyncShapes.Set(shapes);
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
            new draw::PixelFrame(
                this->demoControl_.pixelView,
                "Quad"),
            MakeShortcuts(this->GetUserControls())};

        this->pixelView_.Get()->Show();
    }

private:
    void OnQuad_(const draw::QuadShape &)
    {
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;
    pex::Endpoint<DemoBrain, draw::QuadShapeControl> quadEndpoint_;
    draw::QuadBrain quadBrain_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
