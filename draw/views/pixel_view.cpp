#include "draw/views/pixel_view.h"
#include <wxpex/button.h>
#include <wxpex/check_box.h>


namespace draw
{


PixelView::PixelView(
    wxWindow *parent,
    PixelViewControl controls)
    :
    wxPanel(parent, wxID_ANY),
    horizontalZoom_(
        new ScaleSlider(
            this,
            controls.viewSettings.scale.horizontal,
            controls.viewSettings.scale.horizontal.value,
            wxpex::Style::horizontal)),
    verticalZoom_(
        new ScaleSlider(
            this,
            controls.viewSettings.scale.vertical,
            controls.viewSettings.scale.vertical.value,
            wxpex::Style::vertical)),
    controlsSizer_(new wxBoxSizer(wxHORIZONTAL))
{
    this->canvas_ = new PixelCanvas(this, controls);

    auto zoomLabel = new wxStaticText(this, wxID_ANY, "Scale");

    auto resetZoom =
        new wxpex::Button(this, "Reset", controls.viewSettings.resetZoom);

    auto fitZoom =
        new wxpex::Button(this, "Fit", controls.viewSettings.fitZoom);

    auto linkZoom =
        new wxpex::CheckBox(this, "Link", controls.viewSettings.linkZoom);

    this->controlsSizer_->Add(this->horizontalZoom_, 0, wxRIGHT, 3);
    this->controlsSizer_->Add(resetZoom, 0, wxRIGHT, 3);
    this->controlsSizer_->Add(fitZoom, 0, wxRIGHT, 3);
    this->controlsSizer_->Add(linkZoom, 0, wxRIGHT);

    auto gridSizer = new wxFlexGridSizer(2, gridSpacing, gridSpacing);

    gridSizer->SetFlexibleDirection(wxBOTH);
    gridSizer->AddGrowableRow(1, 1);
    gridSizer->AddGrowableCol(1, 1);

    gridSizer->Add(zoomLabel, 0);
    gridSizer->Add(this->controlsSizer_, 0);
    gridSizer->Add(this->verticalZoom_, 1);
    gridSizer->Add(this->canvas_, 1, wxEXPAND);

    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(gridSizer, 1, wxALL | wxEXPAND, margin);
    this->SetSizer(sizer);

    this->SetMinSize(wxSize(100, 100));
}


wxSize PixelView::DoGetBestSize() const
{
    auto result =
        this->GetWindowSize_(this->canvas_->GetVirtualSize());

    return wxpex::ToWxSize(result);
}


Size PixelView::GetWindowSize_(const Size &canvasSize) const
{
    // Recalculate window size based on canvas size.
    auto verticalZoomSize =
        wxpex::ToSize<int>(this->verticalZoom_->GetBestSize());

    auto controlsSize = wxpex::ToSize<int>(this->controlsSizer_->GetMinSize());

    Size size(
        std::max(canvasSize.width, controlsSize.width)
            + verticalZoomSize.width,
        std::max(canvasSize.height, verticalZoomSize.height)
            + controlsSize.height);

    auto scrollbarPadding = 20;
    auto extra = gridSpacing + (margin * 2) + scrollbarPadding;
    size += extra;

    return size;
}


PixelFrame::PixelFrame(PixelViewControl control, const std::string &title)
    :
    wxFrame(nullptr, wxID_ANY, title)
{
    auto pixelView = new PixelView(this, control);
    pixelView->SetMinSize(wxSize(400, 225));
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(pixelView, 1, wxEXPAND);
    this->SetSizerAndFit(sizer.release());
}


} // end namespace draw
