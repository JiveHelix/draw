#pragma once

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
        fields::Field(&T::imageSize, "imageSize"),
        fields::Field(&T::viewSize, "viewSize"),
        fields::Field(&T::viewPosition, "viewPosition"),
        fields::Field(&T::imageCenterPixel, "imageCenterPixel"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::linkZoom, "linkZoom"),
        fields::Field(&T::resetZoom, "resetZoom"),
        fields::Field(&T::fitZoom, "fitZoom"));
};


template<template<typename> typename T>
struct ViewTemplate
{
    T<SizeGroup> imageSize;
    T<SizeGroup> viewSize;
    T<PointGroup> viewPosition;
    T<tau::Point2dGroup<double>> imageCenterPixel;
    T<ScaleGroup> scale;
    T<bool> linkZoom;
    T<pex::MakeSignal> resetZoom;
    T<pex::MakeSignal> fitZoom;

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
                Size{defaultWidth, defaultHeight},
                Size{defaultWidth, defaultHeight},
                Point{0, 0},
                Point{0, 0},
                Scale(1.0, 1.0),
                true,
                {},
                {}};
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

        pex::Endpoint<Model, PointControl> viewPositionEndpoint_;
        ScaleEndpoint scaleEndpoint_;
        pex::Endpoint<Model, SizeControl> imageSizeEndpoint_;
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

            this->ResetView_(this->imageSize.Get());
        }

        void SetImageCenterPixel_(const tau::Point2d<double> &point)
        {
            this->imageCenterPixel.Set(point);
        }

        tau::Point2d<double> ComputeImageCenterPixel() const
        {
            auto viewPosition_ = this->viewPosition.Get().template Convert<double>();

            auto halfView =
                this->viewSize.Get().ToPoint2d().template Convert<double>() / 2.0;

            auto viewCenterPixel = viewPosition_ + halfView;

            auto imageCenterPixel_ = viewCenterPixel / this->scale.Get();

            auto asIntegers = imageCenterPixel_.template Convert<int, tau::Floor>();
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

        Point GetViewPositionFromCenterImagePixel() const
        {
            auto scaledCenterPixel =
                this->imageCenterPixel.Get() * this->scale.Get();

            auto halfView =
                (this->viewSize.Get().ToPoint2d().template Convert<double>() / 2.0);

            auto result = (scaledCenterPixel - halfView);

            return result.template Convert<int, tau::Round>();
        }

        void ResetZoom()
        {
            this->scale.Set(Scale{});
            this->ResetView_(this->imageSize.Get());
        }

        void FitZoom()
        {
            auto imageSize_ = tau::Size<double>(this->imageSize.Get());
            auto viewSize_ = this->viewSize.Get().template Convert<double>();

            // imageSize_ * fit = viewSize_
            viewSize_ /= imageSize_;
            auto fit = Scale{viewSize_.height, viewSize_.width};
            auto scaleDeferred = pex::MakeDefer(this->scale);

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

        void RecenterView()
        {
            this->SetViewPosition_(this->GetViewPositionFromCenterImagePixel());
        }

    private:
        void SetViewPosition_(const Point &point)
        {
            // We need to notify observers of the change to view position
            // without calling our own handler `OnViewPosition_`
            this->ignoreViewPosition_ = true;
            this->viewPosition.Set(point);
            this->ignoreViewPosition_ = false;
        }

        void OnHorizontalZoom_(double horizontalZoom)
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

        void OnVerticalZoom_(double verticalZoom)
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
            this->ResetView_(imageSize_);
        }

        void ResetView_(const Size &imageSize_)
        {
            this->SetImageCenterPixel_(
                imageSize_.ToPoint2d().template Convert<double>() / 2.0);

            this->RecenterView();
        }
    };
};


using ViewSettingsGroup =
    pex::Group<ViewFields, ViewTemplate, ViewGroupTemplates_>;


using ViewSettings = typename ViewSettingsGroup::Plain;
using ViewSettingsModel = typename ViewSettingsGroup::Model;
using ViewSettingsControl = typename ViewSettingsGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(ViewSettings)


} // end namespace draw


extern template struct pex::Group
    <
        draw::ViewFields,
        draw::ViewTemplate,
        draw::ViewGroupTemplates_
    >;
