#pragma once


#include <pex/group.h>
#include <wxpex/async.h>
#include <wxpex/modifier.h>
#include <wxpex/cursor.h>
#include "draw/views/view_settings.h"


namespace draw
{


template<template<typename> typename T>
struct CanvasSchema
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

    static constexpr auto fieldsTypeName = "CanvasSettings";
};


struct CanvasFinisher
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
    pex::Group<CanvasSchema, CanvasFinisher>;

using CanvasSettings = typename CanvasGroup::Plain;
using CanvasModel = typename CanvasGroup::Model;
using CanvasControl = typename CanvasGroup::DefaultControl;


} // end namespace draw


extern template struct pex::Group
    <
        draw::CanvasSchema,
        draw::CanvasFinisher
    >;
