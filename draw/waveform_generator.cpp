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
    const Highlights *highlights,
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
            auto beginHighlight = FloatToIndex(
                std::round(
                    highlightsPerColumn
                    * static_cast<float>(column)));

            auto endHighlight = FloatToIndex(
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

    imageSizeEndpoint_(
        this,
        pixelViewControl.viewSettings.imageSize,
        &WaveformGenerator::OnImageSize_),

    waveformSettings_(this->waveformControl_.Get()),
    imageSize_(this->pixelViewControl_.viewSettings.imageSize),

    isRunning_(true),
    inputs_(),
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

    this->inputs_.emplace(
        this->waveformSettings_,
        this->imageSize_,
        data,
        highlights);

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


bool WaveformGenerator::Enabled() const
{
    pex::ReadLock lock(this->mutex_);
    return this->waveformSettings_.enable;
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


void WaveformGenerator::OnImageSize_(const Size &imageSize)
{
    pex::WriteLock lock(this->mutex_);
    this->imageSize_ = imageSize;
}


void WaveformGenerator::Run_()
{
    while (this->isRunning_)
    {
        WaveformInput input;

        {
            pex::WriteLock lock(this->mutex_);

            if (this->inputs_.empty())
            {
                this->hasFrameCondition_.wait(
                    lock,
                    [this]
                    {
                        return !this->inputs_.empty() || !this->isRunning_;
                    });
            }

            if (!this->isRunning_)
            {
                return;
            }

            assert(!this->inputs_.empty());

            input = this->inputs_.front();
            this->inputs_.pop();
        }

        if (input.imageSize.width == 0 || input.imageSize.height == 0)
        {
            continue;
        }

        auto waveformPixels = Pixels::CreateShared(input.imageSize);

        this->colorMap_.Filter(
            input.waveformSettings,
            input.imageSize,
            *input.data,
            input.highlights.get(),
            &waveformPixels->data);

        if (waveformPixels->data.rows() == 0
                || waveformPixels->data.cols() == 0)
        {
            throw std::logic_error("Must not be empty");
        }

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

    imageSizeEndpoint_(
        this,
        this->pixelViewControl_.viewSettings.imageSize,
        &WaveformGenerator::OnImageSize_),

    waveformSettings_(other.waveformSettings_),
    imageSize_(other.imageSize_),

    isRunning_(true),
    inputs_(),
    colorMap_(std::move(other.colorMap_)),
    hasFrameCondition_(),
    thread_(std::bind(&WaveformGenerator::Run_, this))
{

}


} // end namespace draw
