#pragma once


#include "draw/views/node_settings_view.h"
#include "draw/views/shape_display.h"



namespace draw
{




template<typename Control>
class ShapeView: public NodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ShapeView(
        wxWindow *parent,
        Control control,
        ShapeDisplayControl displayControl)
        :
        NodeSettingsView(
            parent,
            control.GetName(),
            displayControl.shapeExpand,
            control.GetNode())
    {
        auto shape = control.CreateShapeView(this->GetPanel());

        auto look = control.CreateLookView(
            this->GetPanel(),
            displayControl.lookExpand);

        this->ConfigureTopSizer(
            wxpex::LayoutItems(wxpex::verticalItems, shape, look));
    }
};


} // end namespace draw
