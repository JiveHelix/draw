#include "draw/waveform_generator.h"


namespace draw
{


PixelMatrix MakeWaveformColorRange(
    const WaveformColor &waveformColor,
    const tau::Hsv<double> &hsv)
{
    auto first = hsv;
    auto last = hsv;

    first.value = waveformColor.range.low;
    last.value = waveformColor.range.high;

    auto gradient = tau::gradient::MakeColormap<uint8_t>(
        waveformColor.count,
        first.ToVector(),
        last.ToVector());

    // Zero will be black
    gradient.row(0) = tau::RowVector<3, uint8_t>(0, 0, 0);

    return gradient;
}


PixelMatrix MakeWaveformColors(const WaveformColor &waveformColor)
{
    PixelMatrix normalValues =
        MakeWaveformColorRange(waveformColor, waveformColor.color);

    PixelMatrix highlightValues =
        MakeWaveformColorRange(waveformColor, waveformColor.highlightColor);

    return tau::VerticalStack(normalValues, highlightValues);
}


WaveformColormap::WaveformColormap(const WaveformColor &waveformColor)
    :
    map_(MakeWaveformColors(waveformColor)),
    rescale_(0, waveformColor.count - 1)
{

}


void WaveformColormap::Filter(
    const WaveformSettings &waveformSettings,
    const Size &displayedSize,
    const DataMatrix &data,
    const std::optional<Highlights> &highlights,
    PixelMatrix *output)
{
    Waveform levelMap = DoGenerateWaveform(
        data,
        waveformSettings.maximumValue,
        waveformSettings.levelCount,
        waveformSettings.columnCount);

    Waveform rescaled = this->rescale_(levelMap);

    if (!highlights)
    {
        auto resized = Resize(
            rescaled,
            displayedSize,
            waveformSettings.verticalScale);

        this->map_(resized, output);

        return;
    }

    if (highlights->any())
    {
        auto highlightsPerColumn =
            static_cast<float>(highlights->cols())
            / static_cast<float>(waveformSettings.columnCount);

        for (
            Eigen::Index column = 0;
            column < rescaled.cols();
            ++column)
        {
            auto beginHighlight = tau::Index(
                std::round(
                    highlightsPerColumn
                    * static_cast<float>(column)));

            auto endHighlight = tau::Index(
                std::round(
                    highlightsPerColumn
                    * static_cast<float>(column + 1)));

            endHighlight = std::min(endHighlight, highlights->cols());
            auto width = endHighlight - beginHighlight;

            if (highlights->middleCols(beginHighlight, width).any())
            {
                // Change this column to the highlight color.
                rescaled.col(column).array()
                    += static_cast<uint16_t>(waveformSettings.color.count);
            }
        }
    }

    auto resized = Resize(
        rescaled,
        displayedSize,
        waveformSettings.verticalScale);

    this->map_(resized, output);
}


WaveformGenerator::WaveformGenerator(
    WaveformControl waveformControl,
    PixelViewControl pixelViewControl)
    :
    mutex_(),
    waveformControl_(waveformControl),
    pixelViewControl_(pixelViewControl),

    colorEndpoint_(
        this,
        waveformControl.color,
        &WaveformGenerator::OnColorMapChanged_),

    waveformSettingsEndpoint_(
        this,
        waveformControl,
        &WaveformGenerator::OnWaveformSettings_),

    viewSizeEndpoint_(
        this,
        pixelViewControl.viewSettings.viewSize,
        &WaveformGenerator::OnViewSize_),

    waveformSettings_(this->waveformControl_.Get()),
    viewSize_(this->pixelViewControl_.viewSettings.viewSize),

    isRunning_(true),
    hasFrame_(false),
    data_(),
    highlights_(),
    colorMap_(waveformControl.color.Get()),
    hasFrameCondition_(),
    thread_(std::bind(&WaveformGenerator::Run_, this))
{

}


WaveformGenerator::WaveformGenerator(WaveformGenerator &&other)
    :
    WaveformGenerator(
        std::move(other),
        pex::WriteLock(other.mutex_))
{

}


WaveformGenerator::~WaveformGenerator()
{
    this->Shutdown();
}


void WaveformGenerator::operator()(
    const DataMatrix &data,
    const std::optional<Highlights> &highlights)
{
    pex::WriteLock lock(this->mutex_);
    this->data_ = data;
    this->highlights_ = highlights;
    this->hasFrame_ = true;
    this->hasFrameCondition_.notify_one();
}

void WaveformGenerator::Shutdown()
{
    if (this->thread_.joinable())
    {
        {
            pex::WriteLock lock(this->mutex_);
            this->isRunning_ = false;

            // Wake up worker so it will exit.
            this->hasFrameCondition_.notify_one();
        }

        this->thread_.join();
    }
}


void WaveformGenerator::OnColorMapChanged_(const WaveformColor &waveformColor)
{
    pex::WriteLock lock(this->mutex_);
    this->colorMap_ = WaveformColormap(waveformColor);
}


void WaveformGenerator::OnWaveformSettings_(
    const WaveformSettings &waveformSettings)
{
    pex::WriteLock lock(this->mutex_);
    this->waveformSettings_ = waveformSettings;
}


void WaveformGenerator::OnViewSize_(const Size &viewSize)
{
    pex::WriteLock lock(this->mutex_);
    this->viewSize_ = viewSize;
}


void WaveformGenerator::Run_()
{
    while (this->isRunning_)
    {
        pex::ReadLock lock(this->mutex_);

        if (!this->hasFrame_)
        {
            this->hasFrameCondition_.wait(
                lock,
                [this]{ return this->hasFrame_ || !this->isRunning_; });
        }

        if (!this->isRunning_)
        {
            return;
        }

        // TODO: Change the input frames to a queue.
        this->hasFrame_ = false;

        auto waveformPixels = Pixels::CreateShared(this->viewSize_);

        this->colorMap_.Filter(
            this->waveformSettings_,
            this->viewSize_,
            this->data_,
            this->highlights_,
            &waveformPixels->data);

        this->pixelViewControl_.asyncPixels.Set(waveformPixels);
    }
}

WaveformGenerator::WaveformGenerator(
    WaveformGenerator &&other,
    const pex::WriteLock &)
    :
    mutex_(),
    waveformControl_(std::move(other.waveformControl_)),
    pixelViewControl_(std::move(other.pixelViewControl_)),

    colorEndpoint_(
        this,
        this->waveformControl_.color,
        &WaveformGenerator::OnColorMapChanged_),

    waveformSettingsEndpoint_(
        this,
        this->waveformControl_,
        &WaveformGenerator::OnWaveformSettings_),

    viewSizeEndpoint_(
        this,
        this->pixelViewControl_.viewSettings.viewSize,
        &WaveformGenerator::OnViewSize_),

    waveformSettings_(other.waveformSettings_),
    viewSize_(other.viewSize_),

    isRunning_(true),
    hasFrame_(false),
    data_(),
    highlights_(),
    colorMap_(std::move(other.colorMap_)),
    hasFrameCondition_(),
    thread_(std::bind(&WaveformGenerator::Run_, this))
{

}


} // end namespace draw
