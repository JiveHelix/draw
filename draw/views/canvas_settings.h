#pragma once


#include <pex/group.h>
#include <wxpex/async.h>
#include <wxpex/modifier.h>
#include <wxpex/cursor.h>
#include "draw/views/view_settings.h"


namespace draw
{


template<typename T>
struct CanvasFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::viewSettings, "viewSettings"),
        fields::Field(&T::mousePosition, "mousePosition"),
        fields::Field(&T::logicalPosition, "logicalPosition"),
        fields::Field(&T::mouseDown, "mouseDown"),
        fields::Field(&T::rightMouseDown, "rightMouseDown"),
        fields::Field(&T::modifier, "modifier"),
        fields::Field(&T::cursor, "cursor"));
};


template<template<typename> typename T>
struct CanvasTemplate
{
    T<ViewSettingsGroup> viewSettings;
    T<PointGroup> mousePosition;
    T<PointGroup> logicalPosition;
    T<bool> mouseDown;
    T<bool> rightMouseDown;
    T<wxpex::Modifier> modifier;
    T<wxpex::Cursor> cursor;

    static constexpr auto fields = CanvasFields<CanvasTemplate>::fields;
    static constexpr auto fieldsTypeName = "CanvasSettings";
};


struct CanvasCustom
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
};


using CanvasGroup =
    pex::Group<CanvasFields, CanvasTemplate, CanvasCustom>;

using CanvasSettings = typename CanvasGroup::Plain;
using CanvasModel = typename CanvasGroup::Model;
using CanvasControl = typename CanvasGroup::Control;


} // end namespace draw


extern template struct pex::Group
    <
        draw::CanvasFields,
        draw::CanvasTemplate,
        draw::CanvasCustom
    >;
