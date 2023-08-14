#include "draw/views/polygon_shape_view.h"


namespace draw
{


PolygonShapeView::PolygonShapeView(
    wxWindow *parent,
    const std::string &name,
    PolygonShapeControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name)
{
    using namespace wxpex;

    auto polygon =
        new PolygonView(
            this->GetPane(),
            "Polygon",
            control.polygon,
            layoutOptions);

    auto look =
        new LookView(this->GetPane(), "Look", control.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        polygon,
        look);

    this->ConfigureTopSizer(std::move(sizer));
}



} // end namespace draw
