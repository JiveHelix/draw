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

    T<ViewSettingsGroup> viewSettings;
    T<PointGroup> mousePosition;
    T<PointGroup> logicalPosition;
    T<bool> mouseDown;
    T<bool> rightMouseDown;
    T<wxpex::Modifier> modifier;
    T<wxpex::Cursor> cursor;
    T<AsyncPixels> pixels;
    T<AsyncShapes> shapes;

    static constexpr auto fields = PixelViewFields<PixelViewTemplate>::fields;
    static constexpr auto fieldsTypeName = "PixelView";
};


struct PixelViewTemplates
{
    template<typename GroupBase>
    struct Model: public GroupBase
    {
        Model()
            :
            GroupBase(),

            viewSettings_(
                this,
                this->viewSettings,
                &Model::OnViewSettings_),

            mousePosition_(
                this,
                this->mousePosition,
                &Model::OnMousePosition_)
        {
            this->OnViewSettings_(this->viewSettings.Get());
        }

        void OnViewSettings_(const ViewSettings &settings)
        {
            this->logicalPosition.Set(
                settings.GetLogicalPosition(this->mousePosition.Get()));
        }

        void OnMousePosition_(const Point &point)
        {
            this->logicalPosition.Set(
                this->viewSettings.Get().GetLogicalPosition(point));
        }

    private:
        pex::Endpoint<Model, ViewSettingsControl> viewSettings_;
        pex::Endpoint<Model, PointControl> mousePosition_;
    };


    template<typename GroupBase>
    struct Control: public GroupBase
    {
        using AsyncPixelsControl =
            typename GroupBase::AsyncPixels::Control;

        using AsyncShapesControl =
            typename GroupBase::AsyncShapes::Control;

        AsyncPixelsControl asyncPixels;
        AsyncShapesControl asyncShapes;

        Control() = default;

        Control(typename GroupBase::Upstream &dataViewModel)
            :
            GroupBase(dataViewModel),
            asyncPixels(dataViewModel.pixels.GetWorkerControl()),
            asyncShapes(dataViewModel.shapes.GetWorkerControl())
        {

        }
    };
};


using PixelViewGroup =
    pex::Group<PixelViewFields, PixelViewTemplate, PixelViewTemplates>;

using PixelViewSettings = typename PixelViewGroup::Plain;
using PixelViewModel = typename PixelViewGroup::Model;
using PixelViewControl = typename PixelViewGroup::Control;


} // end namespace draw


extern template struct pex::Group
    <
        draw::PixelViewFields,
        draw::PixelViewTemplate,
        draw::PixelViewTemplates
    >;
