#include "draw/views/font_look_view.h"


#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/color_picker.h>


namespace draw
{


FontLookView::FontLookView(
    wxWindow *parent,
    const std::string &name,
    FontLookControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto enable =
        new CheckBox(this, "Enable", control.enable);

    auto pointsSize = wxpex::LabeledWidget(
        this,
        "Point Size",
        new Field(
            this,
            control.pointSize));

    auto color =
        new HsvPicker(
            this,
            "Color",
            control.color);

    auto antialias =
        new CheckBox(this, "Anti-alias", control.antialias);

    auto sizer = wxpex::LayoutItems(
        verticalItems,
        enable,
        pointsSize.Layout(wxHORIZONTAL).release(),
        color,
        antialias);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace draw
