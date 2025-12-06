#pragma once

#include <jive/scope_flag.h>
#include <fields/fields.h>
#include <pex/group.h>
#include <pex/endpoint.h>
#include <fmt/core.h>

#include <draw/size.h>
#include <draw/scale.h>
#include <draw/point.h>
#include <draw/views/keep_center.h>


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
        fields::Field(&T::imagePivot, "imagePivot"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::linkZoom, "linkZoom"),
        fields::Field(&T::resetZoom, "resetZoom"),
        fields::Field(&T::fitZoom, "fitZoom"),
        fields::Field(&T::recenter, "recenter"),

        fields::Field(&T::resetHorizontalZoom, "resetHorizontalZoom"),
        fields::Field(&T::resetVerticalZoom, "resetVerticalZoom"),

        fields::Field(&T::fitHorizontalZoom, "fitHorizontalZoom"),
        fields::Field(&T::fitVerticalZoom, "fitVerticalZoom"),

        fields::Field(&T::recenterHorizontal, "recenterHorizontal"),
        fields::Field(&T::recenterVertical, "recenterVertical"),

        fields::Field(&T::keepCenter, "keepCenter"));
};


template<template<typename> typename T>
struct ViewTemplate
{
    T<IntPointGroup> screenPosition;
    T<SizeGroup> imageSize;
    T<SizeGroup> viewSize;
    T<SizeGroup> windowSize;
    T<SizeGroup> virtualSize;
    T<IntPointGroup> viewPosition;
    T<PointGroup> imagePivot;
    T<ScaleGroup> scale;
    T<bool> linkZoom;
    T<pex::MakeSignal> resetZoom;
    T<pex::MakeSignal> fitZoom;
    T<pex::MakeSignal> recenter;
    T<pex::MakeSignal> resetHorizontalZoom;
    T<pex::MakeSignal> resetVerticalZoom;
    T<pex::MakeSignal> fitHorizontalZoom;
    T<pex::MakeSignal> fitVerticalZoom;
    T<pex::MakeSignal> recenterHorizontal;
    T<pex::MakeSignal> recenterVertical;
    T<pex::MakeSelect<KeepCenterChoices>> keepCenter;

    static constexpr auto fields = ViewFields<ViewTemplate>::fields;
    static constexpr auto fieldsTypeName = "View";
};


struct ViewBrain
{
public:
    using ModelMembers = ViewTemplate<pex::ModelSelector>;

    static constexpr auto observerName = "ViewBrain";

private:
    ModelMembers &model_;
    pex::detail::MuteControl muteControl_;

    using ScaleEndpoint = pex::EndpointGroup<ViewBrain, ScaleControl>;
    using SizeEndpoint = pex::Endpoint<ViewBrain, SizeControl>;

    using DimensionEndpoint =
        pex::Endpoint<ViewBrain, decltype(SizeControl::width)>;

    using CoordinateEndpoint =
        pex::Endpoint<ViewBrain, decltype(IntPointControl::x)>;

    using PivotEndpoint = pex::Endpoint<ViewBrain, PointControl>;

    CoordinateEndpoint viewPositionX_;
    CoordinateEndpoint viewPositionY_;
    ScaleEndpoint scaleEndpoint_;
    SizeEndpoint imageSizeEndpoint_;
    DimensionEndpoint viewSizeWidth_;
    DimensionEndpoint viewSizeHeight_;
    PivotEndpoint pivotEndpoint_;

    pex::Endpoint<ViewBrain, pex::model::Value<bool>> linkZoomEndpoint_;

    using SignalEndpoint = pex::Endpoint<ViewBrain, pex::model::Signal>;

    SignalEndpoint resetZoomEndpoint_;
    SignalEndpoint fitZoomEndpoint_;
    SignalEndpoint recenterEndpoint_;
    SignalEndpoint resetHorizontalZoomEndpoint_;
    SignalEndpoint resetVerticalZoomEndpoint_;
    SignalEndpoint fitHorizontalZoomEndpoint_;
    SignalEndpoint fitVerticalZoomEndpoint_;
    SignalEndpoint recenterHorizontalEndpoint_;
    SignalEndpoint recenterVerticalEndpoint_;

    bool ignoreZoom_;
    bool ignoreViewPosition_;
    bool ignoreSize_;

    std::string name_;

public:
    ViewBrain(const ViewBrain &) = delete;
    ViewBrain & operator=(const ViewBrain &) = delete;

    ViewBrain(
        ModelMembers &model,
        const pex::detail::MuteControl &muteControl);

    void SetName(const std::string &name);

    void ResetZoom();

    void FitZoom();

    void Recenter();

    void HoldPivot(const Size &viewSize_);

    void HoldPivot();

    void HoldPivotX(const Size &viewSize_);

    void HoldPivotX();

    void HoldPivotY(const Size &viewSize_);

    void HoldPivotY();

    void OnResetHorizontalZoom_();

    void OnResetVerticalZoom_();

    void OnFitHorizontalZoom_();

    void OnFitVerticalZoom_();

    void OnRecenterHorizontal_();

    void OnRecenterVertical_();

    void UpdateVirtualSize();

private:
    IntPoint GetViewPosition_(const Size &viewSize_);

    int GetViewPositionX_(const Size &viewSize_);

    int GetViewPositionY_(const Size &viewSize_);

    void SetImagePivot_(const tau::Point2d<double> &point);

    double ComputeImagePivotX_() const;

    double ComputeImagePivotY_() const;

    void OnHorizontalZoom_(double horizontalZoom);

    void OnVerticalZoom_(double verticalZoom);

    void OnLinkZoom_(bool isLinked);

    void OnViewPositionX_(int);

    void OnViewPositionY_(int);

    void OnImageSize_(const Size &imageSize_);

    void OnViewSizeWidth_(int);

    void OnViewSizeHeight_(int);

    void OnImagePivot_(const Point &pivot);

    void ResetView_(const Size &imageSize_, const Size &viewSize_);
};


IntPoint GetMaximumViewPosition(const Size &viewSize, const Size &virtualSize);


struct ViewSettingsCustom
{
    template<typename Base>
    struct Plain: public Base
    {
        static constexpr int defaultWidth = 1920;
        static constexpr int defaultHeight = 1080;

        Plain()
            :
            Base{
                IntPoint{0, 0},
                Size{defaultWidth, defaultHeight},
                Size{defaultWidth, defaultHeight},
                Size{defaultWidth, defaultHeight},
                Size{defaultWidth, defaultHeight},
                IntPoint{0, 0},
                {double(defaultWidth) / 2.0, double(defaultHeight) / 2.0},
                Scale(1.0, 1.0),
                true,
                {},
                {},
                {},
                {},
                {},
                {},
                {},
                {},
                {},
                KeepCenter::none}
        {

        }

        // Compute the coordinates of an unscaled point using current zoom.
        Point GetLogicalPosition(const Point &point) const
        {
            return (point + this->viewPosition.template Cast<double>())
                / this->scale;
        }
    };


    template<typename Base>
    struct Model: public Base
    {
    public:
        Model()
            :
            Base{},
            viewBrain_(*this, this->CloneMuteNode())
        {

        }

        void SetName(const std::string &name)
        {
            this->viewBrain_.SetName(name);
        }

        void FitZoom()
        {
            this->viewBrain_.FitZoom();
        }

        void ResetZoom()
        {
            this->viewBrain_.ResetZoom();
        }

        void Recenter()
        {
            this->viewBrain_.Recenter();
        }

    private:
        ViewBrain viewBrain_;
    };


    template<typename Base>
    struct Control: public Base
    {
        using Base::Base;

        Control(typename Base::Upstream &upstream)
            :
            Base(upstream)
        {
            PEX_NAME("ViewSettingsControl");
        }

        void Emplace(const Control &other)
        {
            this->StandardEmplace_(other);
        }
    };
};


using ViewSettingsGroup =
    pex::Group<ViewFields, ViewTemplate, ViewSettingsCustom>;


using ViewSettings = typename ViewSettingsGroup::Plain;
using ViewSettingsModel = typename ViewSettingsGroup::Model;
using ViewSettingsControl = typename ViewSettingsGroup::DefaultControl;

using TestViewSettingsControl = typename TypeTester<ViewSettingsControl>::Type;
constexpr auto forceUpstreamControl = sizeof(TestViewSettingsControl);

template<typename Observer>
using ViewSettingsEndpoint = pex::EndpointGroup<Observer, ViewSettingsControl>;


DECLARE_OUTPUT_STREAM_OPERATOR(ViewSettings)


} // end namespace draw
