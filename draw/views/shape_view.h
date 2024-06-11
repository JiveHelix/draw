#pragma once


#include <wxpex/button.h>
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

        auto depthOrder = control.GetDepthOrder();

        auto down =
            new wxpex::Button(this->GetPanel(), "Down", depthOrder.moveDown);

        auto up =
            new wxpex::Button(this->GetPanel(), "Up", depthOrder.moveUp);

        auto upDownSizer = wxpex::LayoutItems(wxpex::horizontalItems, down, up);

        this->ConfigureTopSizer(
            wxpex::LayoutItems(
                wxpex::verticalItems,
                shape,
                look,
                upDownSizer.release()));
    }
};


} // end namespace draw
