#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>

#include "draw/lines_shape.h"


namespace draw
{


class LinesShapeView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    LinesShapeView(
        wxWindow *parent,
        const std::string &name,
        const LinesShapeControl &controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace draw
