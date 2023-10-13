#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <wxpex/app.h>
#include <wxpex/check_box.h>
#include <wxpex/border_sizer.h>

#include <draw/pixels.h>
#include <draw/drag.h>
#include <draw/ellipse_shape.h>
#include <draw/views/ellipse_shape_view.h>

#include <draw/views/pixel_view_settings.h>
#include <draw/views/pixel_view.h>
#include <draw/shapes.h>

#include "common/observer.h"
#include "common/about_window.h"
#include "common/brain.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::ellipse, "ellipse"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<pex::MakeGroup<draw::EllipseShapeGroup>> ellipse;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;


class DemoControls: public wxPanel
{
public:
    DemoControls(
        wxWindow *parent,
        DemoControl control)
        :
        wxPanel(parent, wxID_ANY)
    {
        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        auto ellipseShapeView =
            new draw::EllipseShapeView(
                this,
                "Ellipse Shape",
                control.ellipse,
                layoutOptions);

        ellipseShapeView->Expand();

        auto topSizer = wxpex::BorderSizer(ellipseShapeView, 5);
        this->SetSizerAndFit(topSizer.release());
    }
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

        ellipseEndpoint_(
            this,
            this->demoControl_.ellipse,
            &DemoBrain::OnEllipse_),

        pixelViewEndpoint_(
            this,
            this->userControl_.pixelView),

        ellipseDrag_()
    {
        this->demoControl_.ellipse.look.strokeEnable.Set(true);

        this->pixelViewEndpoint_.mouseDown.Connect(&DemoBrain::OnMouseDown_);

        this->pixelViewEndpoint_.logicalPosition.Connect(
            &DemoBrain::OnLogicalPosition_);

        this->pixelViewEndpoint_.modifier.Connect(&DemoBrain::OnModifier_);

    }

    std::string GetAppName() const
    {
        return "Ellipse Demo";
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        this->userControl_.pixelView.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        return new DemoControls(parent, this->demoControl_);
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
        wxAboutBox(MakeAboutDialogInfo("Ellipse Demo"));
    }

    void Display()
    {
        auto shapes = draw::Shapes(this->shapesId_.Get());

        shapes.EmplaceBack<draw::EllipseShape>(
            this->demoModel_.ellipse.Get());

        this->userControl_.pixelView.asyncShapes.Set(shapes);
    }

    void Shutdown()
    {
        Brain<DemoBrain>::Shutdown();
    }

    void LoadPng(const draw::GrayPng<PngPixel> &)
    {

    }

private:
    void UpdateCursor_()
    {
        if (this->ellipseDrag_)
        {
            return;
        }

        auto ellipse = this->demoModel_.ellipse.ellipse.Get();
        auto position = this->user_.pixelView.logicalPosition.Get();

        if (ellipse.Contains(position.template Convert<double>()))
        {
            this->user_.pixelView.cursor.Set(wxpex::Cursor::openHand);
        }
        else if (this->user_.pixelView.cursor.Get() != wxpex::Cursor::cross)
        {
            this->user_.pixelView.cursor.Set(wxpex::Cursor::cross);
        }
    }

    void OnModifier_(const wxpex::Modifier &)
    {
        this->UpdateCursor_();
    }

    void OnLogicalPosition_(const tau::Point2d<int> &position)
    {
        this->UpdateCursor_();

        if (this->ellipseDrag_)
        {
            this->demoModel_.ellipse.ellipse.center.Set(
                this->ellipseDrag_->GetPosition(position));

            return;
        }
    }

    void OnMouseDown_(bool isDown)
    {
        if (!isDown)
        {
            this->ellipseDrag_.reset();
            this->UpdateCursor_();
            return;
        }

        auto point = this->user_.pixelView.logicalPosition.Get();

        if (this->user_.pixelView.modifier.Get().IsNone())
        {
            auto ellipse = this->demoModel_.ellipse.ellipse.Get();

            if (ellipse.Contains(point))
            {
                this->ellipseDrag_ = draw::Drag(point, ellipse.center);

                this->user_.pixelView.cursor.Set(
                    wxpex::Cursor::closedHand);

                return;
            }

            return;
        }
    }

    void OnEllipse_(const draw::EllipseShape &)
    {
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;
    pex::Endpoint<DemoBrain, draw::EllipseShapeControl> ellipseEndpoint_;
    pex::EndpointGroup<DemoBrain, draw::PixelViewControl> pixelViewEndpoint_;

    std::optional<draw::Drag> ellipseDrag_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
