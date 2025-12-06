#include "draw/views/regular_polygon_view.h"

#include "draw/views/point_view.h"
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/knob.h>


namespace draw
{


RegularPolygonView::RegularPolygonView(
    wxWindow *parent,
    const std::string &name,
    const RegularPolygonControl &control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto panel = this->GetPanel();

    auto center = wxpex::LabeledWidget(
        panel,
        "center",
        new PointView<double>(panel, control.center));

    auto radius = wxpex::LabeledWidget(
        panel,
        "radius",
        wxpex::CreateFieldSlider<2>(
            panel,
            control.radius));

    auto sideLength = wxpex::LabeledWidget(
        panel,
        "side length",
        wxpex::CreateField<1>(
            panel,
            control.sideLength));

    auto sideCount = wxpex::LabeledWidget(
        panel,
        "side count",
        wxpex::CreateFieldSlider<1>(
            panel,
            control.sides));

    auto rotation = wxpex::LabeledWidget(
        panel,
        "rotation",
        wxpex::CreateFieldKnob<1>(
            panel,
            control.rotation_deg,
            KnobSettings().StartAngle(0.0).Continuous(true)));

    auto sizer = wxpex::LayoutLabeled(
        layoutOptions,
        center,
        radius,
        sideLength,
        sideCount,
        rotation);

    this->ConfigureSizer(std::move(sizer));
}


RegularPolygonView::RegularPolygonView(
    wxWindow *parent,
    const RegularPolygonControl &control,
    const LayoutOptions &layoutOptions)
    :
    RegularPolygonView(parent, "Polygon", control, layoutOptions)
{

}


} // end namespace draw
