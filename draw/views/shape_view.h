#pragma once


#include <wxpex/button.h>
#include <wxpex/collapsible.h>
#include <wxpex/layout_items.h>
#include "draw/views/node_settings_view.h"
#include "draw/views/shape_display.h"
#include "draw/views/order_view.h"



namespace draw
{


template<typename Control>
class ShapeView: public CollapsibleNodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ShapeView(
        wxWindow *parent,
        Control control,
        ShapeDisplayControl displayControl)
        :
        CollapsibleNodeSettingsView(
            parent,
            control.GetName(),
            displayControl.shapeExpand,
            control.GetNode())
    {
        auto shape = control.CreateShapeView(this->GetPanel());

        auto look = control.CreateLookView(
            this->GetPanel(),
            displayControl.lookExpand);

        auto orderView = new OrderView(this->GetPanel(), control.GetOrder());

        this->ConfigureSizer(
            wxpex::LayoutItems(
                wxpex::verticalItems,
                shape,
                look,
                orderView));
    }
};


} // end namespace draw
