#include "waveform_view.h"

#include <jive/range.h>
#include <tau/filter.h>
#include <tau/eigen.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/size.h>
#include "draw/size.h"
#include "draw/views/pixel_view.h"
#include "draw/lines_shape.h"


namespace draw
{


WaveformPixels::WaveformPixels(
    wxWindow *parent,
    PixelViewControl mainViewControl,
    PixelViewControl pixelViewControl,
    WaveformControl waveformControl)
    :
    wxPanel(parent, wxID_ANY),

    oddLinesId_(),
    evenLinesId_(),

    pixelViewControl_(pixelViewControl),
    waveformControl_(waveformControl),

    viewSize_(
        this,
        pixelViewControl.viewSettings.viewSize,
        &WaveformPixels::OnViewSize_),

    imageSize_(
        this,
        mainViewControl.viewSettings.imageSize,
        &WaveformPixels::OnImageSize_)
{
    auto canvas = new PixelCanvas(this, pixelViewControl);

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(canvas, 1, wxEXPAND);
    this->SetSizer(sizer.release());
}


void WaveformPixels::OnViewSize_(const Size &viewSize)
{
    // Let the generated waveform height match the height of the view.
    this->pixelViewControl_.viewSettings.imageSize.height.Set(viewSize.height);

    if (viewSize.height < 20)
    {
        return;
    }

    auto linesVector = this->GetLines();

    auto evenLinesSettings = LinesShapeSettings{};
    auto oddLinesSettings = LinesShapeSettings{};

    // Draw even lines heavy
    evenLinesSettings.look.stroke.weight = 1;
    evenLinesSettings.look.stroke.color.value = 0.8;

    oddLinesSettings.look.stroke.weight = 1;
    oddLinesSettings.look.stroke.color.value = 0.5;

    LinesShape::Lines oddLines;
    LinesShape::Lines evenLines;

    auto lines = this->GetLines();

    for (Eigen::Index i = 0; i < lines.size(); ++i)
    {
        auto line = LinesShape::Line(
            tau::Point2d<double>(0.0, lines(i)),
            tau::Vector2d<double>(1.0, 0.0));

        if (i % 2 == 0)
        {
            evenLines.push_back(line);
        }
        else
        {
            oddLines.push_back(line);
        }
    }

    Shapes oddShapes(this->oddLinesId_.Get());
    Shapes evenShapes(this->evenLinesId_.Get());

    oddShapes.EmplaceBack<LinesShape>(oddLinesSettings, oddLines);
    evenShapes.EmplaceBack<LinesShape>(evenLinesSettings, evenLines);

    this->pixelViewControl_.shapes.Set(oddShapes);
    this->pixelViewControl_.shapes.Set(evenShapes);
}


#if 0
// TODO
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
#endif



void WaveformPixels::OnImageSize_(const Size &imageSize)
{
    // Let the generated waveform width match the width of the original
    // image.
    this->pixelViewControl_.viewSettings.imageSize.width.Set(imageSize.width);
}


Eigen::Vector<wxCoord, Eigen::Dynamic> WaveformPixels::GetLines() const
{
    auto imageHeight =
        this->pixelViewControl_.viewSettings.imageSize.height.Get();

    float scale = static_cast<float>(
        this->waveformControl_.verticalScale.Get());

    float height = static_cast<float>(imageHeight - 1);

    auto lines =
        Eigen::Vector<float, 11>::LinSpaced(0, scale * height);

    Eigen::Vector<float, Eigen::Dynamic> filtered = tau::Filter(
        lines,
        lines.array() < height + 1.0f);

    filtered.array() = filtered.array() * -1.0f + height;

    return filtered.array().round().template cast<wxCoord>();
}



WaveformView::WaveformView(
    wxWindow *parent,
    PixelViewControl mainViewControl,
    PixelViewControl pixelViewControl,
    WaveformControl waveformControl)
    :
    wxPanel(parent, wxID_ANY),

    waveformPixels_(
        new WaveformPixels(
            this,
            mainViewControl,
            pixelViewControl,
            waveformControl)),

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

    viewLink_(
        pixelViewControl.viewSettings,
        waveformPixelViewControl.viewSettings,
        LinkOptions{}.SetAll(Link::horizontal)),

    splitter_(new wxpex::Splitter(this))
{
    auto pixelView = new PixelView(
        this->splitter_,
        pixelViewControl);

    auto waveformView =
        new WaveformView(
            this->splitter_,
            pixelViewControl,
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
