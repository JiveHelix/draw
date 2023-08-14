#include "draw/views/lines_shape_view.h"


#include <wxpex/slider.h>
#include <wxpex/check_box.h>
#include "draw/views/look_view.h"


namespace draw
{


LinesShapeView::LinesShapeView(
    wxWindow *parent,
    const std::string &name,
    LinesShapeControl controls,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name)
{
    auto infinite =
        new wxpex::CheckBox(
            this->GetPane(),
            "infinite",
            controls.infinite);

    auto length = wxpex::LabeledWidget(
        this->GetPane(),
        "length",
        new wxpex::ValueSlider(
            this->GetPane(),
            controls.length,
            controls.length.value));

    auto look =
        new LookView(this->GetPane(), "Look", controls.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        infinite,
        length.Layout(wxHORIZONTAL).release(),
        look);

    this->ConfigureTopSizer(std::move(sizer));
}


} // end namespace draw
