#include "draw/views/polygon_shape_view.h"


namespace draw
{


PolygonShapeView::PolygonShapeView(
    wxWindow *parent,
    const std::string &name,
    PolygonShapeControl control,
    const LayoutOptions &layoutOptions)
    :
    NodeSettingsView(parent, name, control.node)
{
    using namespace wxpex;

    auto polygon =
        new PolygonView(
            this->GetPanel(),
            "Polygon",
            control.shape,
            layoutOptions);

    auto look =
        new LookView(this->GetPanel(), "Look", control.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        polygon,
        look);

    this->ConfigureTopSizer(std::move(sizer));
}



} // end namespace draw
