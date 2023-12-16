#pragma once


#include "draw/quad_shape.h"
#include "draw/views/node_settings_view.h"


namespace draw
{


class QuadShapeView: public NodeSettingsView
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
