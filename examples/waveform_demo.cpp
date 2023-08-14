#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <tau/color_maps/gray.h>
#include <wxpex/app.h>
#include <wxpex/file_field.h>
#include <wxpex/border_sizer.h>

#include <draw/pixels.h>
#include <draw/waveform_settings.h>
#include <draw/waveform_generator.h>
#include <draw/views/waveform_settings_view.h>
#include <draw/views/waveform_view.h>

#include "common/about_window.h"
#include "common/observer.h"
#include "common/brain.h"


class DemoMainFrame: public wxFrame
{
public:
    DemoMainFrame(
        UserControl userControl,
        draw::WaveformControl control)
        :
        wxFrame(nullptr, wxID_ANY, "Waveform Demo"),
        shortcuts_(
            std::make_unique<wxpex::MenuShortcuts>(
                wxpex::Window(this),
                MakeShortcuts(userControl)))
    {
        this->SetMenuBar(this->shortcuts_->GetMenuBar());

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

private:
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    static constexpr size_t valueCount = 766;

    DemoBrain()
        :
        Brain<DemoBrain>(),
        observer_(this, UserControl(this->user_)),
        waveformModel_(),
        waveformPixelViewModel_(),

        waveformEndpoint_(
            this,
            draw::WaveformControl(this->waveformModel_),
            &DemoBrain::OnSettings_),

        waveformViewSettingsEndpoint_(
            this,
            draw::ViewSettingsControl(
                this->waveformPixelViewModel_.viewSettings),
            &DemoBrain::OnWaveformViewSettings_),

        pngIsLoaded_(false),
        pngData_(),

        waveformGenerator_(
            draw::WaveformControl(this->waveformModel_),
            draw::PixelViewControl(this->waveformPixelViewModel_)),

        colorMap_(
            tau::gray::MakeRgb8(valueCount),
            0,
            static_cast<int32_t>(valueCount - 1)),

        doCreateWaveformView_([this](){this->CreateWaveformView_();})

    {

    }

    void LoadPng(const draw::Png<int32_t> &png)
    {
        this->pngIsLoaded_ = false;
        this->pngData_ = png.GetValue(1);
        this->waveformPixelViewModel_.viewSettings.imageSize.Set(png.GetSize());
        this->waveformModel_.maximumValue.Set(valueCount - 1);
        this->waveformModel_.levelCount.SetMaximum(valueCount);
        this->doCreateWaveformView_();
        this->pngIsLoaded_ = true;
    }

    wxpex::Window CreateControlFrame()
    {
        auto window = wxpex::Window(
            new DemoMainFrame(
                this->GetUserControls(),
                draw::WaveformControl(this->waveformModel_)));

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
        wxAboutBox(MakeAboutDialogInfo("Waveform Demo"));
    }

    std::shared_ptr<draw::Pixels>
    MakePixels() const
    {
        auto size = this->waveformPixelViewModel_.viewSettings.imageSize.Get();

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
        this->user_.pixelView.pixels.Set(this->MakePixels());
    }

    void Shutdown()
    {
        this->waveformGenerator_.Shutdown();
        this->waveformView_.Close();
        Brain<DemoBrain>::Shutdown();
    }

private:
    void CreateWaveformView_()
    {
        if (!this->waveformView_)
        {
            this->waveformView_ = {
                new draw::WaveformView(
                    nullptr,
                    draw::PixelViewControl(this->waveformPixelViewModel_),
                    draw::WaveformControl(this->waveformModel_),
                    "Waveform"),
                MakeShortcuts(this->GetUserControls())};

            this->waveformView_.Get()->Show();
        }
    }

    void OnSettings_(const draw::WaveformSettings &)
    {
        if (this->pngIsLoaded_)
        {
            this->Display();
        }
    }

    void OnWaveformViewSettings_(const draw::ViewSettings &)
    {
        if (this->pngIsLoaded_)
        {
            this->Display();
        }
    }

private:
    Observer<DemoBrain> observer_;
    draw::WaveformModel waveformModel_;
    draw::PixelViewModel waveformPixelViewModel_;
    pex::Endpoint<DemoBrain, draw::WaveformControl> waveformEndpoint_;
    pex::Endpoint<DemoBrain, draw::ViewSettingsControl>
        waveformViewSettingsEndpoint_;
    bool pngIsLoaded_;
    draw::DataMatrix pngData_;

    draw::WaveformGenerator waveformGenerator_;

    tau::LimitedColorMap<draw::PixelMatrix, int32_t> colorMap_;
    wxpex::CallAfter doCreateWaveformView_;
    wxpex::ShortcutWindow waveformView_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
