#include "draw/points_shape.h"


namespace draw
{


PointsShapeSettings::PointsShapeSettings()
    :
    PointsShapeTemplate<pex::Identity>{
        2.0,
        {}}
{

}


PointsShape::PointsShape(
    const PointsShapeSettings &settings,
    const PointsDouble &points)
    :
    settings_(settings),
    points_(points)
{

}

void PointsShape::Draw(DrawContext &context)
{
    wxpex::MaintainTransform maintainTransform(context);
    context.ConfigureLook(this->settings_.look);

    auto path = context->CreatePath();

    for (auto &point: this->points_)
    {
        auto rounded = point.template Cast<int>();
        path.AddCircle(rounded.x, rounded.y, this->settings_.radius);
        path.CloseSubpath();
    }

    context->DrawPath(path);
}


ValuePointsShape::ValuePointsShape(
    const PointsShapeSettings &settings,
    const ValuePoints &points)
    :
    settings_(settings),
    points_(points)
{

}

void ValuePointsShape::Draw(DrawContext &context)
{
    wxpex::MaintainTransform maintainTransform(context);
    context.ConfigureLook(this->settings_.look);

    for (auto &point: this->points_)
    {
        auto path = context->CreatePath();

        context.ConfigureColors(this->settings_.look, point.value);
        auto rounded = point.template Cast<int>();
        path.AddCircle(rounded.x, rounded.y, this->settings_.radius);
        path.CloseSubpath();

        context->DrawPath(path);
    }

}


} // end namespace draw


template struct pex::Group
<
    draw::PointsShapeFields,
    draw::PointsShapeTemplate,
    pex::PlainT<draw::PointsShapeSettings>
>;
