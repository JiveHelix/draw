#pragma once


#include "draw/detail/unique_id.h"


namespace draw
{


namespace detail
{


class PolyShapeId: public UniqueId<ssize_t>
{
public:
    PolyShapeId();
};


} // end namespace detail


} // end namespace draw
