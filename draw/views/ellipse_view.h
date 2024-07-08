#pragma once


#include <wxpex/labeled_widget.h>
#include "draw/ellipse.h"
#include <wxpex/static_box.h>


namespace draw
{


class EllipseView: public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    EllipseView(
        wxWindow *parent,
        EllipseControl control,
        const LayoutOptions &layoutOptions);

    EllipseView(
        wxWindow *parent,
        const std::string &name,
        EllipseControl control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
