#pragma once

#include <thread>
#include <condition_variable>

#include <tau/color_map.h>
#include <tau/color_maps/gradient.h>
#include <tau/stack.h>
#include <pex/log.h>
#include <pex/locks.h>

#include <draw/waveform.h>
#include <draw/views/pixel_view_settings.h>
#include <draw/pixels.h>



namespace draw
{


using DataMatrix =
    Eigen::Matrix<int32_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;


PixelMatrix MakeWaveformColorRange(
    const WaveformColor &waveformColor,
    const tau::Hsv<double> &hsv);


PixelMatrix MakeWaveformColors(const WaveformColor &waveformColor);


using Highlights = Eigen::RowVector<bool, Eigen::Dynamic>;


class WaveformColormap
{
public:
    using ColorMap = tau::ColorMap<PixelMatrix>;
    using Rescale = tau::Rescale<size_t>;

    WaveformColormap(const WaveformColor &waveformColor);

    void Filter(
        const WaveformSettings &waveformSettings,
        const Size &displayedSize,
        const DataMatrix &data,
        const std::optional<Highlights> &highlights,
        PixelMatrix *output);

private:
    ColorMap map_;
    Rescale rescale_;
};


class WaveformGenerator
{
public:
    static constexpr auto observerName = "WaveformGenerator";

    WaveformGenerator(
        WaveformControl waveformControl,
        PixelViewControl pixelViewControl);

    WaveformGenerator(WaveformGenerator &&other);

    ~WaveformGenerator();

    void operator()(
        const DataMatrix &data,
        const std::optional<Highlights> &highlights = {});

    void Shutdown();

private:
    void OnColorMapChanged_(const WaveformColor &);
    void OnWaveformSettings_(const WaveformSettings &);
    void OnViewSize_(const Size &);

    void Run_();

private:
    WaveformGenerator(WaveformGenerator &&other, const pex::WriteLock &);

private:
    mutable pex::Mutex mutex_;
    WaveformControl waveformControl_;
    PixelViewControl pixelViewControl_;
    pex::Endpoint<WaveformGenerator, WaveformColorControl> colorEndpoint_;
    pex::Endpoint<WaveformGenerator, WaveformControl> waveformSettingsEndpoint_;
    pex::Endpoint<WaveformGenerator, SizeControl> viewSizeEndpoint_;
    WaveformSettings waveformSettings_;
    Size viewSize_;
    bool isRunning_;
    bool hasFrame_;
    DataMatrix data_;
    std::optional<Highlights> highlights_;

    WaveformColormap colorMap_;

    std::condition_variable_any hasFrameCondition_;
    std::thread thread_;
};


} // end namespace draw
