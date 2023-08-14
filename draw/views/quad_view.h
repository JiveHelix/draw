#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/static_box.h>
#include "draw/quad.h"



namespace draw
{


class QuadView: public wxpex::StaticBox
{
public:
    QuadView(
        wxWindow *parent,
        const std::string &name,
        QuadControl control,
        const wxpex::LayoutOptions &layoutOptions);
};


} // end namespace draw
