#include "draw/views/quad_view.h"

#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/knob.h>
#include <wxpex/button.h>
#include <wxpex/converter.h>

#include "draw/views/affine_view.h"
#include "draw/views/size_view.h"


namespace draw
{

template<typename Control>
class AffineKnobView: public wxControl
{
public:
    AffineKnobView(
        wxWindow *parent,
        Control control,
        const wxpex::LayoutOptions &layoutOptions)
        :
        wxControl(parent, wxID_ANY)
    {
        using namespace wxpex;

        auto x = LabeledWidget(
            this,
            "x",
            CreateFieldKnob<1>(
                this,
                control.x,
                control.x.value));

        auto y = LabeledWidget(
            this,
            "y",
            CreateFieldKnob<1>(
                this,
                control.y,
                control.y.value));

        auto sizer = LayoutLabeled(layoutOptions, x, y);
        this->SetSizerAndFit(sizer.release());
    }
};


QuadView::QuadView(
    wxWindow *parent,
    const std::string &name,
    QuadControl control,
    const wxpex::LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto center = wxpex::LabeledWidget(
        this,
        "center",
        new AffineView(this, control.center));

    auto size = wxpex::LabeledWidget(
        this,
        "size",
        new SizeView<int>(this, control.size));

    auto scale = wxpex::LabeledWidget(
        this,
        "scale",
        new ScaleSlider(
            this,
            control.scale,
            control.scale.value));

    auto rotation = wxpex::LabeledWidget(
        this,
        "rotation",
        wxpex::CreateFieldKnob<1>(
            this,
            control.rotation,
            control.rotation.value,
            KnobSettings().StartAngle(0.0).Continuous(true)));

    auto shear = wxpex::LabeledWidget(
        this,
        "shear",
        new AffineKnobView(this, control.shear, layoutOptions));

    auto perspective = wxpex::LabeledWidget(
        this,
        "perspective",
        new AffineKnobView(this, control.perspective, layoutOptions));

    auto reset =
        new wxpex::Button(this, "Reset", control.reset);

    auto sizer = wxpex::LayoutLabeled(
        layoutOptions,
        center,
        size,
        scale,
        rotation,
        shear,
        perspective);

    auto topSizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        sizer.release(),
        reset);

    this->ConfigureSizer(std::move(topSizer));
}


} // end namespace draw
