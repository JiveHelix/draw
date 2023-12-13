#include "draw/polygon_shape.h"
#include "draw/quad_shape.h"

// #define ENABLE_BLOCK_TIMER
#include <jive/block_timer.h>


namespace draw
{


QuadShape::QuadShape(const Quad &quad, const Look &look_)
    :
    QuadShapeTemplate<pex::Identity>({0, quad, look_})
{

}


QuadShape::QuadShape(size_t id_, const Quad &quad, const Look &look_)
    :
    QuadShapeTemplate<pex::Identity>({id_, quad, look_})
{

}


void QuadShape::Draw(wxpex::GraphicsContext &context)
{
    BLOCK_TIMER(jive::TimeValue());

    if (this->shape.size.GetArea() < 0.5)
    {
        return;
    }

    ConfigureLook(context, this->look);
    DrawPolygon(context, this->shape.GetPoints());
}


QuadShapeModel::QuadShapeModel()
    :
    QuadShapeGroup::Model()
{
    static size_t nextId = 0;
    this->id.Set(nextId++);
}


void QuadShapeModel::Set(const QuadShape &other)
{
    // Do not change the id.
    this->shape.Set(other.shape);
    this->look.Set(other.look);
}


} // end namespace draw
