#include "draw/views/ellipse_shape_view.h"

#include <wxpex/layout_items.h>
#include "draw/views/ellipse_view.h"
#include "draw/views/look_view.h"


namespace draw
{


EllipseShapeView::EllipseShapeView(
    wxWindow *parent,
    const std::string &name,
    const EllipseShapeControl &control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name)
{
    using namespace wxpex;

    auto ellipse =
        new EllipseView(
            this->GetPanel(),
            "Ellipse",
            control.shape,
            layoutOptions);

    auto look =
        new LookView(this->GetPanel(), "Look", control.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        ellipse,
        look);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace draw
