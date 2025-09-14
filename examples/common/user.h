#pragma once


#include <fields/core.h>
#include <pex/group.h>
#include <pex/endpoint.h>
#include <wxpex/shortcut.h>
#include <draw/views/pixel_view_settings.h>


template<typename T>
struct UserFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::openFile, "openFile"),
        fields::Field(&T::fileName, "fileName"),
        fields::Field(&T::saveSettings, "saveSettings"),
        fields::Field(&T::loadSettings, "loadSettings"),
        fields::Field(&T::quit, "quit"),
        fields::Field(&T::about, "about"),
        fields::Field(&T::showPixelViewLayout, "showPixelViewLayout"),
        fields::Field(&T::pixelView, "pixelView"),
        fields::Field(&T::errors, "errors"));
};


template<template<typename> typename T>
struct UserTemplate
{
    T<pex::MakeSignal> openFile;
    T<std::string> fileName;
    T<pex::MakeSignal> saveSettings;
    T<pex::MakeSignal> loadSettings;
    T<pex::MakeSignal> quit;
    T<pex::MakeSignal> about;
    T<pex::MakeSignal> showPixelViewLayout;
    T<draw::PixelViewGroup> pixelView;
    T<std::string> errors;

    static constexpr auto fields = UserFields<UserTemplate>::fields;
    static constexpr auto fieldsTypeName = "User";
};


struct UserCustom
{
    template<typename Base>
    struct Model: public Base
    {
        Model()
            :
            Base{}
        {
            PEX_NAME("UserModel");
        }
    };
};


using UserGroup = pex::Group<UserFields, UserTemplate, UserCustom>;
using UserControl = typename UserGroup::DefaultControl;
using UserModel = typename UserGroup::Model;


template<typename Observer>
using UserEndpoints = pex::EndpointGroup<Observer, UserControl>;


wxpex::ShortcutGroups MakeShortcuts(UserControl userControl);
