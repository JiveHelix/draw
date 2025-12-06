#include "draw/views/look_view.h"


#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/check_box.h>
#include <wxpex/color_picker.h>
#include <wxpex/layout_items.h>


#ifdef __WXMSW__
inline constexpr auto borderStyle = wxBORDER_NONE;
#else
inline constexpr auto borderStyle = wxBORDER_SIMPLE;
#endif


namespace draw
{


StrokeView::StrokeView(
    wxWindow *parent,
    const StrokeControl &control,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Stroke", borderStyle)
{
    this->Initialize_(control);
}


StrokeView::StrokeView(
    wxWindow *parent,
    const StrokeControl &control,
    const wxpex::Collapsible::StateControl &expandControl,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Stroke", expandControl, borderStyle)
{
    this->Initialize_(control);
}


void StrokeView::Initialize_(const StrokeControl &control)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto enable =
        new CheckBox(panel, "Enable", control.enable);

    auto weight = wxpex::LabeledWidget(
        panel,
        "Weight",
        CreateFieldSlider<1>(
            panel,
            control.weight));

    auto color =
        new HsvaPicker(
            panel,
            "Color",
            control.color);

    auto penStyle = wxpex::CreateGraphicsControl(panel, control.penStyle);
    auto penCap = wxpex::CreateGraphicsControl(panel, control.penCap);
    auto penJoin = wxpex::CreateGraphicsControl(panel, control.penJoin);

    auto antialias =
        new CheckBox(panel, "Anti-alias", control.antialias);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        weight.Layout(wxHORIZONTAL),
        color,
        wxpex::LayoutLabeled(
            wxpex::LayoutOptions{},
            penStyle,
            penCap,
            penJoin),
        antialias);

    this->ConfigureSizer(std::move(sizer));
}


FillView::FillView(
    wxWindow *parent,
    const FillControl &control,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Fill", borderStyle)
{
    this->Initialize_(control);
}


FillView::FillView(
    wxWindow *parent,
    const FillControl &control,
    const wxpex::Collapsible::StateControl &expandControl,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Fill", expandControl, borderStyle)
{
    this->Initialize_(control);
}


void FillView::Initialize_(const FillControl &control)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto fillEnable =
        new CheckBox(panel, "Fill Enable", control.enable);

    auto fillColor =
        new HsvaPicker(
            panel,
            "Fill Color",
            control.color);

    auto brushStyle = wxpex::CreateGraphicsControl(panel, control.brushStyle);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        fillEnable,
        fillColor,
        brushStyle.Layout(wxHORIZONTAL).release());

    this->ConfigureSizer(std::move(sizer));
}


LookView::LookView(
    wxWindow *parent,
    const std::string &name,
    const LookControl &control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name, borderStyle)
{
    auto strokeControls = new StrokeView(
        this->GetPanel(),
        control.stroke,
        layoutOptions);

    auto fillControls = new FillView(
        this->GetPanel(),
        control.fill,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        strokeControls,
        fillControls);

    this->ConfigureSizer(std::move(sizer));
}


LookView::LookView(
    wxWindow *parent,
    const LookControl &control,
    const LayoutOptions &layoutOptions)
    :
    LookView(parent, "Look", control, layoutOptions)
{

}


LookView::LookView(
    wxWindow *parent,
    const std::string &name,
    const LookControl &control,
    const LookDisplayControl &displayControl,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name, displayControl.lookExpand, borderStyle)
{
    auto strokeControls = new StrokeView(
        this->GetPanel(),
        control.stroke,
        displayControl.strokeExpand,
        layoutOptions);

    auto fillControls = new FillView(
        this->GetPanel(),
        control.fill,
        displayControl.fillExpand,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        strokeControls,
        fillControls);

    this->ConfigureSizer(std::move(sizer));
}


LookView::LookView(
    wxWindow *parent,
    const LookControl &control,
    const LookDisplayControl &displayControl,
    const LayoutOptions &layoutOptions)
    :
    LookView(parent, "Look", control, displayControl, layoutOptions)
{

}


StrokeOnlyView::StrokeOnlyView(
    wxWindow *parent,
    const std::string &name,
    const StrokeControl &control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name, borderStyle)
{
    auto strokeControls = new StrokeView(
        this->GetPanel(),
        control,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        strokeControls);

    this->ConfigureSizer(std::move(sizer));
}


StrokeOnlyView::StrokeOnlyView(
    wxWindow *parent,
    const StrokeControl &control,
    const LayoutOptions &layoutOptions)
    :
    StrokeOnlyView(parent, "Stroke", control, layoutOptions)
{

}


StrokeOnlyView::StrokeOnlyView(
    wxWindow *parent,
    const std::string &name,
    const StrokeControl &control,
    const LookDisplayControl &displayControl,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, name, displayControl.lookExpand, borderStyle)
{
    auto strokeControls = new StrokeView(
        this->GetPanel(),
        control,
        displayControl.strokeExpand,
        layoutOptions);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        strokeControls);

    this->ConfigureSizer(std::move(sizer));
}


StrokeOnlyView::StrokeOnlyView(
    wxWindow *parent,
    const StrokeControl &control,
    const LookDisplayControl &displayControl,
    const LayoutOptions &layoutOptions)
    :
    StrokeOnlyView(parent, "Stroke", control, displayControl, layoutOptions)
{

}


} // end namespace draw
