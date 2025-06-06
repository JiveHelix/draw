#include "draw/views/points_shape_view.h"


#include <wxpex/slider.h>
#include <wxpex/layout_items.h>
#include "draw/views/look_view.h"


namespace draw
{


PointsShapeView::PointsShapeView(
    wxWindow *parent,
    const std::string &name,
    PointsShapeControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name)
{
    auto radius = wxpex::LabeledWidget(
        this->GetPanel(),
        "radius",
        new wxpex::ValueSlider(
            this->GetPanel(),
            controls.radius,
            controls.radius.value)).Layout(wxHORIZONTAL);

    auto look =
        new LookView(this->GetPanel(), "Look", controls.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        radius,
        look);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace draw
