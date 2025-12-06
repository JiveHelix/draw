#pragma once


#include <wxpex/button.h>
#include <wxpex/check_box.h>
#include "draw/views/canvas.h"
#include "draw/views/canvas_settings.h"


namespace draw
{


struct CanvasViewOptions
{
    bool useHorizontalZoom;
    bool useVerticalZoom;
    bool displayButtons;
    bool displayLinkZoom;

    CanvasViewOptions()
        :
        useHorizontalZoom(true),
        useVerticalZoom(false),
        displayButtons(true),
        displayLinkZoom(false)
    {

    }

    CanvasViewOptions & SetUseHorizontalZoom(bool value)
    {
        this->useHorizontalZoom = value;

        return *this;
    }

    CanvasViewOptions & SetUseVerticalZoom(bool value)
    {
        this->useVerticalZoom = value;

        return *this;
    }

    CanvasViewOptions & SetDisplayButtons(bool value)
    {
        this->displayButtons = value;

        return *this;
    }

    CanvasViewOptions & SetDisplayLinkZoom(bool value)
    {
        this->displayLinkZoom = value;

        return *this;
    }

    bool IsDualZoom() const
    {
        return this->useHorizontalZoom && this->useVerticalZoom;
    }

    bool Any() const
    {
        return this->useHorizontalZoom
            || this->useVerticalZoom
            || this->displayButtons;
    }
};


class CanvasControls
{
public:
    static constexpr int margin = 3;
    static constexpr int gridSpacing = 3;

    CanvasControls(
        wxWindow *window,
        wxBoxSizer *sizer,
        const CanvasControl &control,
        const CanvasViewOptions &options)
        :
        options_(options),
        horizontalZoom_(nullptr),
        verticalZoom_(nullptr),
        controlsSizer_(nullptr)
    {
        if (this->options_.useHorizontalZoom || this->options_.displayButtons)
        {
            // We need a sizer to hold a slider and or buttons.
            this->controlsSizer_ = new wxBoxSizer(wxHORIZONTAL);
        }

        if (this->options_.useHorizontalZoom)
        {
            this->horizontalZoom_ =
                new ScaleSlider(
                    window,
                    control.viewSettings.scale.horizontal,
                    control.viewSettings.scale.horizontal.value,
                    wxpex::Style::horizontal);

            assert(this->controlsSizer_);
            this->controlsSizer_->Add(this->horizontalZoom_, 0, wxRIGHT, 3);
        }

        if (this->options_.useVerticalZoom)
        {
            this->verticalZoom_ =
                new ScaleSlider(
                    window,
                    control.viewSettings.scale.vertical,
                    control.viewSettings.scale.vertical.value,
                    wxpex::Style::vertical);
        }

        if (this->options_.displayButtons)
        {
            auto resetZoom =
                new wxpex::Button(
                    window,
                    "Reset",
                    control.viewSettings.resetZoom);

            auto fitZoom =
                new wxpex::Button(window, "Fit", control.viewSettings.fitZoom);

            auto recenter =
                new wxpex::Button(
                    window,
                    "Recenter",
                    control.viewSettings.recenter);

            this->controlsSizer_->Add(resetZoom, 0, wxRIGHT, 3);
            this->controlsSizer_->Add(fitZoom, 0, wxRIGHT, 3);
            this->controlsSizer_->Add(recenter, 0, wxRIGHT, 3);
        }

        wxSizer *innerSizer;

        if (this->options_.displayLinkZoom)
        {
            auto linkZoom =
                new wxpex::CheckBox(
                    window,
                    "Link",
                    control.viewSettings.linkZoom);

            this->controlsSizer_->Add(linkZoom, 0, wxRIGHT);
        }

        if (this->options_.IsDualZoom())
        {
            this->gridSizer_ =
                new wxFlexGridSizer(2, gridSpacing, gridSpacing);

            this->gridSizer_->SetFlexibleDirection(wxBOTH);
            this->gridSizer_->AddGrowableRow(1, 1);
            this->gridSizer_->AddGrowableCol(1, 1);

            this->gridSizer_->AddSpacer(0);
            this->gridSizer_->Add(this->controlsSizer_, 0);
            this->gridSizer_->Add(this->verticalZoom_, 1);

            innerSizer = this->gridSizer_;
        }
        else if (this->options_.useVerticalZoom)
        {
            this->boxSizer_ = new wxBoxSizer(wxHORIZONTAL);

            this->boxSizer_->Add(
                this->verticalZoom_,
                0,
                wxEXPAND,
                gridSpacing);

            if (this->options_.displayButtons)
            {
                // The buttons are in the controlsSizer

                innerSizer = new wxBoxSizer(wxVERTICAL);

                innerSizer->Add(
                    this->controlsSizer_,
                    0,
                    wxBOTTOM,
                    gridSpacing);

                innerSizer->Add(
                    this->boxSizer_,
                    1,
                    wxEXPAND | wxBOTTOM,
                    gridSpacing);
            }
            else
            {
                innerSizer = this->boxSizer_;
            }

        }
        else
        {
            this->boxSizer_ = new wxBoxSizer(wxVERTICAL);

            this->boxSizer_->Add(
                this->controlsSizer_,
                0,
                wxBOTTOM,
                gridSpacing);

            innerSizer = this->boxSizer_;
        }

        sizer->Add(innerSizer, 1, wxALL | wxEXPAND, margin);
    }

    void AddView(wxWindow *view)
    {
        if (this->options_.IsDualZoom())
        {
            assert(this->gridSizer_);

            this->gridSizer_->Add(view, 1, wxEXPAND);
        }
        else
        {
            assert(this->boxSizer_);

            this->boxSizer_->Add(
                view,
                1,
                wxEXPAND);
        }
    }

    Size GetWindowSize(const Size &canvasSize) const
    {
        // Recalculate window size based on canvas size.
        Size size;

        assert(
            this->controlsSizer_ != nullptr || this->options_.useVerticalZoom);

        if (this->controlsSizer_)
        {
            auto controlsSize =
                wxpex::ToSize<int>(this->controlsSizer_->GetMinSize());

            if (this->options_.useVerticalZoom)
            {
                auto verticalZoomSize =
                    wxpex::ToSize<int>(this->verticalZoom_->GetBestSize());

                size = Size(
                    std::max(canvasSize.width, controlsSize.width)
                        + verticalZoomSize.width,
                    std::max(canvasSize.height, verticalZoomSize.height)
                        + controlsSize.height);
            }
            else
            {
                size = Size(
                    std::max(canvasSize.width, controlsSize.width),
                    canvasSize.height + controlsSize.height);
            }
        }
        else if (this->options_.useVerticalZoom)
        {
            auto verticalZoomSize =
                wxpex::ToSize<int>(this->verticalZoom_->GetBestSize());

            size = Size(
                verticalZoomSize.width + canvasSize.width,
                std::max(canvasSize.height, verticalZoomSize.height));
        }
        else
        {
            size = canvasSize;
        }

        auto scrollbarPadding = 20;
        auto extra = gridSpacing + (margin * 2) + scrollbarPadding;
        size += extra;

        return size;
    }

private:
    CanvasViewOptions options_;
    wxWindow *horizontalZoom_;
    wxWindow *verticalZoom_;
    wxBoxSizer *controlsSizer_;
    wxFlexGridSizer *gridSizer_;
    wxBoxSizer *boxSizer_;
};



template<typename DerivedCanvas>
class CanvasView: public wxPanel
{
    static constexpr int margin = 3;

public:
    static constexpr auto observerName = "CanvasView";

    template<typename ViewControl>
    CanvasView(
        wxWindow *parent,
        CanvasControl control,
        ViewControl viewControl,
        CanvasViewOptions options)
        :
        wxPanel(parent, wxID_ANY),
        options_(options),
        horizontalZoom_(nullptr),
        verticalZoom_(nullptr),
        controlsSizer_(nullptr),
        canvas_(new DerivedCanvas(this, viewControl))
    {
        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);

        if (this->options_.Any())
        {
            this->canvasControls_ =
                std::make_unique<CanvasControls>(
                    this,
                    sizer.get(),
                    control,
                    options);

            this->canvasControls_->AddView(this->canvas_);
        }
        else
        {
            sizer->Add(
                this->canvas_,
                1,
                wxALL | wxEXPAND,
                CanvasControls::margin);
        }

        this->SetSizer(sizer.release());
        this->SetMinSize(wxSize(100, 100));
    }

    wxSize DoGetBestSize() const override
    {
        assert(this->canvas_ != nullptr);

        auto result =
            this->GetWindowSize_(this->canvas_->GetVirtualSize());

        return wxpex::ToWxSize(result);
    }

    DerivedCanvas * GetCanvas()
    {
        return canvas_;
    }

    const DerivedCanvas * GetCanvas() const
    {
        return canvas_;
    }

protected:
    Size GetWindowSize_(const Size &canvasSize) const
    {
        // Recalculate window size based on canvas size.
        Size size;

        if (this->options_.Any())
        {
            assert(this->canvasControls_);
            size = this->canvasControls_->GetWindowSize(canvasSize);
        }
        else
        {
            size = Size(canvasSize.width, canvasSize.height);

            auto scrollbarPadding = 20;
            auto extra = (margin * 2) + scrollbarPadding;
            size += extra;
        }

        return size;
    }

private:
    CanvasViewOptions options_;
    std::unique_ptr<CanvasControls> canvasControls_;
    wxWindow *horizontalZoom_;
    wxWindow *verticalZoom_;
    wxBoxSizer *controlsSizer_;
    DerivedCanvas *canvas_;
};


template<typename DerivedView, typename ViewControl>
class CanvasFrame: public wxFrame
{
public:
    CanvasFrame(
        ViewControl control,
        const std::string &title,
        CanvasViewOptions options = CanvasViewOptions{})
        :
        wxFrame(nullptr, wxID_ANY, title)
    {
        auto view = new DerivedView(this, control, options);
        view->SetMinSize(wxSize(400, 225));
        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(view, 1, wxEXPAND);
        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace draw
