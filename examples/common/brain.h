#pragma once


#include <wxpex/window.h>
#include <wxpex/splitter.h>

#include <draw/views/pixel_view.h>
#include <draw/png.h>
#include "user.h"
#include "display_error.h"


using Pixel = uint32_t;
using PngPixel = uint8_t;
static constexpr size_t valueCount = 256;



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
        // Open PNG file, and read data into Eigen matrix.
        // Display pixel view.
        draw::GrayPng<PngPixel> png(this->user_.fileName.Get());

        this->GetDerived()->LoadPng(png);

        this->user_.pixelView.viewSettings.imageSize.Set(png.GetSize());
        this->user_.pixelView.viewSettings.FitZoom();

        this->GetDerived()->Display();
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

protected:
    UserModel user_;
    UserControl userControl_;
    wxpex::UnclosedFrame applicationFrame_;
    std::unique_ptr<wxpex::MenuShortcuts> shortcuts_;
};
