#include "draw/views/ellipse_view.h"

#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/knob.h>

#include "draw/views/point_view.h"


namespace draw
{


EllipseView::EllipseView(
    wxWindow *parent,
    const std::string &name,
    EllipseControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto center = wxpex::LabeledWidget(
        this,
        "center",
        new PointView<double>(this, control.center));

    auto major = wxpex::LabeledWidget(
        this,
        "major",
        wxpex::CreateFieldSlider<1>(
            this,
            control.major,
            control.major.value));

    auto minor = wxpex::LabeledWidget(
        this,
        "minor",
        wxpex::CreateFieldSlider<1>(
            this,
            control.minor,
            control.minor.value));

    auto rotation = wxpex::LabeledWidget(
        this,
        "rotation",
        wxpex::CreateFieldKnob<1>(
            this,
            control.rotation,
            control.rotation.value,
            KnobSettings().StartAngle(0.0).Continuous(true)));

    auto scale = wxpex::LabeledWidget(
        this,
        "scale",
        new draw::ScaleSlider(
            this,
            control.scale,
            control.scale.value));

    this->ConfigureSizer(
        wxpex::LayoutLabeled(
            layoutOptions,
            center,
            major,
            minor,
            rotation,
            scale));
}


EllipseView::EllipseView(
    wxWindow *parent,
    EllipseControl control,
    const LayoutOptions &layoutOptions)
    :
    EllipseView(parent, "Ellipse", control, layoutOptions)
{

}


} // end namespace draw
