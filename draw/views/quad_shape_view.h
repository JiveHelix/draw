#pragma once


#include <wxpex/collapsible.h>
#include "draw/quad_shape.h"


namespace draw
{


class QuadShapeView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    QuadShapeView(
        wxWindow *parent,
        const std::string &name,
        QuadShapeControl control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
