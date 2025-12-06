#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/static_box.h>


#include "draw/regular_polygon.h"


namespace draw
{


class RegularPolygonView: public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    RegularPolygonView(
        wxWindow *parent,
        const RegularPolygonControl &control,
        const LayoutOptions &layoutOptions);

    RegularPolygonView(
        wxWindow *parent,
        const std::string &name,
        const RegularPolygonControl &control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
