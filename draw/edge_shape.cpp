#include "draw/edge_shape.h"


namespace draw
{



EdgeShape::EdgeShape(
    const EdgeSettings &settings,
    const Edges &edges)
    :
    settings_(settings),
    edges_(edges)
{

}


void EdgeShape::Draw(DrawContext &context)
{
    if (this->edges_.empty())
    {
        return;
    }

    // wxpex::MaintainTransform maintainTransform(context);
    context.ConfigureLook(this->settings_.look);
    auto path = context->CreatePath();

    for (auto &edge: this->edges_)
    {
        path.MoveToPoint(edge.start.x, edge.start.y);
        path.AddLineToPoint(edge.end.x, edge.end.y);
        path.CloseSubpath();
    }

    context->DrawPath(path);
}


} // end namespace draw


template struct pex::Group
<
    draw::EdgeSettingsFields,
    draw::EdgeSettingsTemplate,
    pex::PlainT<draw::EdgeSettings>
>;
