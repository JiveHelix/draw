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
            control.Get().GetValueBase()->GetName(),
            control.GetControlBase()->GetNode())
    {
        auto shape =
            control.GetControlBase()->CreateShapeView(this->GetPanel());

        auto look =
            control.GetControlBase()->CreateLookView(this->GetPanel());

        this->ConfigureTopSizer(
            wxpex::LayoutItems(wxpex::verticalItems, shape, look));
    }
};


} // end namespace draw
