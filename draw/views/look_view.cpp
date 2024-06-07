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
    this->Initialize_(control);
}


StrokeControls::StrokeControls(
    wxWindow *parent,
    LookControl control,
    wxpex::Collapsible::StateControl expandControl,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Stroke", expandControl, borderStyle)
{
    this->Initialize_(control);
}


void StrokeControls::Initialize_(LookControl control)
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
        wxpex::verticalItems,
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
    this->Initialize_(control);
}


FillControls::FillControls(
    wxWindow *parent,
    LookControl control,
    wxpex::Collapsible::StateControl expandControl,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Fill", expandControl, borderStyle)
{
    this->Initialize_(control);
}


void FillControls::Initialize_(LookControl control)
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
        wxpex::verticalItems,
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
        wxpex::verticalItems,
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


LookView::LookView(
    wxWindow *parent,
    const std::string &name,
    LookControl control,
    LookDisplayControl displayControl,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name, displayControl.lookExpand, borderStyle)
{
    auto strokeControls = new StrokeControls(
        this->GetPanel(),
        control,
        displayControl.strokeExpand,
        layoutOptions);

    auto fillControls = new FillControls(
        this->GetPanel(),
        control,
        displayControl.fillExpand,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        strokeControls,
        fillControls);

    this->ConfigureTopSizer(std::move(sizer));
}


LookView::LookView(
    wxWindow *parent,
    LookControl control,
    LookDisplayControl displayControl,
    const LayoutOptions &layoutOptions)
    :
    LookView(parent, "Look", control, displayControl, layoutOptions)
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


StrokeView::StrokeView(
    wxWindow *parent,
    const std::string &name,
    LookControl control,
    LookDisplayControl displayControl,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name, displayControl.lookExpand, borderStyle)
{
    auto strokeControls = new StrokeControls(
        this->GetPanel(),
        control,
        displayControl.strokeExpand,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        strokeControls);

    this->ConfigureTopSizer(std::move(sizer));
}


StrokeView::StrokeView(
    wxWindow *parent,
    LookControl control,
    LookDisplayControl displayControl,
    const LayoutOptions &layoutOptions)
    :
    StrokeView(parent, "Stroke", control, displayControl, layoutOptions)
{

}


} // end namespace draw
