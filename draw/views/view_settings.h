#pragma once

#include <jive/scope_flag.h>
#include <fields/fields.h>
#include <pex/group.h>
#include <pex/endpoint.h>

#include "draw/size.h"
#include "draw/scale.h"
#include "draw/point.h"


namespace draw
{


template<typename T>
struct ViewFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::screenPosition, "screenPosition"),
        fields::Field(&T::imageSize, "imageSize"),
        fields::Field(&T::viewSize, "viewSize"),
        fields::Field(&T::windowSize, "windowSize"),
        fields::Field(&T::virtualSize, "virtualSize"),
        fields::Field(&T::viewPosition, "viewPosition"),
        fields::Field(&T::imageCenterPixel, "imageCenterPixel"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::linkZoom, "linkZoom"),
        fields::Field(&T::resetZoom, "resetZoom"),
        fields::Field(&T::fitZoom, "fitZoom"),
        fields::Field(&T::recenter, "recenter"),
        fields::Field(&T::bypass, "bypass"),
        fields::Field(&T::recenterHorizontal, "recenterHorizontal"),
        fields::Field(&T::recenterVertical, "recenterVertical"));
};


template<template<typename> typename T>
struct ViewTemplate
{
    T<PointGroup> screenPosition;
    T<SizeGroup> imageSize;
    T<SizeGroup> viewSize;
    T<SizeGroup> windowSize;
    T<SizeGroup> virtualSize;
    T<PointGroup> viewPosition;
    T<tau::Point2dGroup<double>> imageCenterPixel;
    T<ScaleGroup> scale;
    T<bool> linkZoom;
    T<pex::MakeSignal> resetZoom;
    T<pex::MakeSignal> fitZoom;
    T<pex::MakeSignal> recenter;
    T<pex::MakeSignal> recenterHorizontal;
    T<pex::MakeSignal> recenterVertical;
    T<bool> bypass;

    static constexpr auto fields = ViewFields<ViewTemplate>::fields;
};


Point GetMaximumViewPosition(const Size &viewSize, const Size &virtualSize);



struct ViewGroupTemplates_
{
    template<typename GroupBase>
    struct Plain: public GroupBase
    {
        static constexpr int defaultWidth = 1920;
        static constexpr int defaultHeight = 1080;

        static Plain Default()
        {
            return Plain{
                Point{0, 0},
                Size{defaultWidth, defaultHeight},
                Size{defaultWidth, defaultHeight},
                Size{defaultWidth, defaultHeight},
                Size{defaultWidth, defaultHeight},
                Point{0, 0},
                {double(defaultWidth) / 2.0, double(defaultHeight) / 2.0},
                Scale(1.0, 1.0),
                true,
                {},
                {},
                {},
                {},
                {},
                false};
        }

        // Compute the coordinates of an unscaled point using current zoom.
        Point GetLogicalPosition(const Point &point) const
        {
            return (point + this->viewPosition) / this->scale;
        }
    };


    template<typename GroupBase>
    struct Model: public GroupBase
    {
    public:
        static constexpr auto observerName = "ViewSettingsModel";

    private:
        using ScaleEndpoint = pex::EndpointGroup<Model, ScaleControl>;
        using SizeEndpoint = pex::Endpoint<Model, SizeControl>;

        using DimensionEndpoint =
            pex::Endpoint<Model, decltype(SizeControl::width)>;

        using CoordinateEndpoint =
            pex::Endpoint<Model, decltype(PointControl::x)>;

        CoordinateEndpoint viewPositionX_;
        CoordinateEndpoint viewPositionY_;
        ScaleEndpoint scaleEndpoint_;
        SizeEndpoint imageSizeEndpoint_;
        DimensionEndpoint viewSizeWidth_;
        DimensionEndpoint viewSizeHeight_;

        pex::Endpoint<Model, pex::model::Value<bool>> linkZoomEndpoint_;

        using SignalEndpoint = pex::Endpoint<Model, pex::model::Signal>;

        SignalEndpoint resetZoomEndpoint_;
        SignalEndpoint fitZoomEndpoint_;
        SignalEndpoint recenterEndpoint_;
        SignalEndpoint recenterHorizontalEndpoint_;
        SignalEndpoint recenterVerticalEndpoint_;

        bool ignoreZoom_;
        bool ignoreViewPosition_;
        bool ignoreSize_;

    public:
        Model()
            :
            GroupBase(),

            viewPositionX_(
                this,
                PointControl(this->viewPosition).x,
                &Model::OnViewPositionX_),

            viewPositionY_(
                this,
                PointControl(this->viewPosition).y,
                &Model::OnViewPositionY_),

            scaleEndpoint_(
                this,
                ScaleControl(this->scale)),
            imageSizeEndpoint_(this, this->imageSize, &Model::OnImageSize_),

            viewSizeWidth_(
                this,
                this->viewSize.width,
                &Model::OnViewSizeWidth_),

            viewSizeHeight_(
                this,
                this->viewSize.height,
                &Model::OnViewSizeHeight_),

            linkZoomEndpoint_(this, this->linkZoom, &Model::OnLinkZoom_),
            resetZoomEndpoint_(this, this->resetZoom, &Model::ResetZoom),
            fitZoomEndpoint_(this, this->fitZoom, &Model::FitZoom),
            recenterEndpoint_(this, this->recenter, &Model::Recenter),

            recenterHorizontalEndpoint_(
                this,
                this->recenterHorizontal,
                &Model::RecenterHorizontalView),

            recenterVerticalEndpoint_(
                this,
                this->recenterVertical,
                &Model::RecenterVerticalView),

            ignoreZoom_(false),
            ignoreViewPosition_(false),
            ignoreSize_(false)
        {
            this->scaleEndpoint_.horizontal.Connect(
                &Model::OnHorizontalZoom_);

            this->scaleEndpoint_.vertical.Connect(
                &Model::OnVerticalZoom_);

            this->ResetView_(this->imageSize.Get(), this->viewSize.Get());
        }

        void ResetZoom()
        {
            this->scale.Set(Scale{});
            this->ResetView_(this->imageSize.Get(), this->viewSize.Get());
        }

        void FitZoom()
        {
            auto imageSize_ = tau::Size<double>(this->imageSize.Get());

            // Fit without scrollbars.
            auto windowSize_ = this->windowSize.Get().template Cast<double>();

            // imageSize_ * fit = windowSize_
            windowSize_ /= imageSize_;
            auto fit = Scale{windowSize_.height, windowSize_.width};

            if (this->linkZoom.Get())
            {
                double smaller = std::min(fit.horizontal, fit.vertical);
                fit.horizontal = smaller;
                fit.vertical = smaller;
            }

            jive::ScopeFlag ignoreZoom(this->ignoreZoom_);
            auto deferScale = pex::MakeDefer(this->scale);
            deferScale.Set(fit);

            // Reset the imageCenterPixel_ before the zoom scale is notified.
            this->ResetView_(this->imageSize.Get(), this->windowSize.Get());
        }

        void Recenter()
        {
            this->ResetView_(this->imageSize.Get(), this->windowSize.Get());

            jive::ScopeFlag ignoreZoom(this->ignoreZoom_);
            pex::AccessReference(this->scale).DoNotify();
        }

        Point GetCenteredViewPosition_(const Size &viewSize_)
        {
            auto scaledCenterPixel =
                this->imageCenterPixel.Get() * this->scale.Get();

            auto halfView =
                viewSize_.ToPoint2d().template Cast<double>() / 2.0;

            auto result = scaledCenterPixel - halfView;
            return result.template Cast<int, tau::Round>();
        }

        void RecenterView(const Size &viewSize_)
        {
            auto viewPosition_ = this->GetCenteredViewPosition_(viewSize_);

            // We need to notify observers of the change to view position
            // without calling our own handler `OnViewPosition_`
            jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
            this->viewPosition.Set(viewPosition_);
            this->UpdateVirtualSize();
        }

        void RecenterView()
        {
            this->RecenterView(this->viewSize.Get());
        }

        void RecenterHorizontalView()
        {
            auto viewPosition_ =
                this->GetCenteredViewPosition_(this->viewSize.Get());

            jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
            this->viewPosition.x.Set(viewPosition_.x);
            this->UpdateVirtualSize();
        }

        void RecenterVerticalView()
        {
            auto viewPosition_ =
                this->GetCenteredViewPosition_(this->viewSize.Get());

            jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
            this->viewPosition.y.Set(viewPosition_.y);
            this->UpdateVirtualSize();
        }

        void UpdateVirtualSize()
        {
            auto imageSize_ = this->imageSize.Get();
            auto virtualSize_ = imageSize_.template Cast<double>();

            virtualSize_.width *= this->scale.horizontal.Get();
            virtualSize_.height *= this->scale.vertical.Get();
            virtualSize_.width = std::floor(virtualSize_.width);
            virtualSize_.height = std::floor(virtualSize_.height);

            auto virtualAsInt = virtualSize_.template Cast<int>();
            auto viewPosition_ = this->viewPosition.Get();
            auto viewSize_ = this->viewSize.Get();

            // When the virtual panel is off screen to the top or left, the
            // position is positive.
            // When virtual panel begins to the right or bottom of the top left
            // corner, the position is negative.
            auto AdjustVirtualSize =
                [](auto position, auto virtualSize, auto viewSize)
                {
                    if (position > 0)
                    {
                        // Off screen to the left or top
                        // We must be able to scroll it back into view.
                        // THe virtualSize needs to as big as what is off
                        // screen plus the viewSize.
                        auto minimumVirtual = position + viewSize;
                        return std::max(minimumVirtual, virtualSize);
                    }

                    return virtualSize;
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

            this->virtualSize.Set(virtualAsInt);
        }

    private:
        void SetImageCenterPixel_(const tau::Point2d<double> &point)
        {
            this->imageCenterPixel.Set(point);
        }

        tau::Point2d<double> ComputeImageCenterPixel_() const
        {
            // TODO: This function runs once for each x/y change in view
            // position.
            // Refactor to compute the x/y image centers independently.
            auto viewPosition_ =
                this->viewPosition.Get().template Cast<double>();

            auto halfView =
                this->viewSize.Get().ToPoint2d().template Cast<double>() / 2.0;

            auto imageSize_ = this->imageSize.Get();
            auto scale_ = this->scale.Get();

            auto viewCenterPixel = viewPosition_ + halfView;

            auto imageCenterPixel_ = viewCenterPixel / scale_;

            auto asIntegers =
                imageCenterPixel_.template Cast<int, tau::Floor>();

            if (asIntegers.x >= imageSize_.width)
            {
                imageCenterPixel_.x = static_cast<double>(imageSize_.width - 1);
            }
            else if (asIntegers.x < 0)
            {
                imageCenterPixel_.x = 0.0;
            }

            if (asIntegers.y >= imageSize_.height)
            {
                imageCenterPixel_.y =
                    static_cast<double>(imageSize_.height - 1);
            }
            else if (asIntegers.y < 0)
            {
                imageCenterPixel_.y = 0.0;
            }

            return imageCenterPixel_;
        }

        void OnHorizontalZoom_(double horizontalZoom)
        {
            if (this->ignoreZoom_)
            {
                return;
            }

            if (this->bypass.Get())
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

        void OnVerticalZoom_(double verticalZoom)
        {
            if (this->ignoreZoom_)
            {
                return;
            }

            if (this->bypass.Get())
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

        void OnLinkZoom_(bool isLinked)
        {
            if (isLinked)
            {
                this->ignoreZoom_ = true;
                this->scale.vertical.Set(this->scale.horizontal.Get());
                this->ignoreZoom_ = false;
            }
        }

        void OnViewPositionX_(int)
        {
            if (this->ignoreViewPosition_)
            {
                return;
            }

            auto imageCenter = this->ComputeImageCenterPixel_();
            this->imageCenterPixel.x.Set(imageCenter.x);

            jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
            this->UpdateVirtualSize();
        }

        void OnViewPositionY_(int)
        {
            if (this->ignoreViewPosition_)
            {
                return;
            }

            auto imageCenter = this->ComputeImageCenterPixel_();
            this->imageCenterPixel.y.Set(imageCenter.y);

            jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
            this->UpdateVirtualSize();
        }

        void OnImageSize_(const Size &imageSize_)
        {
            // Reset the view when the image size changes.
            this->ResetView_(imageSize_, this->viewSize.Get());
        }

        void OnViewSizeWidth_(int)
        {
            // Recompute the image center pixel.
            auto imageCenter = this->ComputeImageCenterPixel_();
            this->imageCenterPixel.x.Set(imageCenter.x);

            if (this->ignoreSize_)
            {
                return;
            }

            jive::ScopeFlag ignoreSize(this->ignoreSize_);
            this->UpdateVirtualSize();
        }

        void OnViewSizeHeight_(int)
        {
            // Recompute the image center pixel.
            auto imageCenter = this->ComputeImageCenterPixel_();
            this->imageCenterPixel.y.Set(imageCenter.y);

            if (this->ignoreSize_)
            {
                return;
            }

            jive::ScopeFlag ignoreSize(this->ignoreSize_);
            this->UpdateVirtualSize();
        }

        void ResetView_(const Size &imageSize_, const Size &viewSize_)
        {
            if (this->bypass.Get())
            {
                return;
            }

            this->SetImageCenterPixel_(
                imageSize_.ToPoint2d().template Cast<double>() / 2.0);

            this->RecenterView(viewSize_);
        }
    };
};


using ViewSettingsGroup =
    pex::Group<ViewFields, ViewTemplate, ViewGroupTemplates_>;


using ViewSettings = typename ViewSettingsGroup::Plain;
using ViewSettingsModel = typename ViewSettingsGroup::Model;
using ViewSettingsControl = typename ViewSettingsGroup::Control;

template<typename Observer>
using ViewSettingsEndpoint = pex::EndpointGroup<Observer, ViewSettingsControl>;


DECLARE_OUTPUT_STREAM_OPERATOR(ViewSettings)


class ViewSettingsBypass
{
public:
    ViewSettingsBypass(ViewSettingsControl &viewSettings)
        :
        bypass_(viewSettings.bypass)
    {
        this->bypass_.Set(true);
    }

    ~ViewSettingsBypass()
    {
        this->bypass_.Set(false);
    }

private:
    using BoolControl = decltype(ViewSettingsControl::bypass);

    BoolControl &bypass_;
};


} // end namespace draw


extern template struct pex::Group
    <
        draw::ViewFields,
        draw::ViewTemplate,
        draw::ViewGroupTemplates_
    >;
