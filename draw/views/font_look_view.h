#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/static_box.h>
#include "draw/font_look.h"


namespace draw
{


class FontLookView: public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    FontLookView(
        wxWindow *parent,
        const std::string &name,
        FontLookControl control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
