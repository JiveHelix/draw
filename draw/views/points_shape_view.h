#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>

#include "draw/points_shape.h"


namespace draw
{


class PointsShapeView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    PointsShapeView(
        wxWindow *parent,
        const std::string &name,
        const PointsShapeControl &controls,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace draw
