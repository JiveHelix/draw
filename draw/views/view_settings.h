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
        fields::Field(&T::viewPosition, "viewPosition"),
        fields::Field(&T::imageCenterPixel, "imageCenterPixel"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::linkZoom, "linkZoom"),
        fields::Field(&T::resetZoom, "resetZoom"),
        fields::Field(&T::fitZoom, "fitZoom"),
        fields::Field(&T::bypass, "bypass"));
};


template<template<typename> typename T>
struct ViewTemplate
{
    T<PointGroup> screenPosition;
    T<SizeGroup> imageSize;
    T<SizeGroup> viewSize;
    T<SizeGroup> windowSize;
    T<PointGroup> viewPosition;
    T<tau::Point2dGroup<double>> imageCenterPixel;
    T<ScaleGroup> scale;
    T<bool> linkZoom;
    T<pex::MakeSignal> resetZoom;
    T<pex::MakeSignal> fitZoom;
    T<bool> bypass;

    static constexpr auto fields = ViewFields<ViewTemplate>::fields;
};


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
                Point{0, 0},
                {double(defaultWidth) / 2.0, double(defaultHeight) / 2.0},
                Scale(1.0, 1.0),
                true,
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

        pex::Endpoint<Model, PointControl> viewPositionEndpoint_;
        ScaleEndpoint scaleEndpoint_;
        SizeEndpoint imageSizeEndpoint_;
        SizeEndpoint viewSizeEndpoint_;
        pex::Terminus<Model, pex::model::Value<bool>> linkZoomTerminus_;
        pex::Terminus<Model, pex::model::Signal> resetZoomTerminus_;
        pex::Terminus<Model, pex::model::Signal> fitZoomTerminus_;
        bool ignoreZoom_;
        bool ignoreViewPosition_;

    public:
        Model()
            :
            GroupBase(),
            viewPositionEndpoint_(
                this,
                PointControl(this->viewPosition),
                &Model::OnViewPosition_),
            scaleEndpoint_(
                this,
                ScaleControl(this->scale)),
            imageSizeEndpoint_(this, this->imageSize, &Model::OnImageSize_),
            viewSizeEndpoint_(this, this->viewSize, &Model::OnViewSize_),
            linkZoomTerminus_(this, this->linkZoom),
            resetZoomTerminus_(this, this->resetZoom),
            fitZoomTerminus_(this, this->fitZoom),
            ignoreZoom_(false),
            ignoreViewPosition_(false)
        {
            this->scaleEndpoint_.horizontal.Connect(
                &Model::OnHorizontalZoom_);

            this->scaleEndpoint_.vertical.Connect(
                &Model::OnVerticalZoom_);

            this->linkZoomTerminus_.Connect(&Model::OnLinkZoom_);
            this->resetZoomTerminus_.Connect(&Model::ResetZoom);
            this->fitZoomTerminus_.Connect(&Model::FitZoom);

            this->ResetView_(this->imageSize.Get(), this->viewSize.Get());
        }

        void SetImageCenterPixel_(const tau::Point2d<double> &point)
        {
            this->imageCenterPixel.Set(point);
        }

        tau::Point2d<double> ComputeImageCenterPixel() const
        {
            auto viewPosition_ =
                this->viewPosition.Get().template Cast<double>();

            auto halfView =
                this->viewSize.Get().ToPoint2d().template Cast<double>() / 2.0;

            auto viewCenterPixel = viewPosition_ + halfView;

            auto imageCenterPixel_ = viewCenterPixel / this->scale.Get();

            auto asIntegers =
                imageCenterPixel_.template Cast<int, tau::Floor>();

            auto size = this->imageSize.Get();

            if (asIntegers.x >= size.width)
            {
                imageCenterPixel_.x = static_cast<double>(size.width - 1);
            }
            else if (asIntegers.x < 0)
            {
                imageCenterPixel_.x = 0.0;
            }

            if (asIntegers.y >= size.height)
            {
                imageCenterPixel_.y = static_cast<double>(size.height - 1);
            }
            else if (asIntegers.y < 0)
            {
                imageCenterPixel_.y = 0.0;
            }

            return imageCenterPixel_;
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

        void RecenterView(const Size &viewSize_)
        {
            auto scaledCenterPixel =
                this->imageCenterPixel.Get() * this->scale.Get();

            auto halfView =
                viewSize_.ToPoint2d().template Cast<double>() / 2.0;

            auto viewPosition_ = (scaledCenterPixel - halfView);

            // We need to notify observers of the change to view position
            // without calling our own handler `OnViewPosition_`
            this->ignoreViewPosition_ = true;

            this->viewPosition.Set(
                viewPosition_.template Cast<int, tau::Round>());

            this->ignoreViewPosition_ = false;
        }

    private:
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

            this->RecenterView(this->viewSize.Get());
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

            this->RecenterView(this->viewSize.Get());
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

        void OnViewPosition_(const Point &)
        {
            if (this->ignoreViewPosition_)
            {
                return;
            }

            this->SetImageCenterPixel_(this->ComputeImageCenterPixel());
        }

        void OnImageSize_(const Size &imageSize_)
        {
            // Reset the view when the image size changes.
            this->ResetView_(imageSize_, this->viewSize.Get());
        }

        void OnViewSize_(const Size &)
        {
            // Recompute the image center pixel.
            this->SetImageCenterPixel_(this->ComputeImageCenterPixel());
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
