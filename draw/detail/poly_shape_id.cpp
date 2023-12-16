#include "draw/detail/poly_shape_id.h"


namespace draw
{


namespace detail
{


std::set<ssize_t> polyShapeIds;



PolyShapeId::PolyShapeId()
    :
    UniqueId<ssize_t>(polyShapeIds)
{

}


} // end namespace detail


} // end namespace draw
