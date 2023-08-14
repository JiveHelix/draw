#include "draw/views/view_settings.h"


namespace draw
{


ViewSettingsModel::ViewSettingsModel()
    :
    ViewSettingsGroup::Model(),
    viewPositionEndpoint_(
        this,
        PointControl(this->viewPosition),
        &ViewSettingsModel::OnViewPosition_),
    scaleEndpoint_(
        this,
        ScaleControl(this->scale)),
    imageSizeEndpoint_(this, this->imageSize, &ViewSettingsModel::OnImageSize_),
    linkZoomTerminus_(this, this->linkZoom),
    resetZoomTerminus_(this, this->resetZoom),
    fitZoomTerminus_(this, this->fitZoom),
    ignoreZoom_(false),
    ignoreViewPosition_(false)
{
    this->scaleEndpoint_.horizontal.Connect(
        &ViewSettingsModel::OnHorizontalZoom_);

    this->scaleEndpoint_.vertical.Connect(
        &ViewSettingsModel::OnVerticalZoom_);

    this->linkZoomTerminus_.Connect(&ViewSettingsModel::OnLinkZoom_);
    this->resetZoomTerminus_.Connect(&ViewSettingsModel::ResetZoom);
    this->fitZoomTerminus_.Connect(&ViewSettingsModel::FitZoom);

    this->ResetView_(this->imageSize.Get());
}


// Compute the coordinates of the unscaled pixel using current zoom.
Point ViewSettings::GetLogicalPosition(const Point &point) const
{
    return (point + this->viewPosition) / this->scale;
}


void ViewSettingsModel::SetImageCenterPixel_(const tau::Point2d<double> &point)
{
    this->imageCenterPixel_ = point;
}


tau::Point2d<double> ViewSettingsModel::ComputeImageCenterPixel() const
{
    auto viewPosition_ = this->viewPosition.Get().template Convert<double>();

    auto halfView =
        this->viewSize.Get().ToPoint2d().template Convert<double>() / 2.0;

    auto viewCenterPixel = viewPosition_ + halfView;

    auto imageCenterPixel = viewCenterPixel / this->scale.Get();

    auto asIntegers = imageCenterPixel.template Convert<int, tau::Floor>();
    auto size = this->imageSize.Get();

    if (asIntegers.x >= size.width)
    {
        imageCenterPixel.x = static_cast<double>(size.width - 1);
    }
    else if (asIntegers.x < 0)
    {
        imageCenterPixel.x = 0.0;
    }

    if (asIntegers.y >= size.height)
    {
        imageCenterPixel.y = static_cast<double>(size.height - 1);
    }
    else if (asIntegers.y < 0)
    {
        imageCenterPixel.y = 0.0;
    }

    return imageCenterPixel;
}


Point ViewSettingsModel::GetViewPositionFromCenterImagePixel() const
{
    auto scaledCenterPixel = this->imageCenterPixel_ * this->scale.Get();

    auto halfView =
        this->viewSize.Get().template Convert<double>().ToPoint2d() / 2.0;

    auto result = (scaledCenterPixel - halfView)
        .template Convert<int, tau::Floor>();

    return result;
}


void ViewSettingsModel::ResetZoom()
{
    this->scale.Set(Scale{});
    this->ResetView_(this->imageSize.Get());
}


void ViewSettingsModel::FitZoom()
{
    auto imageSize_ = tau::Size<double>(this->imageSize.Get());
    auto viewSize_ = this->viewSize.Get().template Convert<double>();

    // imageSize_ * fit = viewSize_
    viewSize_ /= imageSize_;
    auto fit = Scale{viewSize_.height, viewSize_.width};

    auto scaleDeferred =
        pex::Defer<decltype(this->scale)>(this->scale);

    if (this->linkZoom.Get())
    {
        double smaller = std::min(fit.horizontal, fit.vertical);
        fit.horizontal = smaller;
        fit.vertical = smaller;
        scaleDeferred.Set(fit);
    }
    else
    {
        // Fit horizontal and vertical zoom independently
        scaleDeferred.Set(fit);
    }

    // Reset the imageCenterPixel_ before the zoom scale is notified.
    this->ResetView_(this->imageSize.Get());
}


void ViewSettingsModel::OnHorizontalZoom_(double horizontalZoom)
{
    if (this->ignoreZoom_)
    {
        return;
    }

    if (this->linkZoom.Get())
    {
        this->ignoreZoom_ = true;
        this->scale.vertical.Set(horizontalZoom);
        this->ignoreZoom_ = false;
    }

    this->RecenterView();
}


void ViewSettingsModel::OnVerticalZoom_(double verticalZoom)
{
    if (this->ignoreZoom_)
    {
        return;
    }

    if (this->linkZoom.Get())
    {
        this->ignoreZoom_ = true;
        this->scale.horizontal.Set(verticalZoom);
        this->ignoreZoom_ = false;
    }

    this->RecenterView();
}


void ViewSettingsModel::OnLinkZoom_(bool isLinked)
{
    if (isLinked)
    {
        this->ignoreZoom_ = true;
        this->scale.vertical.Set(this->scale.horizontal.Get());
        this->ignoreZoom_ = false;
    }
}


void ViewSettingsModel::OnViewPosition_(const Point &)
{
    if (this->ignoreViewPosition_)
    {
        return;
    }

    this->SetImageCenterPixel_(this->ComputeImageCenterPixel());
}


void ViewSettingsModel::OnImageSize_(const Size &imageSize_)
{
    // Reset the view when the image size changes.
    this->ResetView_(imageSize_);
}


void ViewSettingsModel::ResetView_(const Size &imageSize_)
{
    this->SetImageCenterPixel_(
        imageSize_.template Convert<double>().ToPoint2d() / 2.0);

    this->RecenterView();
}


void ViewSettingsModel::SetViewPosition_(const Point &point)
{
    // We need to notify observers of the change to view position without
    // calling our own handler `OnViewPosition_`
    this->ignoreViewPosition_ = true;
    this->viewPosition.Set(point);
    this->ignoreViewPosition_ = false;
}


void ViewSettingsModel::RecenterView()
{
    this->SetViewPosition_(this->GetViewPositionFromCenterImagePixel());
}


} // end namespace draw



template struct pex::Group
    <
        draw::ViewFields,
        draw::ViewTemplate,
        draw::ViewSettings
    >;


template struct pex::MakeGroup
    <
        draw::ViewSettingsGroup,
        draw::ViewSettingsModel
    >;
