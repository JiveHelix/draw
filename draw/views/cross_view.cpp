#include "draw/views/cross_view.h"
#include "draw/views/point_view.h"


namespace draw
{


CrossView::CrossView(
    wxWindow *parent,
    const std::string &name,
    CrossControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto center = wxpex::LabeledWidget(
        this,
        "center",
        new PointView<double>(this, control.center));

    auto size = wxpex::LabeledWidget(
        this,
        "size",
        new wxpex::ValueSlider(
            this,
            control.size,
            control.size.value));

    auto rotation = wxpex::LabeledWidget(
        this,
        "rotation",
        wxpex::CreateFieldKnob<1>(
            this,
            control.rotation,
            control.rotation.value,
            KnobSettings().StartAngle(0.0).Continuous(true)));

    auto sizer = wxpex::LayoutLabeled(
        layoutOptions,
        center,
        size,
        rotation);

    this->ConfigureSizer(std::move(sizer));
}


CrossView::CrossView(
    wxWindow *parent,
    CrossControl control,
    const LayoutOptions &layoutOptions)
    :
    CrossView(parent, "Cross", control, layoutOptions)
{

}


} // end namespace draw
