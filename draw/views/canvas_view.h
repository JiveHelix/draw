#pragma once


#include <wxpex/button.h>
#include <wxpex/check_box.h>
#include "draw/views/canvas.h"
#include "draw/views/canvas_settings.h"


namespace draw
{


struct CanvasViewOptions
{
    bool useDualZoom;
    bool displayControls;

    CanvasViewOptions()
        :
        useDualZoom(false),
        displayControls(true)
    {

    }

    CanvasViewOptions & SetUseDualZoom(bool value)
    {
        this->useDualZoom = value;

        return *this;
    }

    CanvasViewOptions & SetDisplayControls(bool value)
    {
        this->displayControls = value;

        return *this;
    }
};


template<typename DerivedCanvas>
class CanvasView: public wxPanel
{
    static constexpr int margin = 3;
    static constexpr int gridSpacing = 3;

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

        if (this->options_.displayControls)
        {
            this->horizontalZoom_ =
                new ScaleSlider(
                    this,
                    control.viewSettings.scale.horizontal,
                    control.viewSettings.scale.horizontal.value,
                    wxpex::Style::horizontal);

            if (this->options_.useDualZoom)
            {
                this->verticalZoom_ =
                    new ScaleSlider(
                        this,
                        control.viewSettings.scale.vertical,
                        control.viewSettings.scale.vertical.value,
                        wxpex::Style::vertical);
            }

            this->controlsSizer_ = new wxBoxSizer(wxHORIZONTAL);

            auto resetZoom =
                new wxpex::Button(
                    this,
                    "Reset",
                    control.viewSettings.resetZoom);

            auto fitZoom =
                new wxpex::Button(this, "Fit", control.viewSettings.fitZoom);

            auto recenter =
                new wxpex::Button(this, "Recenter", control.viewSettings.recenter);

            this->controlsSizer_->Add(this->horizontalZoom_, 0, wxRIGHT, 3);
            this->controlsSizer_->Add(resetZoom, 0, wxRIGHT, 3);
            this->controlsSizer_->Add(fitZoom, 0, wxRIGHT, 3);
            this->controlsSizer_->Add(recenter, 0, wxRIGHT, 3);

            wxSizer *innerSizer;

            if (this->options_.useDualZoom)
            {
                auto linkZoom =
                    new wxpex::CheckBox(

                        this,
                        "Link",
                        control.viewSettings.linkZoom);

                this->controlsSizer_->Add(linkZoom, 0, wxRIGHT);

                auto gridSizer = new wxFlexGridSizer(2, gridSpacing, gridSpacing);

                gridSizer->SetFlexibleDirection(wxBOTH);
                gridSizer->AddGrowableRow(1, 1);
                gridSizer->AddGrowableCol(1, 1);

                gridSizer->AddSpacer(0);
                gridSizer->Add(this->controlsSizer_, 0);
                gridSizer->Add(this->verticalZoom_, 1);
                gridSizer->Add(this->canvas_, 1, wxEXPAND);

                innerSizer = gridSizer;
            }
            else
            {
                auto boxSizer = new wxBoxSizer(wxVERTICAL);
                boxSizer->Add(this->controlsSizer_, 0, wxBOTTOM, gridSpacing);
                boxSizer->Add(this->canvas_, 1, wxEXPAND);

                innerSizer = boxSizer;
            }

            sizer->Add(innerSizer, 1, wxALL | wxEXPAND, margin);
        }
        else
        {
            sizer->Add(this->canvas_, 1, wxALL | wxEXPAND, margin);
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

        if (this->options_.displayControls)
        {
            assert(this->controlsSizer_ != nullptr);

            auto controlsSize =
                wxpex::ToSize<int>(this->controlsSizer_->GetMinSize());

            if (this->options_.useDualZoom)
            {
                assert(this->verticalZoom_ != nullptr);

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
        else
        {
            size = Size(canvasSize.width, canvasSize.height);
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
