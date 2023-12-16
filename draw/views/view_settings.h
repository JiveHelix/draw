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


struct ViewSettings: public ViewTemplate<pex::Identity>
{
    static constexpr int defaultWidth = 1920;
    static constexpr int defaultHeight = 1080;

    static ViewSettings Default()
    {
        return ViewSettings{{
            Size{{defaultWidth, defaultHeight}},
            Size{{defaultWidth, defaultHeight}},
            Point{{0, 0}},
            Point{{0, 0}},
            Scale(1.0, 1.0),
            true,
            {},
            {}}};
    }

    // Compute the coordinates of an unscaled point using current zoom.
    Point GetLogicalPosition(const Point &point) const;
};


using ViewSettingsGroup = pex::Group<ViewFields, ViewTemplate, ViewSettings>;


DECLARE_OUTPUT_STREAM_OPERATOR(ViewSettings)


struct ViewSettingsModel: public ViewSettingsGroup::Model
{
public:
    static constexpr auto observerName = "ViewSettingsModel";

private:
    using ScaleEndpoint = pex::EndpointGroup<ViewSettingsModel, ScaleControl>;

    pex::Endpoint<ViewSettingsModel, PointControl> viewPositionEndpoint_;
    ScaleEndpoint scaleEndpoint_;
    pex::Endpoint<ViewSettingsModel, SizeControl> imageSizeEndpoint_;
    pex::Terminus<ViewSettingsModel, pex::model::Value<bool>> linkZoomTerminus_;
    pex::Terminus<ViewSettingsModel, pex::model::Signal> resetZoomTerminus_;
    pex::Terminus<ViewSettingsModel, pex::model::Signal> fitZoomTerminus_;
    bool ignoreZoom_;
    bool ignoreViewPosition_;

public:
    ViewSettingsModel();

    void SetImageCenterPixel_(const tau::Point2d<double> &point);

    tau::Point2d<double> ComputeImageCenterPixel() const;

    Point GetViewPositionFromCenterImagePixel() const;

    void ResetZoom();

    void FitZoom();

    void RecenterView();

private:
    void SetViewPosition_(const Point &);

    void OnHorizontalZoom_(double horizontalZoom);

    void OnVerticalZoom_(double verticalZoom);

    void OnLinkZoom_(bool isLinked);

    void OnViewPosition_(const Point &);

    void OnImageSize_(const Size &);

    void ResetView_(const Size &imageSize);
};


using ViewSettingsGroupMaker =
    pex::MakeGroup<ViewSettingsGroup, ViewSettingsModel>;

using ViewSettingsControl = typename ViewSettingsGroup::Control;


} // end namespace draw



extern template struct pex::Group
    <
        draw::ViewFields,
        draw::ViewTemplate,
        draw::ViewSettings
    >;


extern template struct pex::MakeGroup
    <
        draw::ViewSettingsGroup,
        draw::ViewSettingsModel
    >;
