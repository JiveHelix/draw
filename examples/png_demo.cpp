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
#include <draw/shape_creator.h>
#include <draw/views/cross_shape_view.h>


#include "common/about_window.h"
#include "common/observer.h"
#include "common/shape_demo_brain.h"


class DemoBrain: public ShapeDemoBrain<DemoBrain>
{
public:
    using Base = ShapeDemoBrain<DemoBrain>;

    DemoBrain()
        :
        Base(),

        crossBrain_(
            this->demoControl_.shapes,
            this->userControl_.pixelView.canvas),

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
        auto controls = new wxPanel(parent, wxID_ANY);

        wxpex::FileDialogOptions options{};
        options.message = "Choose a PNG file";
        options.wildcard = "*.png";

        auto fileSelector = new wxpex::FileField(
            controls,
            this->GetUserControls().fileName,
            options);

        auto shapeControls = this->Base::CreateControls(controls);

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            fileSelector,
            shapeControls);

        controls->SetSizerAndFit(sizer.release());

        return controls;
    }

    std::shared_ptr<draw::Pixels> MakePixels() const
    {
        auto result = draw::Pixels::CreateShared(this->pngData_.GetSize());
        this->colorMap_(this->pngData_.GetValues(), &result->data);

        return result;
    }

    void Display()
    {
        this->Base::Display();

        if (!this->pngIsLoaded_)
        {
            return;
        }

        this->user_.pixelView.pixels.Set(this->MakePixels());
    }

private:
    draw::CrossCreatorBrain crossBrain_;
    bool pngIsLoaded_;
    draw::GrayPng<PngPixel> pngData_;
    tau::LimitedColorMap<draw::PixelMatrix, int32_t> colorMap_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
