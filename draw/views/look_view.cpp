#include "draw/views/look_view.h"


#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/color_picker.h>


namespace draw
{


LookView::LookView(
    wxWindow *parent,
    const std::string &name,
    LookControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto strokeEnable =
        new CheckBox(this, "Stroke Enable", control.strokeEnable);

    auto strokeWeight = wxpex::LabeledWidget(
        this,
        "Stroke Weight",
        new FieldSlider(
            this,
            control.strokeWeight,
            control.strokeWeight.value));

    auto strokeColor =
        new HsvPicker(
            this,
            "Stroke Color",
            control.strokeColor);

    auto fillEnable =
        new CheckBox(this, "Fill Enable", control.fillEnable);

    auto fillColor =
        new HsvPicker(
            this,
            "Fill Color",
            control.fillColor);

    auto antialias =
        new CheckBox(this, "Anti-alias", control.antialias);

    auto sizer = wxpex::LayoutItems(
        verticalItems,
        strokeEnable,
        strokeWeight.Layout(wxHORIZONTAL).release(),
        strokeColor,
        fillEnable,
        fillColor,
        antialias);

    this->ConfigureSizer(std::move(sizer));
}


LookView::LookView(
    wxWindow *parent,
    LookControl control,
    const LayoutOptions &layoutOptions)
    :
    LookView(parent, "Look", control, layoutOptions)
{

}


StrokeView::StrokeView(
    wxWindow *parent,
    const std::string &name,
    LookControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto strokeEnable =
        new CheckBox(this, "Stroke Enable", control.strokeEnable);

    auto strokeWeight = wxpex::LabeledWidget(
        this,
        "Stroke Weight",
        new FieldSlider(
            this,
            control.strokeWeight,
            control.strokeWeight.value));

    auto strokeColor =
        new HsvPicker(
            this,
            "Stroke Color",
            control.strokeColor);

    auto antialias =
        new CheckBox(this, "Anti-alias", control.antialias);

    auto sizer = wxpex::LayoutItems(
        verticalItems,
        strokeEnable,
        strokeWeight.Layout(wxHORIZONTAL).release(),
        strokeColor,
        antialias);

    this->ConfigureSizer(std::move(sizer));
}


StrokeView::StrokeView(
    wxWindow *parent,
    LookControl control,
    const LayoutOptions &layoutOptions)
    :
    StrokeView(parent, "Stroke", control, layoutOptions)
{

}


} // end namespace draw
