#include "draw/views/ellipse_shape_view.h"

#include "draw/views/ellipse_view.h"
#include "draw/views/look_view.h"


namespace draw
{


EllipseShapeView::EllipseShapeView(
    wxWindow *parent,
    const std::string &name,
    EllipseShapeControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name)
{
    using namespace wxpex;

    auto ellipse =
        new EllipseView(
            this->GetPanel(),
            "Ellipse",
            control.ellipse,
            layoutOptions);

    auto look =
        new LookView(this->GetPanel(), "Look", control.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        ellipse,
        look);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace draw
