#include "draw/views/points_shape_view.h"


#include <wxpex/slider.h>
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
        this->GetPane(),
        "radius",
        new wxpex::ValueSlider(
            this->GetPane(),
            controls.radius,
            controls.radius.value)).Layout(wxHORIZONTAL);

    auto look =
        new LookView(this->GetPane(), "Look", controls.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        radius.release(),
        look);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace draw
