#pragma once


#include <vector>
#include <tau/vector2d.h>


namespace draw
{


using PointDouble = tau::Point2d<double>;
using PointsDouble = std::vector<PointDouble>;
using PointsIterator = typename PointsDouble::const_iterator;


} // end namespace artis
