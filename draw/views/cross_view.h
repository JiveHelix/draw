#pragma once


#include <pex/endpoint.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/knob.h>
#include <wxpex/converter.h>
#include <wxpex/static_box.h>

#include "draw/cross.h"


namespace draw
{


class CrossView: public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    CrossView(
        wxWindow *parent,
        const std::string &name,
        CrossControl control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
