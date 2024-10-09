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
            this->GetPanel(),
            "infinite",
            controls.infinite);

    auto length = wxpex::LabeledWidget(
        this->GetPanel(),
        "length",
        new wxpex::ValueSlider(
            this->GetPanel(),
            controls.length,
            controls.length.value));

    auto look =
        new LookView(this->GetPanel(), "Look", controls.look, layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        infinite,
        length.Layout(wxHORIZONTAL).release(),
        look);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace draw
