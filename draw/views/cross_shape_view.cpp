#include "draw/views/cross_shape_view.h"

#include <wxpex/layout_items.h>


namespace draw
{


CrossShapeView::CrossShapeView(
    wxWindow *parent,
    const std::string &name,
    CrossShapeControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name)
{
    using namespace wxpex;

    auto cross =
        new CrossView(
            this->GetPanel(),
            "Cross",
            control.shape,
            layoutOptions);

    auto look =
        new StrokeOnlyView(
            this->GetPanel(),
            "Stroke",
            control.look.stroke,
            layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        cross,
        look);

    this->ConfigureSizer(std::move(sizer));
}



} // end namespace draw
