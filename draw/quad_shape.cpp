#include "draw/polygon_shape.h"
#include "draw/quad_shape.h"

// #define ENABLE_BLOCK_TIMER
#include <jive/block_timer.h>


namespace draw
{


QuadShape::QuadShape(const Quad &quad, const Look &look_)
    :
    QuadShapeTemplate<pex::Identity>({quad, look_})
{

}

void QuadShape::Draw(wxpex::GraphicsContext &context)
{
    BLOCK_TIMER(jive::TimeValue());

    if (this->quad.size.GetArea() < 0.5)
    {
        return;
    }

    ConfigureLook(context, this->look);
    DrawPolygon(context, this->quad.GetPoints());
}


} // end namespace draw
