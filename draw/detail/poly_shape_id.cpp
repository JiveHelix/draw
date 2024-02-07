#include "draw/detail/poly_shape_id.h"


namespace draw
{


namespace detail
{


std::set<int64_t> polyShapeIds;



PolyShapeId::PolyShapeId()
    :
    UniqueId<int64_t>(polyShapeIds)
{

}


} // end namespace detail


} // end namespace draw
