#include <iostream>
#include <tau/color_map.h>
#include <tau/color_maps/gray.h>
#include <wxpex/app.h>
#include <wxpex/file_field.h>
#include <wxpex/scrolled.h>

#include <draw/pixels.h>
#include <draw/png.h>
#include <draw/cross_shape.h>
#include <draw/shapes.h>
#include <draw/views/cross_shape_view.h>


#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"


class DemoControls: public wxpex::Scrolled
{
public:
    DemoControls(
        wxWindow *parent,
        UserControl userControl,
        draw::CrossShapeControl crossShapeControl)
        :
        wxpex::Scrolled(parent)
    {
        wxpex::FileDialogOptions options{};
        options.message = "Choose a PNG file";
        options.wildcard = "*.png";

        auto fileSelector = new wxpex::FileField(
            this,
            userControl.fileName,
            options);

        auto crossShapeView =
            new draw::CrossShapeView(
                this,
                "Target",
                crossShapeControl,
                wxpex::LayoutOptions{});

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(fileSelector, 0, wxEXPAND | wxALL, 5);
        sizer->Add(crossShapeView, 0, wxEXPAND | wxALL, 5);

        this->ConfigureSizer(
            wxpex::verticalScrolled,
            std::move(sizer));
    }
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        Brain<DemoBrain>(),
        shapesId_(),
        observer_(this, UserControl(this->user_)),
        crossShapeModel_(),
        crossShapeEndpoint_(
            this,
            this->crossShapeModel_,
            &DemoBrain::OnCrossShape_),

        pngIsLoaded_(false),
        pngData_(),

        colorMap_(
            tau::gray::MakeRgb8(valueCount),
            0,
            static_cast<int32_t>(valueCount - 1))
    {

    }

    std::string GetAppName() const
    {
        return "PNG Demo";
    }

    void LoadPng(const draw::GrayPng<PngPixel> &png)
    {
        this->pngIsLoaded_ = false;
        this->pngData_ = png;
        this->pngIsLoaded_ = true;
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        return new DemoControls(
            parent,
            this->GetUserControls(),
            this->crossShapeModel_);
    }

    void ShowAbout()
    {
        wxAboutBox(MakeAboutDialogInfo("PNG Demo"));
    }

    std::shared_ptr<draw::Pixels>
    MakePixels() const
    {
        auto result = draw::Pixels::CreateShared(this->pngData_.GetSize());
        this->colorMap_(this->pngData_.GetValues(), &result->data);

        return result;
    }

    void Display()
    {
        if (!this->pngIsLoaded_)
        {
            return;
        }

        this->user_.pixelView.pixels.Set(this->MakePixels());
        auto shapes = draw::Shapes(this->shapesId_.Get());

        shapes.Append(
            std::make_shared<draw::CrossShape>(this->crossShapeModel_.Get()));

        this->userControl_.pixelView.asyncShapes.Set(shapes);
    }

protected:
    void OnCrossShape_(const draw::CrossShape &)
    {
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    draw::CrossShapeModel crossShapeModel_;
    pex::Endpoint<DemoBrain, draw::CrossShapeControl> crossShapeEndpoint_;


    bool pngIsLoaded_;
    draw::GrayPng<PngPixel> pngData_;

    tau::LimitedColorMap<draw::PixelMatrix, int32_t> colorMap_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
