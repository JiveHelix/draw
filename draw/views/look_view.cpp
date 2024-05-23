#include "draw/views/look_view.h"


#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/color_picker.h>


#ifdef __WXMSW__
inline constexpr auto borderStyle = wxBORDER_NONE;
#else
inline constexpr auto borderStyle = wxBORDER_SIMPLE;
#endif


namespace draw
{


StrokeControls::StrokeControls(
    wxWindow *parent,
    LookControl control,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Stroke", borderStyle)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto strokeEnable =
        new CheckBox(panel, "Stroke Enable", control.strokeEnable);

    auto strokeWeight = wxpex::LabeledWidget(
        panel,
        "Stroke Weight",
        new FieldSlider(
            panel,
            control.strokeWeight,
            control.strokeWeight.value));

    auto strokeColor =
        new HsvPicker(
            panel,
            "Stroke Color",
            control.strokeColor);

    auto antialias =
        new CheckBox(panel, "Anti-alias", control.antialias);

    auto sizer = wxpex::LayoutItems(
        wxpex::ItemOptions(verticalItems).SetProportion(1),
        strokeEnable,
        strokeWeight.Layout(wxHORIZONTAL).release(),
        strokeColor,
        antialias);

    this->ConfigureTopSizer(std::move(sizer));
}


FillControls::FillControls(
    wxWindow *parent,
    LookControl control,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Fill", borderStyle)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto fillEnable =
        new CheckBox(panel, "Fill Enable", control.fillEnable);

    auto fillColor =
        new HsvPicker(
            panel,
            "Fill Color",
            control.fillColor);

    auto sizer = wxpex::LayoutItems(
        wxpex::ItemOptions(verticalItems).SetProportion(1),
        fillEnable,
        fillColor);

    this->ConfigureTopSizer(std::move(sizer));
}


LookView::LookView(
    wxWindow *parent,
    const std::string &name,
    LookControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name, borderStyle)
{
    auto strokeControls = new StrokeControls(
        this->GetPanel(),
        control,
        layoutOptions);

    auto fillControls = new FillControls(
        this->GetPanel(),
        control,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::ItemOptions(wxpex::verticalItems).SetProportion(1),
        strokeControls,
        fillControls);

    this->ConfigureTopSizer(std::move(sizer));
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
    wxpex::Collapsible(parent, name, borderStyle)
{
    auto strokeControls = new StrokeControls(
        this->GetPanel(),
        control,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        strokeControls);

    this->ConfigureTopSizer(std::move(sizer));
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
