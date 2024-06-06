#pragma once


#include "draw/views/node_settings_view.h"



namespace draw
{


template<typename Control>
class ShapeView: public NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ShapeView(
        wxWindow *parent,
        Control control)
        :
        NodeSettingsView(
            parent,
            control.GetName(),
            control.GetNode())
    {
        auto shape = control.CreateShapeView(this->GetPanel());
        auto look = control.CreateLookView(this->GetPanel());

        this->ConfigureTopSizer(
            wxpex::LayoutItems(wxpex::verticalItems, shape, look));
    }
};


} // end namespace draw
