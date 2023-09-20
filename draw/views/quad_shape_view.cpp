#include "draw/views/quad_shape_view.h"

#include "draw/views/look_view.h"
#include "draw/views/quad_view.h"


namespace draw
{


QuadShapeView::QuadShapeView(
    wxWindow *parent,
    const std::string &name,
    QuadShapeControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name)
{
    auto quad =
        new QuadView(this->GetPanel(), "Quad", control.quad, layoutOptions);

    auto look =
        new LookView(this->GetPanel(), "Look", control.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        quad,
        look);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace draw
