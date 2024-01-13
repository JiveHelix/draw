#include "draw/views/polygon_view.h"


namespace draw
{


PointsView::PointsView(wxWindow *parent, Control control)
    :
    wxListView(parent, wxID_ANY),
    endpoint_(this, control, &PointsView::OnPoints_)
{
    this->InsertColumn(0, "Point");
    this->InsertColumn(1, "x");
    this->InsertColumn(2, "y");

    auto points = control.Get();

    if (!points.empty())
    {
        this->OnPoints_(control.Get());
    }
}

void PointsView::OnPoints_(const Points &points)
{
    this->DeleteAllItems();

    size_t pointIndex = 0;

    for (auto &point: points)
    {
        long index = this->InsertItem(0, std::to_string(pointIndex++));
        this->SetItem(index, 1, Converter::ToString(point.x));
        this->SetItem(index, 2, Converter::ToString(point.y));
    }
}


PolygonView::PolygonView(
    wxWindow *parent,
    const std::string &name,
    PolygonControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::StaticBox(parent, name)
{
    using namespace wxpex;

    auto center = wxpex::LabeledWidget(
        this,
        "center",
        new PointView<double>(this, control.center));

    auto scale = wxpex::LabeledWidget(
        this,
        "scale",
        new draw::ScaleSlider(
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

    auto sizer = wxpex::LayoutLabeled(
        layoutOptions,
        center,
        scale,
        rotation);

    this->ConfigureSizer(std::move(sizer));
}


PolygonView::PolygonView(
    wxWindow *parent,
    PolygonControl control,
    const LayoutOptions &layoutOptions)
    :
    PolygonView(parent, "Polygon", control, layoutOptions)
{

}


} // end namespace draw
