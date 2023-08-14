#pragma once


#include <pex/group.h>
#include <wxpex/async.h>
#include <wxpex/modifier.h>
#include <wxpex/cursor.h>
#include "draw/pixels.h"
#include "draw/shapes.h"
#include "draw/views/view_settings.h"
#include "draw/pixels.h"


namespace draw
{


template<typename T>
struct PixelViewFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::viewSettings, "viewSettings"),
        fields::Field(&T::mousePosition, "mousePosition"),
        fields::Field(&T::logicalPosition, "logicalPosition"),
        fields::Field(&T::mouseDown, "mouseDown"),
        fields::Field(&T::rightMouseDown, "rightMouseDown"),
        fields::Field(&T::modifier, "modifier"),
        fields::Field(&T::cursor, "cursor"),
        fields::Field(&T::pixels, "pixels"),
        fields::Field(&T::shapes, "shapes"));
};


template<template<typename> typename T>
struct PixelViewTemplate
{
    using AsyncPixels = wxpex::MakeAsync<std::shared_ptr<draw::Pixels>>;
    using AsyncShapes = wxpex::MakeAsync<Shapes>;

    T<ViewSettingsGroupMaker> viewSettings;
    T<PointGroupMaker> mousePosition;
    T<PointGroupMaker> logicalPosition;
    T<bool> mouseDown;
    T<bool> rightMouseDown;
    T<wxpex::Modifier> modifier;
    T<wxpex::Cursor> cursor;
    T<AsyncPixels> pixels;
    T<AsyncShapes> shapes;

    static constexpr auto fields = PixelViewFields<PixelViewTemplate>::fields;
    static constexpr auto fieldsTypeName = "PixelView";
};


using PixelViewGroup = pex::Group<PixelViewFields, PixelViewTemplate>;

using PixelViewSettings = typename PixelViewGroup::Plain;

struct PixelViewModel: public PixelViewGroup::Model
{
    PixelViewModel();
    void OnViewSettings_(const ViewSettings &settings);
    void OnMousePosition_(const Point &point);

private:
    pex::Endpoint<PixelViewModel, ViewSettingsControl> viewSettings_;
    pex::Endpoint<PixelViewModel, PointControl> mousePosition_;
};


struct PixelViewControl: public PixelViewGroup::Control
{
    using AsyncPixelsControl =
        typename PixelViewTemplate::AsyncPixels::Control;

    using AsyncShapesControl =
        typename PixelViewTemplate::AsyncShapes::Control;

    AsyncPixelsControl asyncPixels;
    AsyncShapesControl asyncShapes;

    PixelViewControl() = default;

    PixelViewControl(PixelViewModel &dataViewModel);
};


using PixelViewGroupMaker = pex::MakeGroup
    <
        PixelViewGroup,
        PixelViewModel,
        PixelViewControl
    >;



} // end namespace draw


extern template struct pex::Group
    <
        draw::PixelViewFields,
        draw::PixelViewTemplate
    >;


extern template struct pex::MakeGroup
    <
        draw::PixelViewGroup,
        draw::PixelViewModel,
        draw::PixelViewControl
    >;
