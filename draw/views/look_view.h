#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/static_box.h>
#include <wxpex/collapsible.h>
#include "draw/look.h"


namespace draw
{


template<typename T>
struct LookDisplayFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::lookExpand, "lookExpand"),
        fields::Field(&T::strokeExpand, "strokeExpand"),
        fields::Field(&T::fillExpand, "fillExpand"));
};


template<template<typename> typename T>
struct LookDisplayTemplate
{
    T<bool> lookExpand;
    T<bool> strokeExpand;
    T<bool> fillExpand;

    static constexpr auto fields =
        LookDisplayFields<LookDisplayTemplate>::fields;
};


using LookDisplayGroup = pex::Group<LookDisplayFields, LookDisplayTemplate>;
using LookDisplayControl = typename LookDisplayGroup::Control;


class StrokeView: public wxpex::Collapsible
{
public:
    StrokeView(
        wxWindow *parent,
        StrokeControl control,
        const wxpex::LayoutOptions &layoutOptions);

    StrokeView(
        wxWindow *parent,
        StrokeControl control,
        wxpex::Collapsible::StateControl expandControl,
        const wxpex::LayoutOptions &layoutOptions);

protected:
    void Initialize_(StrokeControl);
};


class FillView: public wxpex::Collapsible
{
public:
    FillView(
        wxWindow *parent,
        FillControl control,
        const wxpex::LayoutOptions &layoutOptions);

    FillView(
        wxWindow *parent,
        FillControl control,
        wxpex::Collapsible::StateControl expandControl,
        const wxpex::LayoutOptions &layoutOptions);

protected:
    void Initialize_(FillControl);
};


class LookView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    LookView(
        wxWindow *parent,
        const std::string &name,
        LookControl control,
        LookDisplayControl displayControl,
        const LayoutOptions &layoutOptions);

    LookView(
        wxWindow *parent,
        LookControl control,
        LookDisplayControl displayControl,
        const LayoutOptions &layoutOptions);

    LookView(
        wxWindow *parent,
        const std::string &name,
        LookControl control,
        const LayoutOptions &layoutOptions);

    LookView(
        wxWindow *parent,
        LookControl control,
        const LayoutOptions &layoutOptions);
};


class StrokeOnlyView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    StrokeOnlyView(
        wxWindow *parent,
        const std::string &name,
        StrokeControl control,
        const LayoutOptions &layoutOptions);

    StrokeOnlyView(
        wxWindow *parent,
        StrokeControl control,
        const LayoutOptions &layoutOptions);

    StrokeOnlyView(
        wxWindow *parent,
        const std::string &name,
        StrokeControl control,
        LookDisplayControl displayControl,
        const LayoutOptions &layoutOptions);

    StrokeOnlyView(
        wxWindow *parent,
        StrokeControl control,
        LookDisplayControl displayControl,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
