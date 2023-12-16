#pragma once


#include <wxpex/collapsible.h>
#include "draw/polygon_shape.h"
#include "draw/views/polygon_view.h"
#include "draw/views/look_view.h"
#include "draw/views/node_settings_view.h"


namespace draw
{


class PolygonShapeView: public NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    PolygonShapeView(
        wxWindow *parent,
        const std::string &name,
        PolygonShapeControl control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
