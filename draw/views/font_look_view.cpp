#include "draw/views/font_look_view.h"


#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/color_picker.h>
#include <wxpex/layout_items.h>


namespace draw
{


FontLookView::FontLookView(
    wxWindow *parent,
    const std::string &name,
    const FontLookControl &control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto enable =
        new CheckBox(panel, "Enable", control.enable);

    auto pointsSize = wxpex::LabeledWidget(
        panel,
        "Point Size",
        new Field(
            panel,
            control.pointSize));

    auto color =
        new HsvPicker(
            panel,
            "Color",
            control.color);

    auto antialias =
        new CheckBox(panel, "Anti-alias", control.antialias);

    auto sizer = wxpex::LayoutItems(
        verticalItems,
        enable,
        pointsSize.Layout(wxHORIZONTAL),
        color,
        antialias);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace draw
