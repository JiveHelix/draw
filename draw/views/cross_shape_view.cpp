#include "draw/views/cross_shape_view.h"


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
            control.cross,
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
