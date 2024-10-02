#include <iostream>
#include <tau/color_map.h>
#include <tau/color_maps/gray.h>
#include <wxpex/app.h>
#include <wxpex/file_field.h>

#include <draw/pixels.h>
#include <draw/png.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"


class DemoControls: public wxPanel
{
public:
    DemoControls(
        wxWindow *parent,
        UserControl userControl)
        :
        wxPanel(parent)
    {
        wxpex::FileDialogOptions options{};
        options.message = "Choose a PNG file";
        options.wildcard = "*.png";

        auto fileSelector = new wxpex::FileField(
            this,
            userControl.fileName,
            options);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(fileSelector, 0, wxEXPAND | wxALL, 5);
        this->SetSizerAndFit(sizer.release());
    }
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        Brain<DemoBrain>(),
        observer_(this, UserControl(this->user_)),

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
            this->GetUserControls());
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
    }

private:
    Observer<DemoBrain> observer_;

    bool pngIsLoaded_;
    draw::GrayPng<PngPixel> pngData_;

    tau::LimitedColorMap<draw::PixelMatrix, int32_t> colorMap_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
