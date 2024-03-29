#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/static_box.h>
#include "draw/look.h"


namespace draw
{


class LookView: public wxpex::StaticBox
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


class StrokeView: public wxpex::StaticBox
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
