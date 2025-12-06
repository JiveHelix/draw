#pragma once


#include <wxpex/collapsible.h>
#include "draw/ellipse_shape.h"


namespace draw
{


class EllipseShapeView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    EllipseShapeView(
        wxWindow *parent,
        const std::string &name,
        const EllipseShapeControl &control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
