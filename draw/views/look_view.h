#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/static_box.h>
#include <wxpex/collapsible.h>
#include "draw/look.h"


namespace draw
{


class StrokeControls: public wxpex::Collapsible
{
public:
    StrokeControls(
        wxWindow *parent,
        LookControl control,
        const wxpex::LayoutOptions &layoutOptions);
};


class FillControls: public wxpex::Collapsible
{
public:
    FillControls(
        wxWindow *parent,
        LookControl control,
        const wxpex::LayoutOptions &layoutOptions);
};


class LookView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

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


class StrokeView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    StrokeView(
        wxWindow *parent,
        const std::string &name,
        LookControl control,
        const LayoutOptions &layoutOptions);

    StrokeView(
        wxWindow *parent,
        LookControl control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
