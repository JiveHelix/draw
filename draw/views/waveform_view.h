#pragma once


#include <mutex>
#include <pex/value.h>
#include <pex/endpoint.h>
#include <wxpex/wxshim.h>

#include "draw/pixels.h"
#include "draw/waveform_settings.h"
#include "draw/views/pixel_view_settings.h"


namespace draw
{


class WaveformPixels: public wxPanel
{
public:
    static constexpr auto observerName = "WaveformPixels";

    WaveformPixels(
        wxWindow *parent,
        PixelViewControl pixelViewControl,
        WaveformControl waveformControl);

    Eigen::Vector<wxCoord, Eigen::Dynamic> GetLines() const;

private:
    void OnPaint_(wxPaintEvent &);

    void OnPixels_(const std::shared_ptr<Pixels> &pixels);

    void OnSize_(wxSizeEvent &);

private:
    WaveformControl waveformControl_;
    PixelViewControl pixelViewControl_;
    pex::Endpoint<WaveformPixels, PixelsControl> waveformPixels_;
    std::shared_ptr<Pixels> waveformData_;
    wxImage image_;
};


class WaveformView: public wxFrame
{
public:
    static constexpr auto observerName = "WaveformView";

    WaveformView(
        wxWindow *parent,
        PixelViewControl pixelViewControl,
        WaveformControl waveformControl,
        const std::string &title);

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


} // end namespace draw
