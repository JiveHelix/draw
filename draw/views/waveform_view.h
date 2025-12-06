#pragma once


#include <mutex>
#include <pex/value.h>
#include <pex/endpoint.h>
#include <wxpex/wxshim.h>
#include <wxpex/splitter.h>

#include "draw/pixels.h"
#include "draw/waveform_settings.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/views/pixel_canvas.h"
#include "draw/views/view_link.h"
#include "draw/shapes.h"


namespace draw
{


class WaveformPixels: public wxPanel
{
public:
    static constexpr auto observerName = "WaveformPixels";

    WaveformPixels(
        wxWindow *parent,
        const PixelViewControl &mainViewControl,
        const PixelViewControl &pixelViewControl,
        const WaveformControl &waveformControl);

    Eigen::Vector<wxCoord, Eigen::Dynamic> GetLines() const;

private:
    void OnViewSize_(const Size &viewSize);
    void OnImageSize_(const Size &imageSize);

private:
    ShapesId oddLinesId_;
    ShapesId evenLinesId_;
    PixelViewControl pixelViewControl_;
    WaveformControl waveformControl_;
    SizeEndpoint<WaveformPixels> viewSize_;
    SizeEndpoint<WaveformPixels> imageSize_;
};


class WaveformView: public wxPanel
{
public:
    static constexpr auto observerName = "WaveformView";

    WaveformView(
        wxWindow *parent,
        const PixelViewControl &mainViewControl,
        const PixelViewControl &pixelViewControl,
        const WaveformControl &waveformControl);

private:
    void OnPaint_(wxPaintEvent &);

    void OnWaveformSize_(size_t);

    void OnWaveformVerticalScale_(double);

private:
    WaveformPixels *waveformPixels_;

    using ColumnCountEndpoint =
        pex::Endpoint<WaveformView, decltype(WaveformControl::columnCount)>;

    using LevelCountEndpoint =
        pex::Endpoint<WaveformView, decltype(WaveformControl::levelCount)>;

    using VerticalScaleControl = decltype(WaveformControl::verticalScale);

    ColumnCountEndpoint columnCountEndpoint_;
    LevelCountEndpoint levelCountEndpoint_;
    pex::Endpoint<WaveformView, VerticalScaleControl> verticalScaleEndpoint_;
};


class PixelViewAndWaveform: public wxPanel
{
public:
    PixelViewAndWaveform(
        wxWindow *parent,
        const PixelViewControl &mainViewControl,
        const PixelViewControl &waveformPixelView,
        const WaveformControl &waveformControl);

    void SetSashPosition(int sashPosition);

    bool Layout() override;

private:
    ViewLink viewLink_;
    wxpex::Splitter * splitter_;
};


} // end namespace draw
