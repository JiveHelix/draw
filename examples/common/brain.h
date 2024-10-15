#pragma once


#include <wxpex/window.h>
#include <wxpex/splitter.h>
#include <wxpex/file_field.h>

#include <draw/views/pixel_view.h>
#include <draw/png.h>
#include "user.h"
#include "about_window.h"
#include "display_error.h"


using Pixel = uint32_t;
using PngPixel = uint8_t;
static constexpr size_t valueCount = 256;


template<typename T>
concept HasLoadPng = requires(T t)
{
    { t.LoadPng(std::declval<draw::GrayPng<PngPixel>>()) };
};


template<typename Derived>
class Brain
{
public:
    Brain()
        :
        user_{},
        userControl_(this->user_),
        applicationFrame_(new wxFrame(nullptr, wxID_ANY, "")),
        shortcuts_(
            std::make_unique<wxpex::MenuShortcuts>(
                wxpex::UnclosedWindow(this->applicationFrame_.Get()),
                MakeShortcuts(this->userControl_)))
    {
        this->applicationFrame_.Get()->SetMenuBar(
            this->shortcuts_->GetMenuBar());
    }

    Derived * GetDerived()
    {
        return static_cast<Derived *>(this);
    }

    const Derived * GetDerived() const
    {
        return static_cast<const Derived *>(this);
    }

    UserControl GetUserControls()
    {
        return UserControl(this->user_);
    }

    void CreateFrame()
    {
        auto splitter = new wxpex::Splitter(this->applicationFrame_.Get());
        auto controls = this->GetDerived()->CreateControls(splitter);
        auto pixelView = this->GetDerived()->CreatePixelView(splitter);

        splitter->SplitVerticallyLeft(
            controls,
            pixelView);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(splitter, 1, wxEXPAND);
        this->applicationFrame_.Get()->SetSizerAndFit(sizer.release());

        this->applicationFrame_.Get()->SetTitle(
            this->GetDerived()->GetAppName());

        this->applicationFrame_.Get()->Maximize();
        this->GetDerived()->Display();
    }

    void OpenFile()
    {
        if constexpr (HasLoadPng<Derived>)
        {
            auto fileName = this->user_.fileName.Get();

            if (!jive::path::IsFile(fileName))
            {
                auto [directory, file] = jive::path::Split(fileName);

                wxpex::FileDialogOptions options{};
                options.message = "Choose a PNG file";
                options.wildcard = "*.png";

                wxFileDialog openFile(
                    nullptr,
                    wxString(options.message),
                    wxString(directory),
                    wxString(file),
                    wxString(options.wildcard),
                    options.style);

                if (openFile.ShowModal() == wxID_CANCEL)
                {
                    return;
                }

                this->user_.fileName.Set(openFile.GetPath());

                return;
            }

            // Open PNG file, and read data into Eigen matrix.
            // Display pixel view.
            draw::GrayPng<PngPixel> png(this->user_.fileName.Get());

            this->GetDerived()->LoadPng(png);

            this->user_.pixelView.canvas.viewSettings.imageSize.Set(
                png.GetSize());

            this->user_.pixelView.canvas.viewSettings.FitZoom();

            this->GetDerived()->Display();
        }
    }

    void Shutdown()
    {
        this->applicationFrame_.Close();
    }

    wxWindow * CreatePixelView(wxWindow *parent)
    {
        return new draw::PixelView(
            parent,
            draw::PixelViewControl(this->user_.pixelView));
    }

    void ShowAbout() const
    {
        wxAboutBox(MakeAboutDialogInfo(this->GetDerived()->GetAppName()));
    }

protected:
    UserModel user_;
    UserControl userControl_;
    wxpex::UnclosedFrame applicationFrame_;
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};
