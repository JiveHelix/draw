#include "draw/views/view_settings.h"
#include <jive/path.h>


// #define ENABLE_VIEW_SETTINGS_LOG
#define SHORTFILE jive::path::Base(__FILE__)

#ifdef ENABLE_VIEW_SETTINGS_LOG

#include <fmt/core.h>

#define VIEW_SETTINGS_LOG \
        fmt::print("{}:{}:{} {}\n", SHORTFILE, __FUNCTION__, __LINE__, name_);

#else

#define VIEW_SETTINGS_LOG

#endif

#define VIEW_SHORT_LOG \
        fmt::print("{}:{}:{} {}\n", SHORTFILE, __FUNCTION__, __LINE__, name_);


namespace draw
{


ViewBrain::ViewBrain(
    ModelMembers &model,
    const pex::detail::MuteControl &muteControl)
    :
    model_(model),
    muteControl_(muteControl),

    viewPositionX_(
        PEX_THIS("ViewSettingsModel"),
        IntPointControl(this->model_.viewPosition).x,
        &ViewBrain::OnViewPositionX_),

    viewPositionY_(
        this,
        IntPointControl(this->model_.viewPosition).y,
        &ViewBrain::OnViewPositionY_),

    scaleEndpoint_(
        this,
        ScaleControl(this->model_.scale)),

    imageSizeEndpoint_(this, this->model_.imageSize, &ViewBrain::OnImageSize_),

    viewSizeWidth_(
        this,
        this->model_.viewSize.width,
        &ViewBrain::OnViewSizeWidth_),

    viewSizeHeight_(
        this,
        this->model_.viewSize.height,
        &ViewBrain::OnViewSizeHeight_),

    pivotEndpoint_(),
        // this,
        // this->model_.imagePivot,
        // &ViewBrain::OnImagePivot_),

    linkZoomEndpoint_(
        this,
        this->model_.linkZoom,
        &ViewBrain::OnLinkZoom_),

    resetZoomEndpoint_(
        this,
        this->model_.resetZoom,
        &ViewBrain::ResetZoom),

    fitZoomEndpoint_(
        this,
        this->model_.fitZoom,
        &ViewBrain::FitZoom),

    recenterEndpoint_(
        this,
        this->model_.recenter,
        &ViewBrain::Recenter),

    resetHorizontalZoomEndpoint_(
        this,
        this->model_.resetHorizontalZoom,
        &ViewBrain::OnResetHorizontalZoom_),

    resetVerticalZoomEndpoint_(
        this,
        this->model_.resetVerticalZoom,
        &ViewBrain::OnResetVerticalZoom_),

    fitHorizontalZoomEndpoint_(
        this,
        this->model_.fitHorizontalZoom,
        &ViewBrain::OnFitHorizontalZoom_),

    fitVerticalZoomEndpoint_(
        this,
        this->model_.fitVerticalZoom,
        &ViewBrain::OnFitVerticalZoom_),

    recenterHorizontalEndpoint_(
        this,
        this->model_.recenterHorizontal,
        &ViewBrain::OnRecenterHorizontal_),

    recenterVerticalEndpoint_(
        this,
        this->model_.recenterVertical,
        &ViewBrain::OnRecenterVertical_),

    ignoreZoom_(false),
    ignoreViewPosition_(false),
    ignoreSize_(false)
{
    PEX_MEMBER(scaleEndpoint_);

    this->scaleEndpoint_.horizontal.Connect(
        &ViewBrain::OnHorizontalZoom_);

    this->scaleEndpoint_.vertical.Connect(
        &ViewBrain::OnVerticalZoom_);

    this->ResetView_(this->model_.imageSize.Get(), this->model_.viewSize.Get());
}

void ViewBrain::SetName(const std::string &name)
{
    this->name_ = name;
}

void ViewBrain::ResetZoom()
{
    VIEW_SETTINGS_LOG

    auto deferScale = pex::MakeDefer(this->model_.scale);

    deferScale.Set(Scale{});
    this->ResetView_(this->model_.imageSize.Get(), this->model_.viewSize.Get());
}

void ViewBrain::FitZoom()
{
    VIEW_SETTINGS_LOG

    auto imageSize_ = tau::Size<double>(this->model_.imageSize.Get());

    // Fit without scrollbars.
    auto windowSize_ = this->model_.windowSize.Get().template Cast<double>();

    // imageSize_ * fit = windowSize_
    windowSize_ /= imageSize_;
    auto fit = Scale{windowSize_.height, windowSize_.width};

    if (this->model_.linkZoom.Get())
    {
        double smaller = std::min(fit.horizontal, fit.vertical);
        fit.horizontal = smaller;
        fit.vertical = smaller;
    }

    jive::ScopeFlag ignoreZoom(this->ignoreZoom_);

    auto deferScale = pex::MakeDefer(this->model_.scale);

    if (fit.horizontal < this->model_.scale.horizontal.GetMinimum())
    {
        this->model_.scale.horizontal.SetMinimum(fit.horizontal);
    }

    if (fit.vertical < this->model_.scale.vertical.GetMinimum())
    {
        this->model_.scale.vertical.SetMinimum(fit.vertical);
    }

    deferScale.Set(fit);

    // Reset the imageCenterPixel_ before the zoom scale is notified.
    this->ResetView_(
        this->model_.imageSize.Get(),
        this->model_.windowSize.Get());
}

void ViewBrain::Recenter()
{
    VIEW_SETTINGS_LOG

    this->ResetView_(
        this->model_.imageSize.Get(),
        this->model_.windowSize.Get());

    jive::ScopeFlag ignoreZoom(this->ignoreZoom_);
    pex::AccessReference(this->model_.scale).Notify();
}

void ViewBrain::HoldPivot(const Size &viewSize_)
{
    if (this->ignoreViewPosition_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    auto viewPosition_ = this->GetViewPosition_(viewSize_);

    // We need to notify observers of the change to view position
    // without calling our own handler `OnViewPosition_`
    jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
    this->model_.viewPosition.Set(viewPosition_);
    this->UpdateVirtualSize();
}

void ViewBrain::HoldPivot()
{
    this->HoldPivot(this->model_.viewSize.Get());
}

void ViewBrain::HoldPivotX(const Size &viewSize_)
{
    if (this->ignoreViewPosition_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    auto viewPositionX = this->GetViewPositionX_(viewSize_);

    // We need to notify observers of the change to view position
    // without calling our own handler `OnViewPosition_`
    jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
    this->model_.viewPosition.x.Set(viewPositionX);
    this->UpdateVirtualSize();
}

void ViewBrain::HoldPivotX()
{
    this->HoldPivotX(this->model_.viewSize.Get());
}

void ViewBrain::HoldPivotY(const Size &viewSize_)
{
    if (this->ignoreViewPosition_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    auto viewPositionY = this->GetViewPositionY_(viewSize_);

    // We need to notify observers of the change to view position
    // without calling our own handler `OnViewPosition_`
    jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
    this->model_.viewPosition.y.Set(viewPositionY);
    this->UpdateVirtualSize();
}

void ViewBrain::HoldPivotY()
{
    this->HoldPivotY(this->model_.viewSize.Get());
}

void ViewBrain::OnResetHorizontalZoom_()
{
    VIEW_SETTINGS_LOG

    auto deferScale = pex::MakeDefer(this->model_.scale.horizontal);
    deferScale.Set(1);

    this->ResetView_(
        this->model_.imageSize.Get(),
        this->model_.viewSize.Get());
}

void ViewBrain::OnResetVerticalZoom_()
{
    VIEW_SETTINGS_LOG

    auto deferScale = pex::MakeDefer(this->model_.scale.vertical);
    deferScale.Set(1);

    this->ResetView_(
        this->model_.imageSize.Get(),
        this->model_.viewSize.Get());
}

void ViewBrain::OnFitHorizontalZoom_()
{
    VIEW_SETTINGS_LOG

    // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

    auto imageWidth_ = static_cast<double>(this->model_.imageSize.width.Get());

    // Fit without scrollbars.
    auto windowWidth_ =
        static_cast<double>(this->model_.windowSize.width.Get());

    // imageWidth_ * fit = windowWidth_
    auto fit = windowWidth_ / imageWidth_;

    jive::ScopeFlag ignoreZoom(this->ignoreZoom_);

    auto deferHorizontalScale = pex::MakeDefer(this->model_.scale.horizontal);

    if (fit < this->model_.scale.horizontal.GetMinimum())
    {
        this->model_.scale.horizontal.SetMinimum(fit);
    }

    deferHorizontalScale.Set(fit);

    // Reset the imageCenterPixel_ before the zoom scale is notified.
    this->ResetView_(
        this->model_.imageSize.Get(),
        this->model_.windowSize.Get());
}

void ViewBrain::OnFitVerticalZoom_()
{
    VIEW_SETTINGS_LOG

    // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

    auto imageHeight_ =
        static_cast<double>(this->model_.imageSize.height.Get());

    // Fit without scrollbars.
    auto windowHeight_ =
        static_cast<double>(this->model_.windowSize.height.Get());

    // imageHeight_ * fit = windowHeight_
    auto fit = windowHeight_ / imageHeight_;

    jive::ScopeFlag ignoreZoom(this->ignoreZoom_);

    auto deferVerticalScale = pex::MakeDefer(this->model_.scale.vertical);

    if (fit < this->model_.scale.vertical.GetMinimum())
    {
        this->model_.scale.vertical.SetMinimum(fit);
    }

    deferVerticalScale.Set(fit);

    // Reset the imageCenterPixel_ before the zoom scale is notified.
    this->ResetView_(
        this->model_.imageSize.Get(),
        this->model_.windowSize.Get());
}

void ViewBrain::OnRecenterHorizontal_()
{
    if (this->ignoreViewPosition_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

    auto imageWidth_ = this->model_.imageSize.width.Get();
    this->model_.imagePivot.x.Set(imageWidth_ / 2.0);
    auto viewPositionX = this->GetViewPositionX_(this->model_.viewSize.Get());

    jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
    this->model_.viewPosition.x.Set(viewPositionX);
    this->UpdateVirtualSize();
}

void ViewBrain::OnRecenterVertical_()
{
    if (this->ignoreViewPosition_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

    auto imageHeight_ = this->model_.imageSize.height.Get();
    this->model_.imagePivot.y.Set(imageHeight_ / 2.0);

    auto viewPositionY =
        this->GetViewPositionY_(this->model_.viewSize.Get());

    jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
    this->model_.viewPosition.y.Set(viewPositionY);
    this->UpdateVirtualSize();
}

void ViewBrain::UpdateVirtualSize()
{

    // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

    auto imageSize_ = this->model_.imageSize.Get();
    auto virtualSize_ = imageSize_.template Cast<double>();

    virtualSize_.width *= this->model_.scale.horizontal.Get();
    virtualSize_.height *= this->model_.scale.vertical.Get();
    virtualSize_.width = std::round(virtualSize_.width);
    virtualSize_.height = std::round(virtualSize_.height);

    auto virtualAsInt = virtualSize_.template Cast<int>();
    auto viewPosition_ = this->model_.viewPosition.Get();
    auto viewSize_ = this->model_.viewSize.Get();

    // When the virtual panel is off screen to the top or left, the
    // position is positive.
    // When virtual panel begins to the right or bottom of the top left
    // corner, the position is negative.
    auto AdjustVirtualSize =
        [](auto &position_, auto virtual_, auto view_)
        {
            if (position_ > 0)
            {
                // Off screen to the left or top
                // We must be able to scroll it back into view.
                // THe virtual_ needs to as big as what is off
                // screen plus the view_.
                auto minimumVirtual = position_ + view_;
                return std::max(minimumVirtual, virtual_);
            }

#if 0
            if (position_ < 0)
            {
                auto minimumVirtual = view_ - position_;

                if (minimumVirtual > virtual_)
                {
                    // Adjust virtual_ and position_
                    auto difference = minimumVirtual - virtual_;
                    position_ += difference;

                    return minimumVirtual;
                }
            }
#endif

            return virtual_;
        };

    virtualAsInt.width =
        AdjustVirtualSize(
            viewPosition_.x,
            virtualAsInt.width,
            viewSize_.width);

    virtualAsInt.height =
        AdjustVirtualSize(
            viewPosition_.y,
            virtualAsInt.height,
            viewSize_.height);

    this->model_.virtualSize.Set(virtualAsInt);

}

IntPoint ViewBrain::GetViewPosition_(const Size &viewSize_)
{
    auto scaledPivot =
        this->model_.imagePivot.Get() * this->model_.scale.Get();

    auto halfView =
        viewSize_.ToPoint2d().template Cast<double>() / 2.0;

    auto result = scaledPivot - halfView;
    return result.template Cast<int, tau::Round>();
}

int ViewBrain::GetViewPositionX_(const Size &viewSize_)
{
    auto scaledPivot =
        this->model_.imagePivot.x.Get() * this->model_.scale.horizontal.Get();

    auto halfView =
        static_cast<double>(viewSize_.width) / 2.0;

    auto result = scaledPivot - halfView;

    return static_cast<int>(std::round(result));
}

int ViewBrain::GetViewPositionY_(const Size &viewSize_)
{
    auto scaledPivot =
        this->model_.imagePivot.y.Get() * this->model_.scale.vertical.Get();

    auto halfView =
        static_cast<double>(viewSize_.height) / 2.0;

    auto result = scaledPivot - halfView;

    return static_cast<int>(std::round(result));
}

void ViewBrain::SetImagePivot_(
    const tau::Point2d<double> &point)
{

    this->model_.imagePivot.Set(point);
}

double ViewBrain::ComputeImagePivotX_() const
{
    auto viewPositionX =
        static_cast<double>(this->model_.viewPosition.x.Get());

    auto halfWidth =
        static_cast<double>(this->model_.viewSize.width.Get()) / 2.0;

    auto imageWidth = this->model_.imageSize.width.Get();
    auto horizontalScale = this->model_.scale.horizontal.Get();

    auto viewCenterPixelX = viewPositionX + halfWidth;

    auto imageCenterPixelX = viewCenterPixelX / horizontalScale;

    auto asInteger = static_cast<int>(std::round(imageCenterPixelX));

    if (asInteger >= imageWidth)
    {
        imageCenterPixelX = static_cast<double>(imageWidth - 1);
    }
    else if (asInteger < 0)
    {
        imageCenterPixelX = 0.0;
    }

    return imageCenterPixelX;
}

double ViewBrain::ComputeImagePivotY_() const
{
    auto viewPositionY =
        static_cast<double>(this->model_.viewPosition.y.Get());

    auto halfHeight =
        static_cast<double>(this->model_.viewSize.height.Get()) / 2.0;

    auto imageHeight = this->model_.imageSize.height.Get();
    auto verticalScale = this->model_.scale.vertical.Get();

    auto viewCenterPixelY = viewPositionY + halfHeight;

    auto imageCenterPixelY = viewCenterPixelY / verticalScale;

    auto asInteger = static_cast<int>(std::round(imageCenterPixelY));

    if (asInteger >= imageHeight)
    {
        imageCenterPixelY = static_cast<double>(imageHeight - 1);
    }
    else if (asInteger < 0)
    {
        imageCenterPixelY = 0.0;
    }

    return imageCenterPixelY;
}

void ViewBrain::OnHorizontalZoom_(double horizontalZoom)
{
    if (this->ignoreZoom_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    auto ignore = jive::ScopeFlag(this->ignoreZoom_);
    auto deferScale = pex::MakeDefer(this->model_.scale.vertical);

    if (this->model_.linkZoom.Get())
    {
        deferScale.Set(horizontalZoom);
    }

    if (HasHorizontal(this->model_.keepCenter.Get()))
    {
        this->OnRecenterHorizontal_();
    }

    this->HoldPivot();
}

void ViewBrain::OnVerticalZoom_(double verticalZoom)
{
    if (this->ignoreZoom_)
    {
        return;
    }

    auto ignore = jive::ScopeFlag(this->ignoreZoom_);
    auto deferScale = pex::MakeDefer(this->model_.scale.horizontal);

    if (this->model_.linkZoom.Get())
    {
        deferScale.Set(verticalZoom);
    }

    if (HasVertical(this->model_.keepCenter.Get()))
    {
        this->OnRecenterVertical_();
    }

    this->HoldPivot();
}

void ViewBrain::OnLinkZoom_(bool isLinked)
{
    if (isLinked)
    {
        VIEW_SETTINGS_LOG

        std::cout << &this->model_ << " is linked" << std::endl;
        // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

        this->ignoreZoom_ = true;
        this->model_.scale.vertical.Set(this->model_.scale.horizontal.Get());
        this->ignoreZoom_ = false;
    }
    else
    {
        std::cout << &this->model_ << " not linked" << std::endl;
    }
}

void ViewBrain::OnViewPositionX_(int)
{
    if (this->ignoreViewPosition_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

    this->model_.imagePivot.x.Set(this->ComputeImagePivotX_());

    jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
    this->UpdateVirtualSize();
}

void ViewBrain::OnViewPositionY_(int positionY)
{
    if (this->ignoreViewPosition_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

    this->model_.imagePivot.y.Set(this->ComputeImagePivotY_());

    jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
    this->UpdateVirtualSize();
}

void ViewBrain::OnImageSize_(const Size &imageSize_)
{
    VIEW_SETTINGS_LOG

    // Reset the view when the image size changes.
    this->ResetView_(imageSize_, this->model_.viewSize.Get());
}

void ViewBrain::OnViewSizeWidth_(int)
{
    VIEW_SETTINGS_LOG

    // Recompute the image center pixel.
    this->model_.imagePivot.x.Set(this->ComputeImagePivotX_());

    if (this->ignoreSize_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    jive::ScopeFlag ignoreSize(this->ignoreSize_);
    this->UpdateVirtualSize();
}

void ViewBrain::OnViewSizeHeight_(int)
{
    VIEW_SETTINGS_LOG

    // Recompute the image center pixel.
    this->model_.imagePivot.y.Set(this->ComputeImagePivotY_());

    if (this->ignoreSize_)
    {
        return;
    }

    VIEW_SETTINGS_LOG

    jive::ScopeFlag ignoreSize(this->ignoreSize_);
    this->UpdateVirtualSize();
}

void ViewBrain::OnImagePivot_(const Point &pivot)
{
    std::cout << this->name_ << " imagePivot: " << pivot << std::endl;
}

void ViewBrain::ResetView_(
    const Size &imageSize_,
    const Size &viewSize_)
{
    VIEW_SETTINGS_LOG

    // auto mute = pex::detail::ScopeMute(this->muteControl_, false);

    this->SetImagePivot_(
        imageSize_.ToPoint2d().template Cast<double>() / 2.0);

    this->HoldPivot(viewSize_);
}


IntPoint GetMaximumViewPosition(const Size &viewSize, const Size &virtualSize)
{
    IntPoint maximumPosition = (virtualSize - viewSize).ToPoint2d();
    maximumPosition.x = std::max(0, maximumPosition.x);
    maximumPosition.y = std::max(0, maximumPosition.y);

    return maximumPosition;
}


} // end namespace draw
