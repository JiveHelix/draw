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
        fields::Field(&T::window, "window"),
        fields::Field(&T::mouseDown, "mouseDown"),
        fields::Field(&T::rightMouseDown, "rightMouseDown"),
        fields::Field(&T::modifier, "modifier"),
        fields::Field(&T::keyCode, "keyCode"),
        fields::Field(&T::cursor, "cursor"),
        fields::Field(&T::menuId, "menuId"));
};


template<template<typename> typename T>
struct CanvasTemplate
{
    T<ViewSettingsGroup> viewSettings;
    T<PointGroup> mousePosition;
    T<PointGroup> logicalPosition;
    T<wxWindow *> window;
    T<bool> mouseDown;
    T<bool> rightMouseDown;
    T<wxpex::Modifier> modifier;
    T<int> keyCode;
    T<wxpex::Cursor> cursor;
    T<wxWindowID> menuId;

    static constexpr auto fields = CanvasFields<CanvasTemplate>::fields;
    static constexpr auto fieldsTypeName = "CanvasSettings";
};


struct CanvasCustom
{
    template<typename Base>
    struct Model: public Base
    {
        Model()
            :
            Base(),

            viewSettings_(
                PEX_THIS("CanvasModel"),
                this->viewSettings,
                &Model::OnViewSettings_),

            mousePosition_(
                this,
                this->mousePosition,
                &Model::OnMousePosition_)
        {
            PEX_MEMBER(viewSettings_);
            PEX_MEMBER(mousePosition_);

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

    template<typename Base>
    struct Control: public Base
    {
        using Base::Base;

        Control()
            :
            Base()
        {
            PEX_NAME("CanvasControl");
        }

        Control(typename Base::Upstream &upstream)
            :
            Base(upstream)
        {
            PEX_NAME("CanvasControl");
        }

        void Emplace(typename Base::Upstream &upstream)
        {
            this->StandardEmplace_(upstream);
        }

        void Emplace(const Control &other)
        {
            this->StandardEmplace_(other);
        }
    };
};


using CanvasGroup =
    pex::Group<CanvasFields, CanvasTemplate, CanvasCustom>;

using CanvasSettings = typename CanvasGroup::Plain;
using CanvasModel = typename CanvasGroup::Model;
using CanvasControl = typename CanvasGroup::DefaultControl;


} // end namespace draw


extern template struct pex::Group
    <
        draw::CanvasFields,
        draw::CanvasTemplate,
        draw::CanvasCustom
    >;
