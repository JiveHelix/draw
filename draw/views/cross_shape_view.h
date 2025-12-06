#pragma once


#include <wxpex/collapsible.h>
#include "draw/cross_shape.h"
#include "draw/views/cross_view.h"
#include "draw/views/look_view.h"


namespace draw
{


class CrossShapeView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    CrossShapeView(
        wxWindow *parent,
        const std::string &name,
        const CrossShapeControl &control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
