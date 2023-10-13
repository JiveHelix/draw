#include "waveform_view.h"

#include <jive/range.h>
#include <tau/filter.h>
#include <tau/eigen.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/labeled_widget.h>
#include "draw/size.h"
#include "draw/views/pixel_view.h"


namespace draw
{


WaveformPixels::WaveformPixels(
    wxWindow *parent,
    PixelViewControl pixelViewControl,
    WaveformControl waveformControl)
    :
    wxPanel(parent, wxID_ANY),
    pixelViewControl_(pixelViewControl),
    waveformControl_(waveformControl),
    waveformPixels_(this, pixelViewControl.pixels, &WaveformPixels::OnPixels_),
    waveformData_(),

    image_(
        this->pixelViewControl_.viewSettings.viewSize.Get().width,
        this->pixelViewControl_.viewSettings.viewSize.Get().height)
{
    this->Bind(wxEVT_PAINT, &WaveformPixels::OnPaint_, this);
    this->Bind(wxEVT_SIZE, &WaveformPixels::OnSize_, this);
    this->SetMinSize(wxSize(300, 250));
}


Eigen::Vector<wxCoord, Eigen::Dynamic> WaveformPixels::GetLines() const
{
    auto size = this->GetClientSize();

    float scale = static_cast<float>(
        this->waveformControl_.verticalScale.Get());

    float height = static_cast<float>(size.GetHeight() - 1);

    auto lines =
        Eigen::Vector<float, 11>::LinSpaced(0, scale * height);

    Eigen::Vector<float, Eigen::Dynamic> filtered = tau::Filter(
        lines,
        lines.array() < height + 1.0f);

    filtered.array() = filtered.array() * -1.0f + height;

    return filtered.array().round().template cast<wxCoord>();
}


void WaveformPixels::OnPixels_(const std::shared_ptr<Pixels> &waveform)
{
    this->waveformData_ = waveform;

    // Pixels are sent asynchronously using the wx event system.
    // When the size of the frame is changing rapidly, it is possible that
    // there are pixels of the wrong size in the event queue.
    // Ignore any that do not match.

    auto imageSize = wxpex::ToSize<Pixels::Index>(this->image_.GetSize());
    auto dataSize = this->waveformData_->size;

    if (imageSize != dataSize)
    {
        this->image_ = wxImage(dataSize.width, dataSize.height, false);
    }

    this->image_.SetData(this->waveformData_->data.data(), true);
    this->Refresh(false);
    this->Update();
}


void WaveformPixels::OnSize_(wxSizeEvent &event)
{
    event.Skip();

    auto size = this->GetClientSize();

    if (size.GetHeight() < 1 || size.GetWidth() < 1)
    {
        return;
    }

    auto imageSize = Size{{
        static_cast<uint16_t>(size.GetWidth()),
        static_cast<uint16_t>(size.GetHeight())}};

    this->image_ = wxImage(
        size.GetWidth(),
        size.GetHeight());

    this->pixelViewControl_.viewSettings.viewSize.Set(imageSize);
}


void WaveformPixels::OnPaint_(wxPaintEvent &)
{
    auto paintDc = wxPaintDC(this);
    auto bitmap = wxBitmap(this->image_);
    auto source = wxMemoryDC(bitmap);
    auto size = this->GetSize();

    paintDc.Blit(
        0,
        0,
        size.GetWidth(),
        size.GetHeight(),
        &source,
        0,
        0);

    auto lines = this->GetLines();
    auto farEdge = size.GetWidth();

    auto lightPen = *wxLIGHT_GREY_PEN;
    auto heavyPen = *wxGREY_PEN;

    for (Eigen::Index i = 0; i < lines.size(); ++i)
    {
        if (i % 2 == 0)
        {
            // Draw even lines heavy
            paintDc.SetPen(heavyPen);
        }
        else
        {
            paintDc.SetPen(lightPen);
        }

        auto index = lines(i);
        paintDc.DrawLine(0, index, farEdge, index);
    }
}


WaveformView::WaveformView(
    wxWindow *parent,
    PixelViewControl pixelViewControl,
    WaveformControl waveformControl)
    :
    wxPanel(parent, wxID_ANY),

    waveformPixels_(
        new WaveformPixels(this, pixelViewControl, waveformControl)),

    columnCountEndpoint_(
        this,
        waveformControl.columnCount,
        &WaveformView::OnWaveformSize_),

    levelCountEndpoint_(
        this,
        waveformControl.levelCount,
        &WaveformView::OnWaveformSize_),

    verticalScaleEndpoint_(
        this,
        waveformControl.verticalScale,
        &WaveformView::OnWaveformVerticalScale_)
{
    this->Bind(wxEVT_PAINT, &WaveformView::OnPaint_, this);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    sizer->Add(this->waveformPixels_, 1, wxEXPAND | wxLEFT | wxRIGHT, 30);
    topSizer->Add(sizer.release(), 1, wxEXPAND | wxTOP | wxBOTTOM, 10);

    this->SetSizer(topSizer.release());
}


void WaveformView::OnWaveformSize_(size_t)
{
    this->Refresh(true);
    this->Update();
}


void WaveformView::OnWaveformVerticalScale_(double)
{
    this->Refresh(true);
    this->Update();
}


void WaveformView::OnPaint_(wxPaintEvent &)
{
    auto paintDc = wxPaintDC(this);

    auto size = paintDc.GetSize();
    auto height = size.GetHeight();

    auto waveformPosition = this->waveformPixels_->GetPosition();
    auto lines = this->waveformPixels_->GetLines();

    auto offsetLines = lines.array() + waveformPosition.y;

    auto rightEdge = waveformPosition.x - 2;

    paintDc.SetFont(wxFontInfo().Family(wxFONTFAMILY_MODERN));
    auto textSize = paintDc.GetTextExtent("100");
    auto leftEdge = rightEdge - textSize.GetWidth();
    auto halfHeight = textSize.GetHeight() / 2;

    auto box = wxRect(
        leftEdge,
        offsetLines(0) - halfHeight,
        textSize.GetWidth(),
        textSize.GetHeight());

    paintDc.SetPen(*wxBLACK_PEN);
    paintDc.SetBrush(*wxBLACK_BRUSH);
    paintDc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    paintDc.SetPen(*wxLIGHT_GREY_PEN);
    paintDc.SetBrush(*wxLIGHT_GREY_BRUSH);

    Eigen::Index lineCount = lines.size();

    for (auto index: jive::Range<Eigen::Index>(0, lineCount))
    {
        auto line = offsetLines(index);
        auto label = std::to_string(index * 10);

        box.SetY(line - halfHeight);

        if (box.GetBottom() >= height)
        {
            // There is not enough room to draw this label.
            continue;
        }

        paintDc.DrawLabel(
            label,
            box,
            wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    }
}


PixelViewAndWaveform::PixelViewAndWaveform(
    wxWindow *parent,
    PixelViewControl pixelViewControl,
    PixelViewControl waveformPixelViewControl,
    WaveformControl waveformControl)
    :
    wxPanel(parent, wxID_ANY),
    splitter_(new wxpex::Splitter(this))
{
    auto pixelView = new PixelView(
        this->splitter_,
        pixelViewControl);

    auto waveformView =
        new WaveformView(
            this->splitter_,
            waveformPixelViewControl,
            waveformControl);

    this->splitter_->SplitHorizontallyTop(pixelView, waveformView);
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(this->splitter_, 1, wxEXPAND);
    this->SetSizerAndFit(sizer.release());
}


void PixelViewAndWaveform::SetSashPosition(int sashPosition)
{
    this->splitter_->SetSashPosition(sashPosition);
}


bool PixelViewAndWaveform::Layout()
{
    bool baseResult = this->wxPanel::Layout();
    bool derivedResult = this->splitter_->Layout();

    return baseResult && derivedResult;
}


} // end namespace draw
