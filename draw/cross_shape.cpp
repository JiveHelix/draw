#include "draw/cross_shape.h"
#include <tau/angles.h>


namespace draw
{


void DrawCross(
    DrawContext &context,
    const Cross &cross)
{
    auto transform = context->GetTransform();
    transform.Translate(cross.center.x, cross.center.y);
    context->SetTransform(transform);

    if (cross.rotation != 0.0)
    {
        context->Rotate(tau::ToRadians(cross.rotation));
    }

    auto path = context->CreatePath();
    auto halfSize = cross.size / 2.0;

    path.MoveToPoint(-halfSize, 0);
    path.AddLineToPoint(halfSize, 0);
    path.CloseSubpath();

    path.MoveToPoint(0, -halfSize);
    path.AddLineToPoint(0, halfSize);
    path.CloseSubpath();

    context->DrawPath(path);
}


CrossShape::CrossShape(const Cross &cross_, const Look &look_)
    :
    CrossShapeTemplate<pex::Identity>({cross_, look_})
{

}

void CrossShape::Draw(DrawContext &context)
{
    wxpex::MaintainTransform maintainTransform(context);
    context.ConfigureLook(this->look);
    DrawCross(context, this->cross);
}


} // end namespace draw
