#include <wxpex/wxshim_app.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <tau/color_maps/gray.h>
#include <wxpex/app.h>
#include <wxpex/file_field.h>
#include <wxpex/border_sizer.h>
#include <wxpex/layout_items.h>

#include <draw/pixels.h>
#include <draw/waveform_settings.h>
#include <draw/waveform_generator.h>
#include <draw/views/waveform_settings_view.h>
#include <draw/views/waveform_view.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"


class DemoControls: public wxPanel
{
public:
    DemoControls(
        wxWindow *parent,
        const UserControl &userControl,
        const draw::WaveformControl &control)
        :
        wxPanel(parent, wxID_ANY)
    {
        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        wxpex::FileDialogOptions options{};
        options.message = "Choose a PNG file";
        options.wildcard = "*.png";

        auto fileSelector = new wxpex::FileField(
            this,
            userControl.fileName,
            options);

        auto waveformSettings = new draw::WaveformSettingsView(
            this,
            control,
            layoutOptions);

        waveformSettings->Expand();

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            fileSelector,
            waveformSettings);

        auto topSizer = wxpex::BorderSizer(std::move(sizer), 5);
        this->SetSizerAndFit(topSizer.release());
    }
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        Brain<DemoBrain>(),
        observer_(this, this->userControl_),
        waveformModel_(),
        waveformPixelModel_(),
        waveformPixelControl_(this->waveformPixelModel_),

        waveformGenerator_(
            draw::WaveformControl(this->waveformModel_),
            this->waveformPixelControl_),

        viewSettingsEndpoint_(
            this,
            this->waveformPixelControl_.canvas.viewSettings,
            &DemoBrain::OnViewSettings_),

        waveformSettingsEndpoint_(
            this,
            draw::WaveformControl(this->waveformModel_),
            &DemoBrain::OnWaveformSettings_),

        pngIsLoaded_(false),
        pngData_(),

        colorMap_(
            tau::gray::MakeRgb8(rgbValueCount),
            0,
            static_cast<int32_t>(rgbValueCount - 1))
    {
        this->waveformPixelModel_.canvas.viewSettings.linkZoom.Set(false);
        this->waveformPixelModel_.canvas.viewSettings.scale.vertical.Set(1.0);
    }

    ~DemoBrain()
    {
        this->Shutdown();
    }

    void LoadGrayPng(const draw::GrayPng<PngPixel> &png)
    {
        this->pngIsLoaded_ = false;
        this->pngData_ = png.GetValues().template cast<int32_t>();

        this->userControl_.pixelView.canvas.viewSettings.imageSize.Set(
            png.GetSize());

        this->waveformModel_.maximumValue.Set(rgbValueCount - 1);
        this->waveformModel_.levelCount.SetMaximum(rgbValueCount);

        this->waveformModel_.columnCount.SetMaximum(
            static_cast<size_t>(this->pngData_.cols()));

        this->waveformModel_.columnCount.Set(
            static_cast<size_t>(this->pngData_.cols()));

        this->pngIsLoaded_ = true;

        auto pixelViewSize = this->pixelView_->GetClientSize();
        this->pixelView_->SetSashPosition(pixelViewSize.GetHeight() / 2);
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        return new DemoControls(
            parent,
            this->userControl_,
            draw::WaveformControl(this->waveformModel_));
    }

    std::string GetAppName() const
    {
        return "Waveform Demo";
    }

    void ShowAbout()
    {
        wxAboutBox(MakeAboutDialogInfo("Waveform Demo"));
    }

    std::shared_ptr<draw::Pixels>
    MakePixels() const
    {
        auto size =
            this->userControl_.pixelView.canvas.viewSettings.imageSize.Get();

        auto result = draw::Pixels::CreateShared(size);

        this->colorMap_(this->pngData_, &result->data);

        return result;
    }

    void Display()
    {
        if (!this->pngIsLoaded_)
        {
            return;
        }

        this->waveformGenerator_(this->pngData_);

        // this->pngIsLoaded_ = false;
        this->user_.pixelView.pixels.Set(this->MakePixels());
        // this->pngIsLoaded_ = true;
    }

    void Shutdown()
    {
        this->waveformGenerator_.Shutdown();
        Brain<DemoBrain>::Shutdown();
    }

    wxWindow * CreatePixelView(wxWindow *parent)
    {
        return this->pixelView_ = new draw::PixelViewAndWaveform(
            parent,
            this->userControl_.pixelView,
            this->waveformPixelControl_,
            draw::WaveformControl(this->waveformModel_));
    }

private:
    void OnWaveformSettings_(const draw::WaveformSettings &)
    {
        if (this->pngIsLoaded_)
        {
            this->Display();
        }
    }

    void OnViewSettings_(const draw::ViewSettings &)
    {
        if (this->pngIsLoaded_)
        {
            this->Display();
        }
    }

private:
    Observer<DemoBrain> observer_;
    draw::WaveformModel waveformModel_;
    draw::PixelViewModel waveformPixelModel_;
    draw::PixelViewControl waveformPixelControl_;

    draw::WaveformGenerator waveformGenerator_;

    pex::Endpoint<DemoBrain, draw::ViewSettingsControl>
        viewSettingsEndpoint_;

    pex::Endpoint<DemoBrain, draw::WaveformControl> waveformSettingsEndpoint_;

    bool pngIsLoaded_;
    draw::DataMatrix pngData_;

    tau::LimitedColorMap<draw::PixelMatrix, int32_t> colorMap_;
    draw::PixelViewAndWaveform *pixelView_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
